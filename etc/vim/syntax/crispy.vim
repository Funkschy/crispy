" Vim syntax file
" Language: Crispy
" Maintainer: Felix Schoeller
" Latest Revision: 28.06.2018

if exists("b:current_syntax")
	finish
endif

" Keywords and Storage Classes
syntax keyword crispyStorageClass var val
syntax keyword crispyKeyword return or and import in continue
syntax keyword crispyKeyword while if else fun break for

" Booleans and nil
syntax keyword crispyBoolean true false nil

" Operators
syntax match crispyOperator "\v\*"
syntax match crispyOperator "\v\+"
syntax match crispyOperator "\v-"
syntax match crispyOperator "\v\%"
syntax match crispyOperator "\v/"
syntax match crispyOperator "\v\="
syntax match crispyOperator "\v-\>"
syntax match crispyOperator "\v\=\="
syntax match crispyOperator "\v\!\="
syntax match crispyOperator "\v\<"
syntax match crispyOperator "\v\>"
syntax match crispyOperator "\v\<\="
syntax match crispyOperator "\v\>\="

" Comments
syntax match crispyComment "\v\/\/.*$"
syntax region crispyBlockComment start=/\v\/\*/ skip=/\v\\./ end=/\v\*\//

" Numbers
syntax match crispyNumber "\v-?[0-9]*\.?[0-9]+"
syntax match crispyNumber "\v-?0x\x+"

" Strings
syntax region crispyDoubleString start=/\v"/ skip=/\v\\./ end=/\v"/ 
syntax region crispySingleString start=/\v'/ skip=/\v\\./ end=/\v'/ 

highlight link crispyStorageClass StorageClass
highlight link crispyKeyword Keyword
highlight link crispyBoolean Boolean
highlight link crispyOperator Operator 
highlight link crispyComment Comment
highlight link crispyBlockComment Comment
highlight link crispyNumber Number
highlight link crispyDoubleString String
highlight link crispySingleString String

let b:current_syntax = "crispy"
