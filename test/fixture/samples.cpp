#include "samples.hpp"
#include "haikan/logger.hpp"
#include "haikan/logger.hpp"
#include "haikan/impl/tag_invoke_complex.hpp"

#include "haikan/decorators/precise_real.hpp"
#include "haikan/decorators/underlying.hpp"

using namespace haikan;
using namespace haikan::decorators;
using haikan::impl::Keyword;
using haikan::impl::type;

using J = boost::json::value;
using L = boost::json::array;

std::ostream& operator<<(std::ostream& os, TestEvalSample const& sample)
{
    return os << "{expr: " << sample.expr.prettify()
             << ", expected: " << sample.expected.prettify() << "}";
}


std::vector<TestEvalSample> const& TestSamples()
{
static std::vector<TestEvalSample> const samples
{
    {            Id, nullptr  },
    {       42 | Id, 42       },
    {       "" | Id, ""       },
    {    "foo" | Id, "foo"    },
    {  nullptr | Id, nullptr  },
    { L{1,2,3} | Id, {1,2,3}  },
    {         Q    , nullptr  },
    {    13 | Q(42), 42       },
    {    13 | Q    , nullptr  },
    {         Q(42), 42       },
    {         Q(42), 42       },
    { "foo" | Q(42), 42       },
    { "foo" | Q(42), 42       },

    {              D(42), 42     },
    {    nullptr | D(42), 42     },
    {         "" | D(42), ""     },
    {    L{1,2,3}| D(42), {1,2,3}},
    { 2 | Div(0) | IsErr, true   },
    { 2 | Div(0) | D(42), 42     },

    // boolean cast
    {true    | Bool , true   },
    {false   | Bool , false  },
    {42      | Bool , true   },
    {42.1    | Bool , true   },
    {"false" | Bool , true   },
    {L{1,2}  | Bool , true   },
    {0       | Bool , false  },
    {0.0     | Bool , false  },
    {""      | Bool , false  },
    {L()     | Bool , false  },
    {""      | Nil , true    },
    {L()     | Nil , true    },
    {"false" | Nil , false   },
    {L{1,2}  | Nil , false   },

    // basic constants
    {        True , true    },
    {        False, false   },
    {        Null , nullptr },
    {        Noop , true    },
    {true  | Noop , true    },
    {false | Noop , true    },
    {"foo" | Noop , true    },

    // math constants
    { Pi    , *Pi    },
    { E     , *E     },
    { Inf   , *Inf   },
    { Eps   , *Eps   },

    { Thread | Nil | Not, true},

    // random
    { Rand | Ge(0), true },
    { Rand | Lt(1), true },
    { 16 | Sequence(RandInt(1)) | SetEq({0,1}) , true                  },
    {  3 | Sequence(RandInt(9))                , {5,5,0} /* seed = 42*/},
    { RandInt(0) | IsErr                       , true                  },
    { RandInt(1, 1) | IsErr                    , true                  },
    { RandInt(-3) | IsErr                      , true                  },
    { 999 | Sequence(Rand) | All(
            Size|Eq(999),
            Each(Ge(0)),
            Each(Lt(1)),
            Avg|Near({0.5, 0.1})
        )
        , true
    },


    // comparison
    {42       | Eq(42)  , true  },
    {13       | Eq(42)  , false },
    {L{42, 42} | Eq     , true  },
    {L{13, 42} | Eq     , false },
    {42       | Ne(42)  , false },
    {13       | Ne(42)  , true  },
    {L{42, 42} | Ne     , false },
    {L{13, 42} | Ne     , true  },

    {42                    | Approx(42)              , true  },
    {42.0 + 1e-09          | Approx(42)              , true  },
    {42                    | Approx({42.0, 1e-09})   , true  },
    {L{42, 42.0 + 1e-09}   | Approx                  , true  },
    {42                    | Approx(42.001)          , false },
    {*Pi                   | Approx({3.14, 0.001})   , true  },
    {*E                    | Approx({2.71, 0.005})   , true  },
    {*Pi                   | Approx({3.14, 0, 0.01}) , true  },
    {*E                    | Approx({2.71, 0, 0.01}) , true  },
    {L{*Pi, {3.14, 0.001}} | Approx                  , true  },
    {L{*Pi, {3.15, 0.001}} | Approx                  , false },
    // range check
    {*Pi | All(Gt(3.14), Lt(3.15)), true},

    { 1.49999 | Round,  1 },
    { 1.5     | Round,  2 },
    { 0.49999 | Round,  0 },
    {-0.49999 | Round,  0 },
    {-0.5     | Round, -1 },
    {-1.49999 | Round, -1 },
    {-1.5     | Round, -2 },

    {1.49999 | Mul(2)|Round|Div(2),  1.5},

    // order relation
    { 41        | Lt(42) , true  },
    { 42        | Lt(42) , false },
    { L{41, 42} | Lt     , true  },
    { L{42, 42} | Lt     , false },
    { 43        | Gt(42) , true  },
    { 42        | Gt(42) , false },
    { L{43, 42} | Gt     , true  },
    { L{42, 42} | Gt     , false },
    { 41        | Le(42) , true  },
    { 42        | Le(42) , true  },
    { 43        | Le(42) , false },
    { L{41, 42} | Le     , true  },
    { L{42, 42} | Le     , true  },
    { L{43, 42} | Le     , false },
    { 43        | Ge(42) , true  },
    { 42        | Ge(42) , true  },
    { 41        | Ge(42) , false },
    { L{43, 42} | Ge     , true  },
    { L{42, 42} | Ge     , true  },
    { L{41, 42} | Ge     , false },

    {"bbbb"     | Gt("aaaa")  , true  },
    {"aaaa"     | Gt("aa")    , true  },
    { L{1,2,3}  | Gt({1,1,1}) , true  },

    { L{1,3}             |    Subset({1,2,3}) , true  },
    { L{1,2,3}           |    Subset({1,3}  ) , false },
    { L{1,2}             |    Subset({1,2}  ) , true  },
    { L{1,2,3}           |  Superset({1,3}  ) , true  },
    { L{1,3}             |  Superset({1,2,3}) , false },
    { L{1,2}             |  Superset({1,2})   , true  },
    { L{1,3}             |   PSubset({1,2,3})      , true  },
    { L{1,2,3}           |   PSubset({1,3}  )      , false },
    { L{1,2}             |   PSubset({1,2}  )      , false },
    { L{1,2,3}           | PSuperset({1,3}  )      , true  },
    { L{1,3}             | PSuperset({1,2,3})      , false },
    { L{1,2}             | PSuperset({1,2})        , false },
    { L{{1,3}  ,{1,2,3}} |    Subset, true                  },
    { L{{1,2,3},{1,3}  } |  Superset, true                  },
    { L{{1,3}  , {1,2,3}}|   PSubset, true                  },
    { L{{1,2,3}, {1,3}  }| PSuperset, true                  },
    { L{}      |    Subset({1}) , true  },
    { L{}      |  Superset({1}) , false },
    { L{}      |   PSubset({1}) , true  },
    { L{}      | PSuperset({1}) , false },
    { L{1}     |    Subset(L{}) , false },
    { L{1}     |  Superset(L{}) , true  },
    { L{1}     |   PSubset(L{}) , false },
    { L{1}     | PSuperset(L{}) , true  },
    { L{}      |    Subset(L{}) , true  },
    { L{}      |  Superset(L{}) , true  },
    { L{}      |   PSubset(L{}) , false },
    { L{}      | PSuperset(L{}) , false },
    { L{1,2,3} | SetEq({1,2,3}) , true  },
    { L{1,3,2} | SetEq({1,2,3}) , true  },
    { L{1,2}   | SetEq({1,2,3}) , false },
    { L{1,2,2,4,3,4}     | Uniques                 , {3,4,2,1}             },
    { L{{1,2,3},{2,3,4}} | Union|Sort              , {1,2,3,4}             },
    { L{{1,2,3},{2,3,4}} | Intersect|Sort          , {2,3}                 },
    { L{{1,2,3},{2,3,4}} | Diff                    , L{1}                  },
    { L{{1,2,3},{2,3,4}} | Reverse|Diff            , L{4}                  },
    { L{1,2,3}           | Union({2,3,4})|Sort     , {1,2,3,4}             },
    { L{1,2,3}           | Intersect({2,3,4})|Sort , {2,3}                 },
    { L{1,2,3}           | Diff({2,3,4})           , L{1}                  },
    { L{1,2,3}           | Flip(Diff({2,3,4}))     , L{4}                  },
    { L{1,2,3}           |    ~(Diff({2,3,4}))     , L{4}                  },
    { L{L{},L{}}         | Union                   , L{}                   },
    { L{L{},L{}}         | Intersect               , L{}                   },
    { L{L{},L{}}         | Diff                    , L{}                   },
    { L{L{},L{}}         | Reverse|Diff            , L{}                   },

    {2 | In({1, 2})          , true  },
    {3 | In({1, 2})          , false },
    {3 | NotIn({1, 2})       , true  },
    {2 | NotIn({1, 2})       , false },
    {L{2, {1, 2}} | In       , true  },
    {L{3, {1, 2}} | In       , false },
    {L{3, {1, 2}} | NotIn    , true  },
    {L{2, {1, 2}} | NotIn    , false },
    {L{1, 2}      | Ni(2)    , true  },
    {L{1, 2}      | Ni(3)    , false },
    {L{1, 2}      | NotNi(3) , true  },
    {L{1, 2}      | NotNi(2) , false },
    {L{{1, 2}, 2} | Ni       , true  },
    {L{{1, 2}, 3} | Ni       , false },
    {L{{1, 2}, 3} | NotNi    , true  },
    {L{{1, 2}, 2} | NotNi    , false },

    {L{1,2,2,2}           | Size            , 4                     },
    {L{1,2,2,2}           | Card            , 2                     },
    {L{1,2,2,4,3,4}       | Card            , 4                     },
    {L{1,2}               | Sort            , {1,2}                 },
    {L{2,1}               | Sort            , {1,2}                 },
    {L{4,1,3,1,2,1}       | Sort            , {1,1,1,2,3,4}         },
    {L{4,1,3,1,2,1}       | Sort|Reverse    , {4,3,2,1,1,1}         },
    {L{4,1,3,1,2,1}       | Sort(Neg)       , {4,3,2,1,1,1}         },
    {L{{1,3},{2,1},{3,2}} | Sort            , {{1,3},{2,1},{3,2}}   },
    {L{{3,2},{2,1},{1,3}} | Sort            , {{1,3},{2,1},{3,2}}   },
    {L{{1,3},{2,1},{3,2}} | Sort(At(1))     , {{2,1},{3,2},{1,3}}   },
    {L{{3,2},{2,1},{1,3}} | Sort(At(1))     , {{2,1},{3,2},{1,3}}   },
    {L{}                  | Sort            , L{}                   },
    {L{}                  | Reverse         , L{}                   },

    {L{1,2,2,4,3,4,-1}       | Min          , -1                    },
    {L{1,2,2,4,3,4,-1}       | Max          , 4                     },
    {L{1,2,2,4,3,4,-1}       | Argmin       , 6                     },
    {L{1,2,2,4,3,4,-1}       | Argmax       , 3                     },
    {L{{3, 2},{2, 1},{1, 3}} | Min          , {1,3}                 },
    {L{{3, 2},{2, 1},{1, 3}} | Max          , {3,2}                 },
    {L{{3, 2},{2, 1},{1, 3}} | Min(At(1))   , {2,1}                 },
    {L{{3, 2},{2, 1},{1, 3}} | Max(At(1))   , {1,3}                 },
    {L{{3, 2},{2, 1},{1, 3}} | Argmin(At(1)), 1                     },
    {L{{3, 2},{2, 1},{1, 3}} | Argmax(At(1)), 2                     },
    {L{}                     | Min          , nullptr               },
    {L{}                     | Max          , nullptr               },
    {L{}                     | Argmin       , nullptr               },
    {L{}                     | Argmax       , nullptr               },

    {L{1,2,3,4,5} | Slide(3)           , {{1,2,3},{2,3,4},{3,4,5}}},
    {L{1,2,3,4,5} | Slide(3)|Map(Sum)  , {6      , 9      , 12   }},
    {L{1,2,3,4,5} | Slide(3)|Map(Prod) , {6      , 24     , 60   }},
    {L{1,2,3,4,5} | Slide(3)|Map(Avg)  , {2      , 3      ,  4   }},
    {L{1,2,3}     | Slide(42)          , L{}                      },
    {L{}          | Slide(42)          , L{}                      },

    {L{} | Slide(-1) | Kwrd   , "Err"   },
    {L{} | Try(Slide(-1))     , nullptr },

    {L{2,2,3,3} | Fork(Size|4, Size, Card) , {true, 4, 2}   },
    {L{2,2,3,3} | (Size|4) & Size & Card   , {{true, 4}, 2} },
    {L{1,2,3}   | At(0) & At(2)            , {1,3}          },
    {L{2,2,3,3} | Reduce(Add) & Size | Div , 2.5            },


    {L{1,2,3,4,5,6} | Stride(2)  , {{1,2},{3,4},{5,6}} },
    {L{1,2,3,4,5}   | Stride(3)  , L{{1,2,3}}          },
    {L{1,2,3}       | Stride(42) , L{}                 },
    {L{}            | Stride(42) , L{}                 },
    {L{1,2,3,4,5,6} | Chunks(2)  , {{1,2},{3,4},{5,6}} },
    {L{1,2,3,4,5}   | Chunks(3)  , {{1,2,3},{4,5}}     },

    {4        | Arange, {0,1,2,3} },
    {L{4}     | Arange, {0,1,2,3} },
    {4        | Arange, {0,1,2,3} },
    {"2:6"    | Arange, {2,3,4,5} },
    {"1:9:2"  | Arange, {1,3,5,7} },
    {"5:1:-1" | Arange, {5,4,3,2} },
    {L{2,6}   | Arange, {2,3,4,5} },
    {L{1,9,2} | Arange, {1,3,5,7} },
    {L{5,1,-1}| Arange, {5,4,3,2} },
    {0        | Arange, L{}       },
    {"1:9:-1" | Arange, L{}       },


    {J{{"a", 1}, {"b", 2}} | Items     , {L{"a", 1}, L{"b", 2}}},
    {J{{"a", 1}, {"b", 2}} | Keys      , {"a", "b"}            },
    {J{{"a", 1}, {"b", 2}} | Values    , {1, 2}                },
    {J{1,2,3}              | Enumerate , {{0,1},{1,2},{2,3}}   },
    {J{{1,2},{3,4}}        | Flatten   , {1,2,3,4}             },
    {42                    | ToList    , L{42}                 },

    // composition
    {L{1,2} | Pipe(Size, Eq(2)), true},
    {L{1,2} | Size|Eq(2)       , true},
    {L{1,2} | Size|2           , true},

    {42 | (Size, Card), {Size, Card}},

    // arithmetic
    {42          | Neg     , -42          },
    {-42         | Neg     ,  42          },
    {42          | Sign    ,  1           },
    {-42         | Sign    , -1           },
    {42          | Abs     ,  42          },
    {-42         | Abs     ,  42          },
    {42.1        | Ceil    ,  43          },
    {42.9        | Ceil    ,  43          },
    {-42.1       | Ceil    , -42          },
    {42.1        | Floor   ,  42          },
    {42.9        | Floor   ,  42          },
    {-42.1       | Floor   , -43          },
    {42ul        | BitNot  , ~42ul        },
    {L{1ul, 2ul} | BitAnd  , 2ul & 1ul    },
    {L{1ul, 2ul} | BitOr   , 2ul | 1ul    },
    {L{1ul, 2ul} | BitXor  , 2ul xor 1ul  },
    {L{1, 1}     | Lshift  , 2            },
    {L{2, 1}     | Rshift  , 1            },
    {L{3,  2}    | Add     , 5            },
    {L{.5, 1}    | Add     , 1.5          },
    {L{.5, 1.5}  | Add     , 2            },
    {L{3, -2}    | Add     , 1            },
    {3           | Add(2)  , 5            },
    {3           | Add(-2) , 1            },
    {L{3,  2}    | Sub     , 1            },
    {L{3, -2}    | Sub     , 5            },
    {3           | Sub(2)  , 1            },
    {3           | Sub(-2) , 5            },

    // operand flip
    {L{3,  2} | ~Sub          , -1   },
    {L{3, -2} | ~Sub          , -5   },
    {3        | ~Sub(2)       , -1   },
    {3        | ~Sub(-2)      , -5   },
    {L{3,  2} | Mul           ,  6   },
    {L{3, -2} | Mul           , -6   },
    {3        | Mul(2)        ,  6   },
    {3        | Mul(-2)       , -6   },
    {3        | Mul(0.5)      , 1.5  },
    {0.5      | Mul(3)        , 1.5  },
    {L{3,  2} | Div           ,  1.5 },
    {L{3, -2} | Div           , -1.5 },
    {3        | Div(2)        ,  1.5 },
    {3        | Div(-2)       , -1.5 },
    {L{-2, 3} | Reverse | Div , -1.5 },
    {L{-2, 3} | Flip(Div)     , -1.5 },
    {L{-2, 3} | ~Div          , -1.5 },
    {L{4,  2} | Mod           ,  0   },
    {L{7,  4} | Mod           ,  3   },
    {L{7, -4} | Mod           ,  3   },
    {4        | Mod(2)        ,  0   },
    {7        | Mod(-4)       ,  3   },
    {11       | Mod(2)        ,  1   },
    {12       | Mod(2)        ,  0   },
    {13       | Mod(2)        ,  1   },
    {14       | Mod(2)        ,  0   },
    {L{4 ,  2}| Quot          ,  2   },
    {L{17, -4}| Quot          , -4   },
    {11       | Quot(2)       ,  5   },
    {7        | Quot(-4)      , -1   },
    {3        | Pow(2)        ,  9   },
    {4        | Pow(2)        , 16   },
    {9        | Pow(0.5)      ,  3   },
    {16       | Pow(0.5)      ,  4   },
    {0.25     | Pow(0.5)      ,  0.5 },
    {2        | Flip(Pow(3))  ,  9   },
    {0.5      | Flip(Pow(9))  ,  3   },
    {L{3, 2}  | Pow           ,  9   },

    {8     | Log(2)         , 3 },
    {16    | Log(2)         , 4 },
    {0.125 | Log(0.5)       , 3 },
    {0.25  | Log(0.5)       , 2 },
    {0.5   | Log(0.5)       , 1 },
    {*E | Log(*E)           , 1 },
    { E | Log( E)           , 1 },
    {2   | Flip(Log(16   )) , 4 },
    {0.5 | Flip(Log(0.125)) , 3 },
    {L{8, 2} | Log          , 3 },

    {9      | Sqrt  , 3                 },
    {16     | Sqrt  , 4                 },
    {0.25   | Sqrt  , 0.5               },
    {0      | Sin   , 0                 },
    {0      | Cos   , 1                 },
    {0      | Tan   , 0                 },
    {0      | Asin  , 0                 },
    {1      | Acos  , 0                 },
    {0      | Atan  , 0                 },
    {0      | Sinh  , 0                 },
    {0      | Cosh  , 1                 },
    {0      | Tanh  , 0                 },
    {0      | Asinh , 0                 },
    {1      | Acosh , 0                 },
    {0      | Atanh , 0                 },
    {1.0/3  | Exp   , std::exp(1.0/3)   },
    {1.0/3  | Erf   , std::erf(1.0/3)   },
    {1.0/3  | Erfc  , std::erfc(1.0/3)  },
    {1.0/3  | Gamma , std::tgamma(1.0/3)},

    // compose const and fn
    {3|Add(2)                   , 5                     },
    {3|Pow(2)                   , 9                     },

    // eval operator yields literal which interpreted as Q(x)
    {2|Add(-1)                  , 1                     },

    { 1 | Repeat(4)             , {1,1,1,1}         },
    {42 | Repeat(3)             , {42,42,42}        },
    { 1 | Repeat(3) | Repeat(2) , {{1,1,1}, {1,1,1}}},
    { 4 | ~Repeat(1)            , {1,1,1,1}         },
    { 4 | Sequence(1)           , {1,1,1,1}         },
    { 3 | ~Repeat(Pi)           , {*Pi,*Pi,*Pi}     },


    {L{2,2,2,2}        | Reduce(Add)                ,  8                    },
    {L{1,2,3,4}        | Reduce(Add)                , 10                    },
    {L{1,2,3,4}        | Push(3)|Reduce(Add)        , 13                    },
    {L{42}             | Reduce(Add)                , 42                    },
    {L{}               | Reduce(Add)                , nullptr               },
    {L{{2,2,2,2}, Add} | Reduce                     , 8                     },
    {(L{2,2,2,2}, Add) | Reduce                     , 8                     },


    // ternary and or
    {true  | And(42) | Or(13)       , 42    },
    {false | And(42) | Or(13)       , 13    },
    {true  | And(42) | Or(13) | Not , false },
    {true  | And(E)  | Or(NaN)      , *E    },

    {Q(42) | "$X" | Eq(42) | And("X = 42") | Or("$X" | ~Fmt("X = %d")), "X = 42"},
    {Q(13) | "$X" | Eq(42) | And("X = 42") | Or("$X" | ~Fmt("X = %d")), "X = 13"},

    {42 | If(42, "X = 42") | Else(~Fmt("X = %d")), "X = 42"    },
    {13 | If(42, "X = 42") | Else(~Fmt("X = %d")), "X = 13"    },

    {L{"foo", "bar"} | Push("baz")|Reduce(And)    , "bar"                 },
    {L{"foo", "bar"} | Push(""   )|Reduce(And)    , ""                    },
    {L{"foo", "bar"} | Push(42   )|Reduce(Or )    , 42                    },
    {L{"foo", "bar"} | Push(""   )|Reduce(Or )    , "foo"                 },

    {"42"                  | Re("42")             , true  },
    {"43"                  | Re("42")             , false },
    {"123"                 | Re("^.{3}$")         , true  },
    {"1234"                | Re("^.{3}$")         , false },
    {L{1,2}                | Str|Re("\\[1,2\\]")  , true  },
    {42                    | Str|Re("42")         , true  },
    {L{42, "42"}           | (At(0)|Str)&At(1)|Re , true  },
    {L{{1,2}, "\\[1,2\\]"} | (At(0)|Str)&At(1)|Re , true  },

    {"foo" | Capitalize , "Foo"                 },
    {"foo" | UpperCase  , "FOO"                 },
    {"FOO" | LowerCase  , "foo"                 },

    {L{1,2,3}              | At(0)                      , 1                     },
    {L{1,2,3}              | At(1)                      , 2                     },
    {L{1,2,3}              | At(2)                      , 3                     },
    {L{1,2,3}              | At(3)                      , nullptr               },
    {"foo"                 | At("")                     , "foo"                 },
    {42                    | At(0)                      , nullptr               },
    {J{{"a",42}, {"b",13}} | At("/a")                   , 42                    },
    {J{{"a",42}, {"b",13}} | At("/b")                   , 13                    },
    {J{{"a",42}, {"b",13}} | At({"/a", "/b"})           , {42, 13}              },
    {J{{"a",42}, {"b",13}} | At({"/a", "/b"})           , {42, 13}              },
    {J{{"a",42}, {"b",13}} | At({{"f","/a"},{"g","/b"}}), {{"f",42},{"g",13}}   },
    {J{{"a",42}, {"b",13}} | At({{"$/b","/a"}})         , {{"13",42}}           },
    {L{1,2,3,4,5,6,7,8}    | At("::2")                  , {1,3,5,7}             },
    {L{1,2,3,4,5,6,7,8}    | At("4:")                   , {5,6,7,8}             },
    {L{1,2,3,4,5,6,7,8}    | At("::-1")                 , {8,7,6,5,4,3,2,1}     },
    {L{{1,2,3}, 0}         | At                         , 1                    },
    {42                    | At({"", ""})               , {42, 42}             },
    // single-element bracket init is always an array in Expression ctor
    {42 | At({""}), L{42}},

    {L{1,2,3} | First , 1  },
    {L{1,2,3} | Last  , 3  },
    {"abc"    | First , 'a'},
    {"abc"    | Last  , 'c'},


    {42 | Delete({"/lol"}) , 42      },
    {42 | Delete("")       , nullptr },
    {42 | Delete({""})     , nullptr },
    {J{1,2,3,4,5}                 | Delete(2)                 , {1,2,4,5}               },
    {J{1,2,3,4,5}                 | Delete(-2)                , {1,2,3,5}               },
    {J{{1,2,3},4}                 | Delete("/0/1")            , {{1,3}, 4}              },
    {J{{"a", {{"b", {1,2,3,4}}}}} | Del({"/a/b/0", "/a/b/1"}) , {{"a", {{"b", {3,4}}}}} },
    {J{1,2,3,4,5}                 | Del({0, 1, 42, "/2"})     , {4,5}                   },

    { L{} | Upd(2, 42)              , L{}       },
    { L{} | Upd(-1, 42)             , L{}       },
    { L{} | Upd(0, 42)              , L{42}     },
    { L{} | Upd(0, 42) | Upd(1, 42) , L{42, 42} },
    { L{} | Upd({0, 1}, 42)         , L{42, 42} },

    { L{1,2,3,4,5} | Upd(2, 42)               , {1,2,42,4,5}         },
    { L{{1,2},3}   | Upd("/0/1", Mul(-1))     , {{1,-2},3}           },
    { L{1,2,3,4,5} | Upd(":", Mul(10))        , {10,20,30,40,50}     },
    { L{1,2,3,4,5} | Upd("::2", Mul(10))      , {10,2,30,4,50}       },
    { L{1,2,3,4,5} | Upd({1,-1}, Mul(10))     , {1,20,3,4,50}        },
    { L{} | Upd("/0/lol", D(42))              , L{{{"lol",42}}}      },
    { L{} | Upd(0, "a") | Upd("/1/lol", D(42)), L{"a", {{"lol",42}}} },

    {0    | Flip(At({1,2,3}))       , 1             },
    {0    | Lookup({1,2,3})         , 1             },
    {42   | Lookup({{"42", "lol"}}) , nullptr       },
    {0    | Lookup({{"42", "lol"}}) , {"42", "lol"} },
    {-1   | Lookup({{"42", "lol"}}) , {"42", "lol"} },
    {"42" | Lookup({{"42", "lol"}}) , "lol"         },
    {"/42"| Lookup({{"42", "lol"}}) , "lol"         },

    {1 | Lookup(Q({{1,2,3},{4,5,6}}) | At(1))   , 5},
    {L{1,2,3,4,5} | At(False | And(0) | Or(-1)) , 5},

    {"foo"    | At(0)                                 , 'f' },
    {0        | Lookup("foo")                         , 'f' },
    {1        | Lookup({1, Pi, 42})                   ,  Pi },
    {1        | Lookup({1, Pi, 42}) | Eval            , *Pi },
    {"/a/b/1" | Lookup({{"a", {{"b", {10,20}}}}})     , 20  },
    {"pi"     | Lookup({{"pi", Pi}, {"e", E}})        ,  Pi },
    {"pi"     | Lookup({{"pi", Pi}, {"e", E}}) | Eval , *Pi },

    {"abcdefg" | At("-3::")      , "efg"  },
    {"abcdefg" | At("-3:")       , "efg"  },
    {"abcdefg" | At("-1:-3:-1")  , "gf"   },
    {"abcdefg" | At("1:3:1")     , "bc"   },
    {"::2" | Lookup("abcdefg")   , "aceg" },
    {"1:42:3" | Lookup("")       , ""     },
    {"1:-1" | Lookup("abcdef")   , "bcde" },
    {"1:-1" | Lookup({0,1,2,3,4}), {1,2,3}},
    {"1:-2" | Lookup({0,1,2,3,4}), {1,2}  },


    {J{1,2,3,4}  | Map(Add(10))       , {11,12,13,14}       },
    {J{1,2,3,4}  | Map(Mod(2))        , {1,0,1,0}           },
    {J{1,4,9,16} | Map(Pow(0.5))      , {1,2,3,4}           },
    {J{1,2,3,4}  | Map(Flip(Pow(2)))  , {2,4,8,16}          },
    {J{1,2,3,4}  | Map(Flip(Pow(0.5))), {.5,.25,.125,.0625} },

    {J{1,2,3,4} | Filter(Mod(2)|0), {2,4} },
    {J{1,2,3,4} | Filter(Mod(2)|1), {1,3} },

    { 4        | Recur( 0, Add(1) ) , 4            },
    {41        | Recur(42, Add(-1)) , 1            },
    {41        | Recur(42, Sub(1) ) , 1            },
    { 4        | Recur( 2, Pow(2) ) , 65536        },
    { 4        | Unfold(0, Add(1))  , {0,1,2,3,4}  },
    { 4        | Unfold(1, Add(1))  , {1,2,3,4,5}  },
    {Q(Ge(12)) | Recur( 4, Add(1))  , 11           },
    {Q(Ge(12)) | Unfold(8, Add(1))  , {8,9,10,11}  },

    {4 | Recur(
            Q({0,0}), // init value
            (At(0) | Add(1)) & (At(1) | Sub(1)) // function
        ),
        {4, -4}
    },

    {6               | All(Gt(5), Mod(2)|0)       , true                 },
    {7               | All(Gt(5), Mod(2)|0)       , false                },
    {5               | All(Gt(5), Mod(2)|0)       , false                },
    {6               | Any(Gt(5), Mod(2)|0)       , true                 },
    {7               | Any(Gt(5), Mod(2)|0)       , true                 },
    {5               | Any(Gt(5), Mod(2)|0)       , false                },
    {42              | Any(Eq(42),Eq(13))         , true                 },
    {42              | Any(42,13)                 , true                 },
    {13              | Any(42,13)                 , true                 },
    {2               | Any(42,13)                 , false                },
    {J{1,2,3,4,5}    | Count(Mod(2)|0)            , 2                    },
    {J{1,2,3,4,5}    | Count(Mod(2)|1)            , 3                    },
    {J{1, 2, 3, 4}   | Each(Gt(2))                , false                },
    {J{1, 2, 3, 4}   | Slide(2)|Each(Lt)          , true                 },
    {J{2, 2, 2, 2}   | Each(1|Add(1))             , true                 },
    {J{2,4,8,42,1,2} | Saturate(Eq(42), Mod(2)|0) , true                 },
    {J{2,4,8,42,1,2} | Saturate(42, Mod(2)|0)     , true                 },
    {J{2,4,8,41,2}   | Saturate(42, Mod(2)|0)     , false                },
    {J{2,4,8,42}     | Saturate(42, Mod(2)|0)     , false                },

    {"Hello, "              | Concat("World!")         , "Hello, World!"      },
    {"World!"               | Flip(Concat("Hello, "))  , "Hello, World!"      },
    {L{"Hello, ", "World!"} | Concat                   , "Hello, World!"      },
    {J{1,2}                 | Concat({3,4})            , {1,2,3,4}            },

    {"Hello, World!"             | Format()                   , "Hello, World!"      },
    {"%s, %s!"                   | Format("Hello", "World")   , "Hello, World!"      },
    {"%d + %d = %d"              | Format(2,2,4)              , "2 + 2 = 4"          },
    {"list: %s"                  | Format({1,2,3})            , "list: [1,2,3]"      },
    {J{"Hello, %s!", L{"World"}} | Format                     , "Hello, World!"   },

    {"%s" | Format(Pi)      | Parse      , *Pi                  },
    {"%s" | Format(Q(Mod(3) | Eq({1,2}))), "Mod(3) | Eq([1,2])" },


    {"[1,2,3]" | Parse , J{1,2,3}             },
    {J{1,2,3}  | Str   , "[1,2,3]"            },

    {L{}                  | Transp , L{}                  },
    {L{{1,2}}             | Transp , {L{1},L{2}}          },
    {L{{1,2},{3,4},{5,6}} | Transp , {{1,3,5},{2,4,6}}    },
    {L{{1,2},{"a", "b"}}  | Transp , {{1, "a"},{2, "b"}}  },

    {L{}      | Cartesian , L{}         },
    {L{{1,2}} | Cartesian , {L{1},L{2}} },

    {L{{1,2},{"a", "b"}} | Cartesian,
                                        {{1, "a"},
                                         {1, "b"},
                                         {2, "a"},
                                         {2, "b"}}},

    {Keyword::Noop , "Noop" },
    {Noop          , true   },

    {Q(Cat({Keyword::At})) | Str, R"(Cat(["At"]))"      },
    {Q(Max({Keyword::At})) | Str, R"(Max(["At"]))"      },
    // {Cat({42, Keyword::At, precise<float>()}) | Str, R"(Cat([42,"At","0x0p+0"]))"                  },

    {2  | Add(1 | Add(1))             , 4    },
    {Pi | Cos                         , -1   },
    {Pi | Div(2) | Sin                , 1    },
    {Pi | Div(6) | Sin | Approx(0.5)  , true },
    {Pi | Div(4) | Tan | Approx(1)    , true },

    { 42   | Try(Div(0)) | D(13)         , 13       },
    {42.5  | Try(Cast(type<int>))        , 42       },
    {42.5  | Try(Cast(type<std::string>)), nullptr  },
    {0.2   | Cast(type<float>)           , 0.2f     },
    {0.1   | Cast(type<float>)           , 0.1f     },
    {~0ULL | Cast(type<std::uint8_t>)    , 255      },

    {int(Foo::Bar) | Cast(type<Foo>), "Bar" },
    {int(Foo::Baz) | Cast(type<Foo>), "Baz" },
    // Foo type is lost on serialization to string
    {Foo::Bar | Cast(type<int>) | IsErr, true          },
    {Foo::Bar | Cast(Underlying<Foo>)  , int(Foo::Bar) },
    {Foo::Baz | Cast(Underlying<Foo>)  , int(Foo::Baz) },

    { 0.1 | Cast(Precise<double>)                   , 0.1     },
    {"0x1.999999999999ap-4" | Cast(Precise<double>) , 0.1     },
    {"0x1.999999999999ap-4" | Cast(Precise<float>)  | IsErr , true    },
    {0.1  | Cast(Precise<float>)  | IsErr           , true    },
    {0.1f | Cast(Precise<float>)                    , 0.1f    },


    {42 | Bind(Add)     , Add(42) },
    {42 & Q(Add) | Bind , Add(42) },

    {42 | Id & Q(Add(1)) | Bind(Pipe)           , 42 | Add(1)                   },
    {42 | Id & Q(Add(1)) | Bind(Pipe) | Eval    , 43                            },
    {42 | Id & Q(Add(1)) | Bind(Pipe)           , 42 | Add(1)                   },
    {42 | Id & Q(Add(1)) | Bind(Pipe) | Eval    , 43                            },
    { Q(Fmt) | (ToList & Id) | Bind             , Fmt(Fmt)                      },
    {(42, Add(1), Sub(1), Mul(1)) | Bind(Pipe)  , 42 | Add(1) | Sub(1) | Mul(1) },
    { Q(Fmt) | Bind(Q) | Bind(Q)                , Q(Q(Fmt))                     },
    { 3 | Recur(Q(Fmt), Bind(Q))                , Q(Q(Q(Fmt)))                  },

    {41 | If(Eq(41), 0) | Elif(Ge(43), 14) | Else(11), 0},
    {42 | If(Eq(41), 0) | Elif(Ge(43), 14) | Else(11), 11},
    {43 | If(Eq(41), 0) | Elif(Ge(43), 14) | Else(11), 14},

    {41 | If(Eq(41), 0)                | Id | IsErr, true },
    {41 | If(Ne(41), 0)                | Id | IsErr, true },
    {11 | If(Eq(41), 0) | Elif(11, 42) | Id | IsErr, true },
    {12 | If(Ne(41), 0) | Elif(11, 42) | Id | IsErr, true },
    {12 | Else(42) | IsErr                         , true },

    // Lispy ternary if
    {6 | If(Gt(7), Id, Add(1)), 7},
    {8 | If(Gt(7), Id, Add(1)), 8},

    {67| If(Mod(3)|Nil, Mul(10)) | Elif(Mod(5)|Nil, Mul(100)) | Else(Id),  67 },
    {9 | If(Mod(3)|Nil, Mul(10)) | Elif(Mod(5)|Nil, Mul(100)) | Else(Id), 90  },
    {25| If(Mod(3)|Nil, Mul(10)) | Elif(Mod(5)|Nil, Mul(100)) | Else(Id), 2500},



    {Eval                 , {}   },
    {42 | Eval            , 42   },
    {Q(Sub(2)) | Eval(42) , 40   },
    {Q(Eq(nullptr)) | Eval, true },

    {Dbg(42|Trace(HAIKAN_CUR_LOC)|Sub(2)) , 40      },
    {42 | Debug(Sub(2))                   , 40      },
    {42 | Debug                           , nullptr },
    {40 | Debug(Trace("foo")|Add(2)|Debug(Trace("bar")|Sub(2))), 40},

    {Q(Fold(Add)) | Kwrd     , "Fold"     },
    {Q(Fold(Add)) | Prms     , Tuple(Add) },
    {Q(Pipe(Add, Sub)) | Prms, (Add, Sub) },

    {42       | Op(type<unsigned>, Eq(42))            , true                },
    {-42      | Try(Op(type<unsigned>, Eq(42)))       , nullptr             },
    {J{.5, 2} | Op(type<std::complex<double>>, Add(1)), {1.5, 2}            },

    {41     | Op(type<int>, Eq(42)  | Not | Not)             , false }, // overload by default ignores boolean keywords
    {41     | Op(type<int>, Add(1)) | Str | Eq("42")         ,  true },
    {41     | Op(type<int>, Add(1)  | Str | Op("", Eq("42"))),  true }, // nested

    {Error("foo")       , Err({{"message", "foo"}})                   },
    {Error("foo", "bar"), Err({{"message", "foo"},{"context", "bar"}})},
    {Error(type<std::runtime_error>, "foo", "bar"),
        Err({
            {"type"   , "std::runtime_error"},
            {"message", "foo"               },
            {"context", "bar"               }
        })
    },

    {0 | Assert(Ne(0)) | Flip(Div(1)), Err("assertion failure", "Ne(0)")},
    {1 | Assert(Ne(0)) | Flip(Div(1)), 1                                },
    {Error("foo") | IsErr            , true                             },

    {PreProc(1)             , "$[1]"   },
    {PreProc("lol")         , "$[lol]" },

    {Q({{"foo", {{"bar", "baz"}}}}) | FindPtr("baz") , "/foo/bar" },
    {Q({{"foo", {{"bar", "baz"}}}}) | Find(Size|3)   , "baz"      },


    {Q({{"foo", {{"bar", "baz"}}}}) | FindIdx(At(0)|"foo") , 0    },
    {Q({{"foo", {{"bar", "baz"}}}}) | FindIdx(At(0)|"foo") , 0    },

    {"abcd" | FindIdx('d')      , 3 },

    {42     | Link("$x")        , 42},
    {Q(42)  | "$x"              , 42},
    {Q(3)   | "$x" | Mul("$x")  , 9 },

    {42 | ("$x" | Eq(42))                 , true         },
    {42 | ("$x" | Eq(42)) | Id & "$x"     , {true, true} },
    {42 | ("$x" | Eq(42)) | Id & Get("$x"), {true, 42}   },
    {Q(42) | ("$f" << Add(1))             , 43           },

    // Side effects!
    {EnvLoad("/foo")        , nullptr         },
    {42 | EnvStore("/foo")  , 42              },
    {EnvLoad("/foo")        , 42              },
    {EnvLoad("foo")         , 42              },
    {43 | EnvStore("foo")   , 43              },
    {EnvLoad("foo")         , 43              },
    {EnvLoad("/foo")        , 43              },
    {EnvStore | IsErr       , true            },
    {EnvLoad  | IsErr       , true            },
    {EnvLoad("")            , {{"foo", 43}}   },
    {EnvStore("") | IsErr   , true            },
    {EnvLoad("")            , {{"foo", 43}}   },
};

    return samples;
}
