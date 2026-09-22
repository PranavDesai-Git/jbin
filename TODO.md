### TODO
- [ ] C struct codegen
- [ ] C++ struct codegen
- [ ] Python codegen
- [ ] JS codegen
- [ ] Runtime FFI bindings / getters

### DOING
- [ ] Codegen abstraction (Visitor pattern for multi-lang)
### DONE
- [x] Dynamic JSON Binary Packer (serialize JSON to .jbin dynamically)
- [x] CLI Interface (`jbin build schema.jbin --out c`)
- [x] Dynamic JSON Binary Reader (deserialize .jbin to JSON)
- [x] Lexer (with string literals, map, union, optional)
- [x] Parser (Recursive Descent AST builder)
- [x] Encode/Decode String
- [x] Encode/Decode Tag
- [x] Encode/Decode Varint
- [x] Semantic Analyzer
    - [x] Symbol Table generation
    - [x] Type resolution (check if custom types exist)
    - [x] Validation: Map keys must be scalar
    - [x] Validation: No duplicate field numbers in a message
