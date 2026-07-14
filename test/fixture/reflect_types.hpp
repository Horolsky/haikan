#pragma once

#include <boost/describe.hpp>
#include <boost/optional.hpp>

#include "haikan/reflect.hpp"

enum class Foo {None, Lol = 42, Kek = 67 };
BOOST_DESCRIBE_ENUM(Foo, None, Lol, Kek)

struct Lol
{
    Foo foo;
    int x;

    Lol() : foo{Foo::Lol}, x{42} {}

    Lol(Foo foo, int x) : foo{foo}, x{x} {}

    friend bool operator==(Lol const& l, Lol const& r)
    {
        return (l.foo == r.foo) && (l.x == r.x);
    }

    friend bool operator!=(Lol const& l, Lol const& r)
    {
        return !(l == r);
    }

    friend bool operator<(Lol const& l, Lol const& r)
    {
        return (l.foo < r.foo) && (l.x < r.x);
    }
};

BOOST_DESCRIBE_STRUCT(Lol, (), (foo, x))

struct Kek : Lol
{
    double y;

    Kek() : Lol{}, y{3.14} {}

    Kek(Foo foo, int x, double y) : Lol{foo, x}, y{y} {}

    friend bool operator==(Kek const& l, Kek const& r)
    {
        return (static_cast<Lol>(l) == static_cast<Lol>(r)) && (l.y == r.y);
    }

    friend bool operator!=(Kek const& l, Kek const& r)
    {
        return !(l == r);
    }

    friend bool operator<(Kek const& l, Kek const& r)
    {
        return (static_cast<Lol>(l) < static_cast<Lol>(r)) && (l.y < r.y);
    }
};

BOOST_DESCRIBE_STRUCT(Kek, (Lol), (y))

template <>
struct ::haikan::custom_reflect<Kek>
{
    static boost::optional<Kek> init()
    {
        return Kek{Foo::Kek, 67, 13};
    }
};
