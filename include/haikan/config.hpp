/**
 * @file
 * @copyright (c) Copyright 2022-2023 Volvo Car Corporation
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <memory>
#include <random>


namespace haikan {

/// Global app config
class Config final
{
public:

    Config();

    Config(Config const&) = default;
    Config(Config &&) = default;
    Config& operator=(Config const&) = default;
    Config& operator=(Config &&) = default;

    ~Config() = default;

    /// Set random number generator seed
    Config& SetRngSeed(std::uint64_t const seed);

    /// Reset random number generator to initial seed/state
    Config& ResetRng();

    /// Thread-local random number generator instance
    std::mt19937& Rng();

    std::uint64_t RngSeed() const;

private:
    struct PersistentConfig
    {
        std::uint64_t rng_seed{0x6a09e667f3bcc909ULL};
        std::atomic<std::uint64_t> rng_counter{0};
        std::atomic<std::uint64_t> rng_epoch{0};
    };

    std::shared_ptr<PersistentConfig> config_;
};

} // namespace haikan
