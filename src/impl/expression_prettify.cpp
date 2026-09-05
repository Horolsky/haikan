/**
 * \file
 * \copyright (c) Copyright 2024-2025 Zenseact AB
 * \license SPDX-License-Identifier: Apache-2.0
 */

#include "haikan/impl/expression_grammar.hpp"
#include "haikan/impl/keyword_grammar.hpp"

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

namespace haikan {
namespace impl {

std::string ExpressionView::prettify() const
{
    std::string out;
    std::back_insert_iterator<std::string> sink(out);

    ExpressionGrammar<std::back_insert_iterator<std::string>> gen;
    boost::spirit::karma::generate(sink, gen, *this);

    return out;
}


std::ostream& ExpressionView::prettify_to(std::ostream& os) const
{
    boost::spirit::ostream_iterator sink(os);
    ExpressionGrammar<boost::spirit::ostream_iterator> gen;
    boost::spirit::karma::generate(sink, gen, *this);

    return os;
}


void ExpressionView::prettify_to(char* buff, std::size_t n) const
{
    boost::iostreams::stream<boost::iostreams::array_sink> stream(buff, n);
    boost::spirit::ostream_iterator sink(stream);
    ExpressionGrammar<boost::spirit::ostream_iterator> gen;
    boost::spirit::karma::generate(sink, gen, *this);

    stream.flush();
    std::size_t written = stream.tellp();
    buff[std::min(written, n - 1)] = '\0';
    return;
}

std::ostream& operator<<(std::ostream& os, ExpressionView const& expr)
{
    return expr.prettify_to(os);
}

// haikan::Logger& operator<<(haikan::Logger& logger, ExpressionView const& expr)
// {
//     logger << static_cast<boost::json::value>(expr.prettify());
//     return logger;
// }

// Boost JSON conversion from ExpressionView
void tag_invoke(boost::json::value_from_tag const&, boost::json::value& v, ExpressionView const& ev)
{
    v = ev.to_json();
}

} // namespace impl
} // namespace haikan
