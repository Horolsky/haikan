/**
 * @file
 * @copyright (c) Copyright 2026 Oleksandr Khorolskyi
 * @license SPDX-License-Identifier: Apache-2.0
 */


#pragma once

#if defined(__GNUC__) || defined(__clang__)
# define HAIKAN_LIKELY(x) (__builtin_expect(!!(x),1))
# define HAIKAN_UNLIKELY(x) (__builtin_expect(!!(x),0))
#else
# define HAIKAN_LIKELY(x) (!!(x))
# define HAIKAN_UNLIKELY(x) (!!(x))
#endif
