/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/expression_lua_grammar.hpp"
#include "haikan/impl/keyword_grammar.hpp"
#include "haikan/impl/keyword_to_str.hpp"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

namespace haikan {

boost::json::string_view ExpressionLua::keyword_to_str() const
{
    return ::haikan::impl::keyword_to_str(keyword());
}

std::string ExpressionLua::prettify() const
{
    std::string out;
    std::back_insert_iterator<std::string> sink(out);

    impl::ExpressionLuaGrammar<std::back_insert_iterator<std::string>> gen;
    boost::spirit::karma::generate(sink, gen, *this);

    return out;
}


std::ostream& ExpressionLua::prettify_to(std::ostream& os) const
{
    boost::spirit::ostream_iterator sink(os);
    impl::ExpressionLuaGrammar<boost::spirit::ostream_iterator> gen;
    boost::spirit::karma::generate(sink, gen, *this);

    return os;
}


void ExpressionLua::prettify_to(char* buff, std::size_t n) const
{
    boost::iostreams::stream<boost::iostreams::array_sink> stream(buff, n);
    boost::spirit::ostream_iterator sink(stream);
    impl::ExpressionLuaGrammar<boost::spirit::ostream_iterator> gen;
    boost::spirit::karma::generate(sink, gen, *this);

    stream.flush();
    std::size_t written = stream.tellp();
    buff[std::min(written, n - 1)] = '\0';
    return;
}

std::ostream& operator<<(std::ostream& os, ExpressionLua const& expr)
{
    return expr.prettify_to(os);
}

// haikan::Logger& operator<<(haikan::Logger& logger, ExpressionLua const& expr)
// {
//     logger << static_cast<boost::json::value>(expr.prettify());
//     return logger;
// }

// Boost JSON conversion from ExpressionLua
// void tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, ExpressionLua const& ev)
// {
//     v = ev.to_json();
// }

} // namespace haikan
