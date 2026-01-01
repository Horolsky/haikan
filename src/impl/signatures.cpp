/**
 * @file
 * @copyright (c) Copyright 2024-2025 Zenseact AB
 * @license SPDX-License-Identifier: Apache-2.0
 */


#include "haikan/impl/signatures.hpp"
#include "haikan/keywords.hpp"


namespace
{
    bool maybe_kv_pair(boost::json::value const& val)
    {
        if (!val.is_array())
        {
            return false;
        }
        auto const& pair = val.get_array();
        if (pair.size() != 2)
        {
            return false;
        }
        return pair.at(0).is_string();
    }

    bool maybe_object(boost::json::array const& value_list)
    {
        return value_list.cend() == std::find_if_not(value_list.cbegin(), value_list.cend(), maybe_kv_pair);
    }
} // namespace


namespace haikan {
namespace impl {
boost::json::value handle_list_init(std::initializer_list<Expression> set)
{
    // this cause a recursive call to Expression template ctor
    // auto const value_list = boost::json::value_from(set).as_array();

    boost::json::array value_list{};
    value_list.reserve(set.size());
    std::transform(set.begin(), set.end(),
               std::back_inserter(value_list),
               [](Expression const& v) { return v.to_json(); });

    if (maybe_object(value_list))
    {
        boost::json::object out{};
        out.reserve(value_list.size());
        for (auto const& pair: value_list)
        {
            auto const& kv = pair.get_array();
            out.insert_or_assign(kv.at(0).as_string(), kv.at(1));
        }
        return out;
    }
    else
    {
        return value_list;
    }
}

void SignatureErr::make_throw_action(std::string const key, std::function<void()> f) const
{
    static_cast<void>(key);
    static_cast<void>(f);
    // Environment().RegisterAction(key, f);
}

Expression SignatureErr::make_error(boost::json::string_view type, boost::json::string_view msg, boost::json::string_view ctx) const
{
    boost::json::object err{};
    if (not type.empty())
    {
        err["type"] = type;
    }
    if (not msg.empty())
    {
        err["message"] = msg;
    }
    if (not ctx.empty())
    {
        err["context"] = ctx;
    }
    return Expression(Expression::encodeNested(Keyword::Err, {err}));
}

} // namespace impl
} // namespace haikan
