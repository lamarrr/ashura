
## C++ Rules

- No exceptions (like, *never*), use `Result<T, E>`
- No `new`, `delete`, `malloc`, or `free`, use the container types (i.e. `Vec`, `PinVec`, `List`), handle types (`Rc`, `Unique`), or the provided allocator API
- No RTTI (`dynamic_cast`, `typeid`, `typeinfo`)
- No multiple inheritance of types sharing common base classes
- No constructors with side-effects, constructors must only copy bytes
- Constructors requiring complex operations to construct members must be split into separate helper functions
- Avoid implicit conversions where possible
- Functions using/allocating memory must take an allocator argument
- Use the provided standard library facilities
- When writing or wrapping one-time C code use the `defer` facility to manage resources, instead of manually cleaning up resources on each path
- Use templates sparingly, only when there's no other way to solve the problem
- Order structs and classes based on their access patterns and cache associativity
