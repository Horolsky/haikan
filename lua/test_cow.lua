#! /usr/bin/env lua

require('unittest')
require('cow')


local TS = TestSuite:new("Test COW.lua")

TS:test("1-level cow",
    function ()
        local orig = { a = 42 }
        local cow = create_deep_cow(orig, {})
        assert(cow.a == orig.a)

        cow.a = 67
        assert(cow.a == 67)
        assert(orig.a == 42)

        cow.b = "hello"
        assert(cow.b == "hello")
        assert(orig.b == nil)
    end
)


TS:test("deep cow",
    function ()
        local orig = { a = { b = { c = 42 }} }
        local cow = create_deep_cow(orig, {})

        cow.a.b.c = 67
        assert(cow.a.b.c == 67)
        assert(orig.a.b.c == 42)
    end
)

TS:test("deep cow circular",
    function ()
        local orig = { a = { b = { c = 42 }} }
        local cow = create_deep_cow(orig, {})
        cow.orig = orig

        cow.orig.a.b.c = 67
        assert(cow.orig.a.b.c == 67)
        assert(cow.a.b.c == 42)
        assert(orig.a.b.c == 42)
    end
)

TS:test("deep cow prune",
    function ()
        local orig = { a = { b = { c = 42 }} }
        local cow = create_deep_cow(orig, {})

        assert(orig.a.b.c == 42)
        cow.a = 67
        assert(cow.a == 67)
        assert(orig.a.b.c == 42)
    end
)


os.exit(TS:run())
