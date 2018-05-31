#ifndef CALC_SCANNER_H
#define CALC_SCANNER_H

typedef struct {
    const char *start;
    const char *current;
} Scanner;

typedef enum {
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_VAR,
    TOKEN_EQUALS,
    TOKEN_IDENTIFIER,
    TOKEN_SEMICOLON,
    TOKEN_PRINT,

    TOKEN_ERROR,
    TOKEN_EOF,
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    size_t length;
} Token;

void init_scanner(Scanner *scanner, const char *source);
Token scan_token(Scanner *scanner);

#endif //CALC_SCANNER_H
