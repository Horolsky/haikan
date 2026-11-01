# (c) Copyright 2024-2025 Zenseact AB

COPTS = [
    "-std=c++14",
    "-Werror",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wno-parentheses", # allow `x & y | z` in tests
    "-Wno-gnu-zero-variadic-macro-arguments",
    "-DBOOST_TEST_MAIN"
]


def make_fixture(name, hdrs, deps, srcs = []):
    native.cc_library(
        name = name,
        testonly = True,
        visibility = [":__pkg__"],
        includes = ["."],
        hdrs = hdrs,
        srcs = srcs,
        deps = deps,
        copts = COPTS,
    )

def make_unit_tests(srcs, tags = [], fixtures = []):
    for src in srcs:
        native.cc_test(
            name = src.replace(".cpp", ""),
            srcs = [src],
            linkstatic=True,
            deps = [
                "//:haikan",
                "@boost.test//:boost.test",
                "@boost.test//:unit_test_main",
            ] + fixtures,
            copts = COPTS,
            linkopts = [
                '-lm'
            ],
            env = {
            },
            tags = tags,
        )
