/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/lua_state.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "haikan/logger.hpp"


namespace haikan {
namespace impl {

struct LuaState::MemoryPool
{
    struct alignas(std::max_align_t) Header
    {
        std::size_t capacity;
    };

    struct FreeBlock
    {
        FreeBlock* next;
    };

    static constexpr std::size_t min_block = 16;
    static constexpr std::size_t max_cached_block = 4096;
    static constexpr std::size_t free_list_count = 9;

    std::size_t capacity;
    std::size_t max_capacity;
    std::size_t allocated;
    bool max_capacity_exceeded;
    std::array<FreeBlock*, free_list_count> free_lists{};

    MemoryPool(std::size_t const initial_capacity, std::size_t const max_capacity)
        : capacity{(max_capacity != 0 && initial_capacity > max_capacity) ? max_capacity : initial_capacity}
        , max_capacity{max_capacity}
        , allocated{0}
        , max_capacity_exceeded{false}
    {
    }

    ~MemoryPool()
    {
        for (std::size_t idx = 0; idx < free_lists.size(); ++idx)
        {
            FreeBlock* block = free_lists[idx];
            while (block != nullptr)
            {
                FreeBlock* const next = block->next;
                allocated -= allocation_size(to_header(block));
                std::free(to_header(block));
                block = next;
            }
        }
    }

    static void* allocator(void* ud, void* ptr, std::size_t osize, std::size_t nsize) noexcept
    {
        MemoryPool& pool = *static_cast<MemoryPool*>(ud);

        if (ptr == nullptr)
        {
            return nsize == 0 ? nullptr : pool.allocate(nsize);
        }

        if (nsize == 0)
        {
            pool.deallocate(ptr);
            return nullptr;
        }

        return pool.reallocate(ptr, osize, nsize);
    }

    static int panic(lua_State* L)
    {
        void* ud = nullptr;
        lua_getallocf(L, &ud);
        MemoryPool const& pool = *static_cast<MemoryPool const*>(ud);

        if (pool.max_capacity_exceeded)
        {
            HAIKAN_LOG(ERROR) << "Lua state memory limit exceeded"
                              << ", used_memory=" << pool.allocated
                              << ", max_capacity=" << pool.max_capacity;
            throw std::bad_alloc();
        }

        char const* const message = lua_tostring(L, -1);
        HAIKAN_LOG(ERROR) << "Lua panic: " << (message == nullptr ? "unknown error" : message);
        throw std::runtime_error(message == nullptr ? "lua panic" : message);
    }

    void* allocate(std::size_t const size) noexcept
    {
        std::size_t const capacity = capacity_for(size);
        return allocate_with_capacity(capacity);
    }

    void* allocate_with_capacity(std::size_t const capacity) noexcept
    {
        std::size_t const idx = free_list_index(capacity);

        if (idx < free_lists.size())
        {
            FreeBlock* const block = free_lists[idx];
            if (block != nullptr)
            {
                free_lists[idx] = block->next;
                Header* const header = to_header(block);
                header->capacity = capacity;
                return block;
            }
        }

        std::size_t const bytes = sizeof(Header) + capacity;
        if (!reserve(bytes))
        {
            return nullptr;
        }

        Header* const header = static_cast<Header*>(std::malloc(bytes));
        if (header == nullptr)
        {
            allocated -= bytes;
            return nullptr;
        }
        header->capacity = capacity;
        return header + 1;
    }

    void deallocate(void* ptr) noexcept
    {
        Header* const header = to_header(ptr);
        std::size_t const idx = free_list_index(header->capacity);

        if (idx >= free_lists.size())
        {
            allocated -= allocation_size(header);
            std::free(header);
            return;
        }

        FreeBlock* const block = static_cast<FreeBlock*>(ptr);
        block->next = free_lists[idx];
        free_lists[idx] = block;
    }

    void* reallocate(void* ptr, std::size_t const old_size, std::size_t const size) noexcept
    {
        Header* const old_header = to_header(ptr);
        if (size <= old_header->capacity)
        {
            return ptr;
        }

        void* const next = allocate_with_capacity(grow_capacity(old_header->capacity, size));
        if (next == nullptr)
        {
            return nullptr;
        }

        std::memcpy(next, ptr, std::min(old_size, size));
        deallocate(ptr);
        return next;
    }

