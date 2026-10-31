/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once


#include <cstddef>
#include <vector>

#define SOL_ALL_SAFETIES_ON
#define SOL_CHECK_ARGUMENTS
#include <sol/sol.hpp>

namespace haikan {
namespace impl {



/// Lua sequence iterator interface
class LuaIter
{
    virtual void step() = 0;
public:

    using Vector = std::vector<sol::object>;

    LuaIter() = default;
    virtual ~LuaIter() = default;
    LuaIter(LuaIter const&) = default;
    LuaIter& operator=(LuaIter const&) = default;
    LuaIter(LuaIter &&) = default;
    LuaIter& operator=(LuaIter &&) = default;

    /// Iterator halted
    virtual bool halt() const = 0;

    /// Iterator value vector
    ///
    virtual Vector get() = 0;

    /// iterator post-increment
    LuaIter& operator++(int)
    {
        if (!halt())
        {
            this->step();
        }
        return *(this);
    };

    /// iterator value
    Vector operator*()
    {
        return get();
    }
};


/// Lua sequence iterator base for zip, cartesian product, or N-wise coverage
class LuaIterZipProdBase : public virtual LuaIter
{
protected:

    bool halt_{false};
    std::vector<Vector> sequences_{};
    std::vector<std::size_t> sizes_{};
    std::vector<std::size_t> indexes_{};

private:

    void init_iters(Vector const& seqences);

public:

    bool halt() const final
    {
        return halt_;
    }

    explicit LuaIterZipProdBase(Vector const& sequences) : LuaIter()
    {
        init_iters(sequences);
    }

    explicit LuaIterZipProdBase(Vector && seqences) : LuaIter()
    {
        init_iters(seqences);
    }

    LuaIterZipProdBase() : LuaIterZipProdBase(Vector{})
    {
    }

    virtual ~LuaIterZipProdBase() = default;

    LuaIterZipProdBase(LuaIterZipProdBase const&) = default;
    LuaIterZipProdBase& operator=(LuaIterZipProdBase const&) = default;

    LuaIterZipProdBase(LuaIterZipProdBase &&) = default;
    LuaIterZipProdBase& operator=(LuaIterZipProdBase &&) = default;


    Vector get() final;
};


/// Cartesian Product Iterator
class LuaProdIter : public virtual LuaIterZipProdBase
{
    void step() final;

    public:
    using LuaIterZipProdBase::LuaIterZipProdBase;

};


/// Zip Iterator
class LuaZipIter : public virtual LuaIterZipProdBase
{
    void step() final;
    public:
    using LuaIterZipProdBase::LuaIterZipProdBase;
};


/// N-wise coverage iterator
class LuaNwiseIter : public virtual LuaIterZipProdBase
{
    std::size_t n_{0};
    std::vector<std::vector<std::size_t>> rows_{};
    std::size_t row_{0};

    void build_rows();
    void step() final;

public:
    LuaNwiseIter(Vector const& sequences, std::size_t n);
    LuaNwiseIter(Vector && sequences, std::size_t n);
    explicit LuaNwiseIter(std::size_t n);
};


} // namespace impl
} // namespace haikan
