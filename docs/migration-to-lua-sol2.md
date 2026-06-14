# Migration from Boost.JSON to Lua + Sol2 / LuaJIT FFI cdata

## Goals
- Replace dynamic JSON allocation (fragmentation bottleneck) with Lua userdata / FFI cdata for zero-copy access to pre-collected C++ data.
- Keep naive AST evaluation for precise data-flow traceability (critical for test assertions).
- Pseudo-JSON frontend via Operator adapter (string keys, implicit types).
- Hybrid: FFI cdata for input/base data (zero-copy), Lua tables for transformation results.
- Use LuaJIT FFI where possible for near-native field access speed.
- One Lua state per context for pooled allocation and simpler management.
- COW only if needed for large shared data (via custom userdata); pure userdata preferred for read-heavy assertions.

## Design
- Input data: bind C++ structs as Lua userdata or FFI cdata (ffi.cast to existing memory).
- Access: Operator adapter provides pseudo-JSON interface (__index for string keys/indices).
- Evaluation: AST nodes operate on sol::object or custom LuaValue wrapper.
- New data from transforms: create Lua tables (fast, GC'd).
- Sol2 for nice C++ binding; FFI for hot zero-copy paths.
- Traceability: AST nodes keep source info; no bytecode VM yet.

## Next steps (incremental commits)
1. Add Lua state manager and basic FFI cdata wrapper.
2. Implement Operator adapter for pseudo-JSON on userdata/cdata.
3. Refactor ExpressionView / eval to use new data type.
4. Update operators, json_* files to hybrid model.
5. Remove or optionalize boost.json dependency.
6. Tests and benchmarks.

This branch tracks divergence from init-project for reviewable progress.