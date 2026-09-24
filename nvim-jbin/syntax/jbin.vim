" Vim syntax file
" Language: jbin schema

if exists("b:current_syntax")
  finish
endif

" Keywords
syntax keyword jbinKeyword package message enum end

" Core Types
syntax keyword jbinType i32 i64 bool string list map

" Numbers (Field IDs)
syntax match jbinNumber "\v<\d+>"

" Identifiers 
" Capitalized words are usually custom messages/enums
syntax match jbinCustomType "\v<[A-Z][a-zA-Z0-9_]*>"

" Strings (for package names)
syntax region jbinString start=/"/ skip=/\\./ end=/"/

" Comments
syntax match jbinComment "\v//.*$"

" Map to standard highlight groups
highlight default link jbinKeyword Keyword
highlight default link jbinType Type
highlight default link jbinNumber Number
highlight default link jbinCustomType Structure
highlight default link jbinString String
highlight default link jbinComment Comment

let b:current_syntax = "jbin"