    static std::size_t grow_capacity(std::size_t const old_capacity, std::size_t const size) noexcept
    {
        std::size_t const delta = size - old_capacity;
        std::size_t const growth = std::max(old_capacity, delta);
        std::size_t const capacity = (old_capacity > static_cast<std::size_t>(-1) - growth)
            ? size
            : old_capacity + growth;

        return capacity_for(std::max(capacity, size));
    }

    bool reserve(std::size_t const bytes) noexcept
    {
        if (bytes > static_cast<std::size_t>(-1) - allocated)
        {
            max_capacity_exceeded = max_capacity != 0;
            return false;
        }

        std::size_t const required = allocated + bytes;
        if (required <= capacity)
        {
            allocated = required;
            return true;
        }

        if (max_capacity != 0 && required > max_capacity)
        {
            max_capacity_exceeded = true;
            return false;
        }

        capacity = grow_pool_capacity(capacity, required);
        allocated = required;
        return true;
    }

    std::size_t grow_pool_capacity(std::size_t old_capacity, std::size_t const required) const noexcept
    {
        if (old_capacity == 0)
        {
            old_capacity = min_block;
        }

        std::size_t next = old_capacity;
        while (next < required)
        {
            std::size_t const delta = required - next;
            std::size_t const growth = std::max(next, delta);
            next = (next > static_cast<std::size_t>(-1) - growth)
                ? required
                : next + growth;
        }

        return (max_capacity != 0 && next > max_capacity) ? max_capacity : next;
    }

    static std::size_t capacity_for(std::size_t size) noexcept
    {
        if (size <= min_block)
        {
            return min_block;
        }

        if (size > max_cached_block)
        {
            return size;
        }

        --size;
        size |= size >> 1;
        size |= size >> 2;
        size |= size >> 4;
        size |= size >> 8;
        size |= size >> 16;
        if (sizeof(std::size_t) > 4)
        {
            size |= size >> 32;
        }
        return size + 1;
    }

    static std::size_t free_list_index(std::size_t capacity) noexcept
    {
        if ((capacity < min_block) || (capacity > max_cached_block))
        {
            return free_list_count;
        }

        std::size_t idx = 0;
        capacity >>= 4;
        while (capacity > 1)
        {
            capacity >>= 1;
            ++idx;
        }
        return idx;
    }

    static Header* to_header(void* ptr) noexcept
    {
        return static_cast<Header*>(ptr) - 1;
    }

    static std::size_t allocation_size(Header const* header) noexcept
    {
        return sizeof(Header) + header->capacity;
    }
};

LuaState::LuaState(std::size_t const initial_capacity, std::size_t const max_capacity)
    : pool_{new MemoryPool(initial_capacity, max_capacity)}
    , state_{lua_newstate(&MemoryPool::allocator, pool_.get())}
{
    if (state_ == nullptr)
    {
        HAIKAN_LOG(ERROR) << "Failed to initialize Lua state"
                          << ", initial_capacity=" << initial_capacity
                          << ", max_capacity=" << max_capacity;
        throw std::bad_alloc();
    }
    lua_atpanic(state_, &MemoryPool::panic);
}

LuaState::LuaState(LuaState&& other) noexcept
    : pool_{std::move(other.pool_)}
    , state_{other.state_}
{
    other.state_ = nullptr;
}

LuaState& LuaState::operator=(LuaState&& other) noexcept
{
    if (this != &other)
    {
        if (state_ != nullptr)
        {
            lua_close(state_);
        }

        pool_ = std::move(other.pool_);
        state_ = other.state_;
        other.state_ = nullptr;
    }

    return *this;
}

LuaState::~LuaState()
{
    if (state_ != nullptr)
    {
        lua_close(state_);
    }
}

void LuaState::open_libraries()
{
    view().open_libraries();
}

std::size_t LuaState::used_memory() const noexcept
{
    return pool_ == nullptr ? 0 : pool_->allocated;
}

}  // namespace impl
}  // namespace haikan
