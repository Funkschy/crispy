// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <string.h>

#include "../cli/common.h"
#include "scanner.h"

static bool at_end(Scanner *scanner) {
    return *scanner->current == '\0';
}

static char advance(Scanner *scanner) {
    scanner->current++;
    return scanner->current[-1];
}

static inline char peek(Scanner *scanner) {
    return *scanner->current;
}

static inline char peek_next(Scanner *scanner) {
    if (at_end(scanner)) { return '\0'; }
    return scanner->current[1];
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static Token make_token(Scanner *scanner, TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (unsigned int) (scanner->current - scanner->start);
    token.line = scanner->line;

    return token;
}

void init_scanner(Scanner *scanner, const char *source) {
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
    scanner->previous = make_token(scanner, TOKEN_ERROR);
}

static Token number(Scanner *scanner) {
    while (is_digit(peek(scanner))) { advance(scanner); }

    if (peek(scanner) == '.' && is_digit(peek_next(scanner))) {
        advance(scanner);
        while (is_digit(peek(scanner))) { advance(scanner); }
    }

    return make_token(scanner, TOKEN_NUMBER);
}

/**
 * Skips the whitespace until the next token starts.
 * @param scanner the scanner.
 * @return true if a semicolon has to be inserted;
 */
static bool skip_whitespace(Scanner *scanner) {
    while (true) {
        char c = *scanner->current;

        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(scanner);
                break;
            case '\n':
                if (scanner->previous.type == TOKEN_RETURN) {
                    advance(scanner);
                    return true;
                }

                ++scanner->line;
                advance(scanner);
                break;
            case '/':
                if (peek_next(scanner) == '/') {
                    while (!at_end(scanner) && peek(scanner) != '\n') {
                        advance(scanner);
                    }
                } else {
                    return false;
                }
                break;
            default:
                return false;
        }
    }
}

static Token error_token(Scanner *scanner, const char *message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int) strlen(message);
    token.line = scanner->line;

    return token;
}

static TokenType check_keyword(Scanner *scanner, int start, size_t rest_length, const char *rest, TokenType type) {
    if (scanner->current - scanner->start == start + rest_length
        && memcmp(scanner->start + start, rest, rest_length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Scanner *scanner) {
    switch (*scanner->start) {
        case 'v':
            if (scanner->current - scanner->start > 1 && scanner->start[1] == 'a') {
                switch (scanner->start[2]) {
                    case 'r':
                        return check_keyword(scanner, 2, 1, "r", TOKEN_VAR);
                    case 'l':
                        return check_keyword(scanner, 2, 1, "l", TOKEN_VAL);
                    default:
                        break;
                }
            }
            break;
        case 'w':
            return check_keyword(scanner, 1, 4, "hile", TOKEN_WHILE);
        case 'i':
            return check_keyword(scanner, 1, 1, "f", TOKEN_IF);
        case 'e':
            return check_keyword(scanner, 1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'u':
                        return check_keyword(scanner, 2, 1, "n", TOKEN_FUN);
                    case 'a':
                        return check_keyword(scanner, 2, 3, "lse", TOKEN_FALSE);
                    default:
                        break;
                }
            }
        case 't':
            return check_keyword(scanner, 1, 3, "rue", TOKEN_TRUE);
        case 'r':
            return check_keyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
        case 'n':
            return check_keyword(scanner, 1, 2, "il", TOKEN_NIL);
        case 'o':
            return check_keyword(scanner, 1, 1, "r", TOKEN_OR);
        case 'a':
            return check_keyword(scanner, 1, 2, "nd", TOKEN_AND);
        default:
            return TOKEN_IDENTIFIER;
    }

    return TOKEN_IDENTIFIER;
}

static Token identifier(Scanner *scanner) {
    while (is_alpha(peek(scanner)) || is_digit(peek(scanner))) { advance(scanner); }
    return make_token(scanner, identifier_type(scanner));
}

static Token string(Scanner *scanner) {
    while (!at_end(scanner) && peek(scanner) != '"') {
        advance(scanner);
    }

    if (at_end(scanner)) {
        return error_token(scanner, "Unterminated String");
    }

    advance(scanner);
    return make_token(scanner, TOKEN_STRING);
}

static Token get_next_token(Scanner *scanner) {
    scanner->start = scanner->current;

    if (at_end(scanner)) { return make_token(scanner, TOKEN_EOF); }

    char c = advance(scanner);

    if (is_digit(c)) { return number(scanner); }
    if (is_alpha(c)) { return identifier(scanner); }

    switch (c) {
        case '(':
            return make_token(scanner, TOKEN_OPEN_PAREN);
        case ')':
            return make_token(scanner, TOKEN_CLOSE_PAREN);
        case '+':
            if (peek(scanner) == '+') {
                advance(scanner);
                return make_token(scanner, TOKEN_PLUS_PLUS);
            }
            return make_token(scanner, TOKEN_PLUS);
        case '-':
            if (peek(scanner) == '>') {
                advance(scanner);
                return make_token(scanner, TOKEN_ARROW);
            } else if (peek(scanner) == '-') {
                advance(scanner);
                return make_token(scanner, TOKEN_MINUS_MINUS);
            }
            return make_token(scanner, TOKEN_MINUS);
        case '*':
            return make_token(scanner, TOKEN_STAR);
        case '%':
            return make_token(scanner, TOKEN_PERCENT);
        case '/':
            return make_token(scanner, TOKEN_SLASH);
        case '{':
            return make_token(scanner, TOKEN_OPEN_BRACE);
        case '}':
            return make_token(scanner, TOKEN_CLOSE_BRACE);
        case ',':
            return make_token(scanner, TOKEN_COMMA);
        case '=':
            if (peek(scanner) == '=') {
                advance(scanner);
                return make_token(scanner, TOKEN_EQUALS_EQUALS);
            }
            return make_token(scanner, TOKEN_EQUALS);
        case '!':
            if (peek(scanner) == '=') {
                advance(scanner);
                return make_token(scanner, TOKEN_BANG_EQUALS);
            } else {
                return make_token(scanner, TOKEN_BANG);
            }
        case ';':
            return make_token(scanner, TOKEN_SEMICOLON);
        case '"':
            return string(scanner);
        case '<':
            if (peek(scanner) == '=') {
                advance(scanner);
                return make_token(scanner, TOKEN_SMALLER_EQUALS);
            }
            return make_token(scanner, TOKEN_SMALLER);
        case '>':
            if (peek(scanner) == '=') {
                advance(scanner);
                return make_token(scanner, TOKEN_GREATER_EQUALS);
            }
            return make_token(scanner, TOKEN_GREATER);
        case ':':
            return make_token(scanner, TOKEN_COLON);
        case '.':
            return make_token(scanner, TOKEN_DOT);
        case '[':
            return make_token(scanner, TOKEN_OPEN_BRACKET);
        case ']':
            return make_token(scanner, TOKEN_CLOSE_BRACKET);
        default:
            return error_token(scanner, "Unexpected Character");
    }
}

Token scan_token(Scanner *scanner) {
    bool insert_semi = skip_whitespace(scanner);

    if (insert_semi) {
        return make_token(scanner, TOKEN_SEMICOLON);
    }

    Token next = get_next_token(scanner);
    scanner->previous = next;
    return next;
}
