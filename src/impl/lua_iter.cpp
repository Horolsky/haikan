/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <utility>

#include "haikan/impl/lua_iter.hpp"

namespace haikan
{
namespace impl
{

namespace
{

using IndexRow = std::vector<std::size_t>;

struct Projection
{
    IndexRow dimensions;
    std::size_t offset;
};

bool next_combination(IndexRow& combination, std::size_t const dimension_count)
{
    std::size_t const n = combination.size();
    for (std::size_t pos = n; pos > 0; --pos)
    {
        std::size_t const idx = pos - 1;
        std::size_t const max_value = dimension_count - n + idx;
        if (combination[idx] < max_value)
        {
            ++combination[idx];
            for (std::size_t tail = idx + 1; tail < n; ++tail)
            {
                combination[tail] = combination[tail - 1] + 1;
            }
            return true;
        }
    }
    return false;
}

std::vector<Projection> make_projections(std::vector<std::size_t> const& sizes, std::size_t const n)
{
    std::vector<Projection> projections;
    IndexRow dimensions(n);
    for (std::size_t idx = 0; idx < n; ++idx)
    {
        dimensions[idx] = idx;
    }

    std::size_t offset = 0;
    do
    {
        Projection projection{dimensions, offset};
        std::size_t projection_size = 1;
        for (std::size_t const dimension: dimensions)
        {
            projection_size *= sizes[dimension];
        }
        offset += projection_size;
        projections.push_back(std::move(projection));
    } while (next_combination(dimensions, sizes.size()));

    return projections;
}

std::size_t projection_key(IndexRow const& row, std::vector<std::size_t> const& sizes, Projection const& projection)
{
    std::size_t key = 0;
    for (std::size_t const dimension: projection.dimensions)
    {
        key *= sizes[dimension];
        key += row[dimension];
    }
    return projection.offset + key;
}

std::vector<IndexRow> make_cartesian_rows(std::vector<std::size_t> const& sizes)
{
    std::vector<IndexRow> rows;
    IndexRow row(sizes.size(), 0);

    while (true)
    {
        rows.push_back(row);
        for (std::size_t dim = row.size(); dim > 0; --dim)
        {
            std::size_t const idx = dim - 1;
            ++row[idx];
            if (row[idx] != sizes[idx])
            {
                break;
            }
            row[idx] = 0;
            if (idx == 0)
            {
                return rows;
            }
        }
    }
}

} // namespace



void LuaIterZipProdBase::init_iters(LuaIter::Vector const& seqences)
{
    halt_ = false;
    sequences_.clear();
    sizes_.clear();
    indexes_.clear();

    if (seqences.empty())
    {
        halt_ = true;
        return;
    }

    sequences_.reserve(seqences.size());
    sizes_.reserve(seqences.size());
    for (auto const& v: seqences)
    {
        if (v.get_type() != sol::type::table)
        {
            throw std::runtime_error("not a sequence");
        }

        sol::table table = v.as<sol::table>();
        std::size_t const size = table.size();
        std::size_t count = 0;

        for (auto const& item: table)
        {
            sol::object const& key = item.first;
            if (!key.is<int>())
            {
                throw std::runtime_error("not a sequence");
            }

            int const index = key.as<int>();
            if (index < 1 || static_cast<std::size_t>(index) > size)
            {
                throw std::runtime_error("not a sequence");
            }
            ++count;
        }
        if (count != size)
        {
            throw std::runtime_error("not a sequence");
        }

        Vector sequence;
        sequence.reserve(size);
        for (std::size_t index = 1; index <= size; ++index)
        {
            sequence.push_back(table[static_cast<int>(index)]);
        }

        if (sequence.empty())
        {
            halt_ = true;
        }
        sizes_.push_back(sequence.size());
        sequences_.push_back(std::move(sequence));
    }

    indexes_.assign(sequences_.size(), 0);
}

LuaIter::Vector LuaIterZipProdBase::get()
{
    LuaIter::Vector arr {};
    if (halt_) { return arr; }
    arr.reserve(sequences_.size());
    for (std::size_t idx = 0; idx < sequences_.size(); ++idx)
    {
        arr.push_back(sequences_[idx][indexes_[idx]]);
    }
    return arr;
}

void LuaProdIter::step()
{
    for (std::size_t dim = indexes_.size(); dim > 0; --dim)
    {
        std::size_t const idx = dim - 1;
        ++indexes_[idx];

        if (indexes_[idx] != sizes_[idx])
        {
            return;
        }

        indexes_[idx] = 0;
        if (idx == 0)
        {
            halt_ = true;
            return;
        }
    }
}

void LuaZipIter::step()
{
    for (std::size_t idx = 0; idx < indexes_.size(); ++idx)
    {
        ++indexes_[idx];
        if (indexes_[idx] == sizes_[idx])
        {
            halt_ = true;
        }
    }
}

LuaNwiseIter::LuaNwiseIter(Vector const& sequences, std::size_t const n)
    : LuaIterZipProdBase(sequences)
    , n_{n}
{
    if (n_ == 0)
    {
        throw std::invalid_argument("n-wise degree must be positive");
    }
    if (!halt_)
    {
        n_ = std::min(n_, sequences_.size());
        build_rows();
    }
}

LuaNwiseIter::LuaNwiseIter(Vector && sequences, std::size_t const n)
    : LuaNwiseIter(static_cast<Vector const&>(sequences), n)
{
}

LuaNwiseIter::LuaNwiseIter(std::size_t const n)
    : LuaIterZipProdBase()
    , n_{n}
{
    if (n_ == 0)
    {
        throw std::invalid_argument("n-wise degree must be positive");
    }
}

void LuaNwiseIter::build_rows()
{
    std::vector<Projection> const projections = make_projections(sizes_, n_);
    std::vector<IndexRow> const candidates = make_cartesian_rows(sizes_);
    std::vector<std::vector<std::size_t>> candidate_keys;
    candidate_keys.reserve(candidates.size());

    std::unordered_set<std::size_t> uncovered;
    for (auto const& candidate: candidates)
    {
        std::vector<std::size_t> keys;
        keys.reserve(projections.size());
        for (auto const& projection: projections)
        {
            std::size_t const key = projection_key(candidate, sizes_, projection);
            keys.push_back(key);
            uncovered.insert(key);
        }
        candidate_keys.push_back(std::move(keys));
    }

    rows_.clear();
    rows_.reserve(candidates.size());
    std::vector<bool> selected(candidates.size(), false);

    while (!uncovered.empty())
    {
        std::size_t best_idx = candidates.size();
        std::size_t best_cover = 0;

        for (std::size_t idx = 0; idx < candidates.size(); ++idx)
        {
            if (selected[idx])
            {
                continue;
            }

            std::size_t cover = 0;
            for (std::size_t const key: candidate_keys[idx])
            {
                cover += uncovered.count(key);
            }

            if (cover > best_cover)
            {
                best_idx = idx;
                best_cover = cover;
            }
        }

        if (best_cover == 0)
        {
            break;
        }

        selected[best_idx] = true;
        rows_.push_back(candidates[best_idx]);
        for (std::size_t const key: candidate_keys[best_idx])
        {
            uncovered.erase(key);
        }
    }

    row_ = 0;
    halt_ = rows_.empty();
    if (!halt_)
    {
        indexes_ = rows_.front();
    }
}

void LuaNwiseIter::step()
{
    ++row_;
    halt_ = row_ == rows_.size();
    if (!halt_)
    {
        indexes_ = rows_[row_];
    }
}

} // namespace impl
} // namespace haikan
