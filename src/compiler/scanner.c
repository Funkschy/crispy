#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "../cli/common.h"

static bool at_end(Scanner *scanner);

static Token make_token(Scanner *scanner, TokenType type);

static Token error_token(const char *message);

static char advance(Scanner *scanner);

static void skip_whitespace(Scanner *scanner);

static bool is_digit(char c);

static Token number(Scanner *scanner);

static bool is_alpha(char c);

static Token identifier(Scanner *scanner);

static TokenType identifier_type(Scanner *scanner);

static TokenType check_keyword(Scanner *scanner, int start, size_t rest_length, const char *rest, TokenType type);

void init_scanner(Scanner *scanner, const char *source) {
    scanner->start = source;
    scanner->current = source;
}

Token scan_token(Scanner *scanner) {
    skip_whitespace(scanner);

    scanner->start = scanner->current;

    if (at_end(scanner)) return make_token(scanner, TOKEN_EOF);

    char c = advance(scanner);

    if (is_digit(c)) return number(scanner);
    if (is_alpha(c)) return identifier(scanner);

    switch (c) {
        case '(':
            return make_token(scanner, TOKEN_OPEN_PAREN);
        case ')':
            return make_token(scanner, TOKEN_CLOSE_PAREN);
        case '+':
            return make_token(scanner, TOKEN_PLUS);
        case '-':
            return make_token(scanner, TOKEN_MINUS);
        case '*':
            return make_token(scanner, TOKEN_STAR);
        case '/':
            return make_token(scanner, TOKEN_SLASH);
        case '=':
            return make_token(scanner, TOKEN_EQUALS);
        case ';':
            return make_token(scanner, TOKEN_SEMICOLON);
        default:
            return error_token("Unexpected Character");
    }
}

static bool is_alpha(char c) {
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

static inline char peek(Scanner *scanner) {
    return *scanner->current;
}

static inline char peek_next(Scanner *scanner) {
    if (at_end(scanner)) return '\0';
    return scanner->current[1];
}

static Token number(Scanner *scanner) {
    while (is_digit(peek(scanner))) advance(scanner);

    if (peek(scanner) == '.' && is_digit(peek_next(scanner))) {
        advance(scanner);
        while (is_digit(peek(scanner))) advance(scanner);
    }

    return make_token(scanner, TOKEN_NUMBER);
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static void skip_whitespace(Scanner *scanner) {
    while (true) {
        char c = *scanner->current;

        switch (c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                advance(scanner);
                break;
            default:
                return;
        }
    }
}

static char advance(Scanner *scanner) {
    scanner->current++;
    return scanner->current[-1];
}

static Token error_token(const char *message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int) strlen(message);

    return token;
}

static Token make_token(Scanner *scanner, TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (unsigned int) (scanner->current - scanner->start);

    return token;
}

static bool at_end(Scanner *scanner) {
    return *scanner->current == '\0';
}

static Token identifier(Scanner *scanner) {
    while (is_alpha(peek(scanner)) || is_digit(peek(scanner))) advance(scanner);
    return make_token(scanner, identifier_type(scanner));
}

static TokenType identifier_type(Scanner *scanner) {
    switch (*scanner->start) {
        case 'v':
            return check_keyword(scanner, 1, 2, "ar", TOKEN_VAR);
        default:
            return TOKEN_IDENTIFIER;
    }
}

static TokenType check_keyword(Scanner *scanner, int start, size_t rest_length, const char *rest, TokenType type) {
    if (scanner->current - scanner->start == start + rest_length
        && memcmp(scanner->start + start , rest, rest_length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}
