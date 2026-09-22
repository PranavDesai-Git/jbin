### TODO
- [ ] CLI Interface (`jbin build schema.jbin --out c`)
- [ ] Dynamic JSON Binary Packer (serialize JSON to .jbin dynamically)
- [ ] Dynamic JSON Binary Reader (deserialize .jbin to JSON)
- [ ] Codegen abstraction (Visitor pattern for multi-lang)
- [ ] C struct codegen
- [ ] C++ struct codegen
- [ ] Python codegen
- [ ] JS codegen
- [ ] Runtime FFI bindings / getters

### DOING
- [ ] Semantic Analyzer
    - [x] Symbol Table generation
    - [ ] Type resolution (check if custom types exist)
    - [ ] Validation: Map keys must be scalar
    - [ ] Validation: No duplicate field numbers in a message

### DONE
- [x] Lexer (with string literals, map, union, optional)
- [x] Parser (Recursive Descent AST builder)
- [x] Encode/Decode String
- [x] Encode/Decode Tag
- [x] Encode/Decode Varint
