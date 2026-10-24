#include "cow.hpp"

namespace
{

std::string const kScriptLua = R"(
function create_deep_cow(original, tracked)
    tracked = tracked or {}

    if tracked[original] then
        return tracked[original]
    end

    local delta = {}
    local deleted = {}
    local proxies = {}
    local proxy = {}

    local mt = {
        __index = function(tbl, key)
            if key == tbl then
                return {
                    delta = delta,
                    deleted = deleted,
                    proxies = proxies,
                    proxy = proxy,
                }
            elseif deleted[key] ~= nil then return nil
            elseif delta[key] ~= nil then return delta[key] end

            local val = original[key]

            if type(val) == "table" then
                -- Reuse the existing proxy from our cache or create a new one
                if not proxies[key] then
                    proxies[key] = create_deep_cow(val, tracked)
                end
                return proxies[key]
            end
            return val
        end,
        __newindex = function(tbl, key, value)
            if key == tbl then
                -- pass
            elseif value == nil then
                delta[key] = nil
                deleted[key] = 1
            elseif type(value) == "table" then
                delta[key] = create_deep_cow(value, {})
                deleted[key] = nil
            else
                delta[key] = value
                deleted[key] = nil
            end
        end,
        __pairs = function (_)
            local delta_phase = true
            local delta_key -- delta key
            local orig_iter
            local orig_state
            local orig_ctl
            local omt = getmetatable(original)
            if original[original] and original[original].proxy == original and omt.__pairs then
                orig_iter, orig_state, orig_ctl = omt.__pairs(original)
            else
                orig_iter = next
                orig_state = original
                orig_ctl = nil
            end
            return function()
                if delta_phase then
                    local k, v = next(delta, delta_key)
                    delta_key = k
                    if k ~= nil then
                        return k, v
                    end
                    delta_phase = false
                end

                local k, v = orig_iter(orig_state, orig_ctl)
                while (k ~= nil) and ((delta[k] ~= nil) or (deleted[k] ~= nil)) do
                    k, v = orig_iter(orig_state, k)
                end
                orig_ctl = k
                if k == nil then
                    return nil
                end

                if type(v) == "table" then
                    local prox = proxies[k]
                    if not prox then
                        prox = create_deep_cow(v, tracked)
                        proxies[k] = prox
                    end
                    v = prox
                end
                return k, v
            end
        end,
        -- __tostring = function ()
        --     return pretty_value(proxy, 0, {})
        -- end
    }
    -- mt[mt] = {
    --     delta = delta,
    --     deleted = deleted,
    --     proxies = proxies,
    --     proxy = proxy,
    -- }

    local p = setmetatable(proxy, mt)
    tracked[original] = p -- Store identity to break cycles
    return p
end
)";
} // namespace


lauseke::impl::Cow::Cow(lua_State* L)
    : lua_{L}
{
    lua_.script(kScriptLua);
    make_cow_ = lua_["create_deep_cow"];
}
