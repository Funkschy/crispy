// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CALC_SCANNER_H
#define CALC_SCANNER_H

#include "../cli/common.h"

typedef enum {
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_DEC_NUMBER,
    TOKEN_HEX_NUMBER,
    TOKEN_STRING,
    TOKEN_PLUS,
    TOKEN_PLUS_PLUS,
    TOKEN_MINUS,
    TOKEN_MINUS_MINUS,
    TOKEN_STAR,
    TOKEN_STAR_STAR,
    TOKEN_SLASH,
    TOKEN_VAR,
    TOKEN_VAL,
    TOKEN_EQUALS,
    TOKEN_IDENTIFIER,
    TOKEN_SEMICOLON,
    TOKEN_WHILE,
    TOKEN_EQUALS_EQUALS,
    TOKEN_BANG_EQUALS,
    TOKEN_BANG,
    TOKEN_OPEN_BRACE,
    TOKEN_CLOSE_BRACE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FUN,
    TOKEN_ARROW,
    TOKEN_COMMA,
    TOKEN_PERCENT,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NIL,
    TOKEN_RETURN,
    TOKEN_SMALLER,
    TOKEN_GREATER,
    TOKEN_SMALLER_EQUALS,
    TOKEN_GREATER_EQUALS,
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,

    TOKEN_ERROR,
    TOKEN_EOF,
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    size_t length;
    uint32_t line;
} Token;

typedef struct {
    const char *start;
    const char *current;

    Token previous;

    uint32_t line;
} Scanner;

void init_scanner(Scanner *scanner, const char *source);
Token scan_token(Scanner *scanner);

#endif //CALC_SCANNER_H
