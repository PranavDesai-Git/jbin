# jbin

jbin is a binary serialization format that removes the boilerplate and legacy overhead of older systems (like Protobuf). It uses a clean, modern schema language and features dynamic JSON-to-binary packing.

## Schema Example

```
package "com.game.core"
import "math.jbin"

enum Activity:
    1. active
    2. inactive
end

message Player:
    1. name: string
    2. health: i32 = 100
    3. weapons: list(string)
    4. connections: list(Player)
    5. activeStatus: Activity
    6. inventory: map(string, i32)
    7. balance: union(string, i32) = "empty"
end
```

## Built-in Types
- **Primitives**: `i32`, `i64`, `f32`, `f64`, `bool`
- **Data**: `bytes`, `string`
- **Collections**: `list(T)`
- **Maps**: `map(K, V)` *(Keys must be scalar: string, i32, or i64)*
- **Unions**: `union(T1, T2, ...)`

## Features
- **Clean Developer Experience**: No wrappers required for optionals or strings.
- **Dynamic Packing**: Instantly serialize/deserialize raw JSON against a schema without needing ahead-of-time code generation.
- **Semantic Validation**: Built-in checks for circular dependencies, invalid default values, and duplicate tags.
