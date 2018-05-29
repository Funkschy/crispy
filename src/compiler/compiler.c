#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "../cli/common.h"
#include "scanner.h"
#include "../vm/opcode.h"

static void term();

static void factor();

static void primary();

typedef struct {
    Token token;
    Token previous;
    Chunk *chunk;
} Compiler;

static Compiler compiler;
static Scanner scanner;

static void error() {
    printf("Error");
    exit(-1);
}

void compile(const char *source, Chunk *chunk) {
    init_scanner(&scanner, source);
    compiler.token = scan_token(&scanner);

    compiler.chunk = chunk;

    term();
    write_chunk(chunk, OP_PRINT);
    write_chunk(chunk, OP_RETURN);
}

static void advance() {
    compiler.previous = compiler.token;
    compiler.token = scan_token(&scanner);
}

static bool match(TokenType type) {
    if (compiler.token.type == type) {
        advance();
        return true;
    }
    return false;
}

static void term() {
    factor();

    while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) {
        if (compiler.previous.type == TOKEN_PLUS) {
            factor();
            write_chunk(compiler.chunk, OP_ADD);
        } else {
            factor();
            write_chunk(compiler.chunk, OP_SUB);
        }
    }
}

static void factor() {
    primary();

    while (match(TOKEN_STAR) || match(TOKEN_SLASH)) {
        if (compiler.previous.type == TOKEN_STAR) {
            primary();
            write_chunk(compiler.chunk, OP_MUL);
        } else {
            primary();
            write_chunk(compiler.chunk, OP_DIV);
        }
    }
}

static void primary() {
    switch (compiler.token.type) {
        case TOKEN_NUMBER: {
            size_t length = compiler.token.length;
            char str[length + 1];
            memcpy(str, compiler.token.start, length);
            str[length] = '\0';
            char *ptr;
            double res;

            res = strtod(str, &ptr);
            uint16_t pos = (uint16_t)add_constant(compiler.chunk, res);

            if (pos > 255) {
                uint8_t index_1 = (uint8_t) (pos >> 8);
                uint8_t index_2 = (uint8_t) (pos & 0xFF);
                write_chunk(compiler.chunk, OP_LDC_W);
                write_chunk(compiler.chunk, index_1);
                write_chunk(compiler.chunk, index_2);
            } else {
                write_chunk(compiler.chunk, OP_LDC);
                write_chunk(compiler.chunk, (uint8_t) pos);
            }

            break;
        }
        case TOKEN_MINUS: {
            primary();
            write_chunk(compiler.chunk, OP_NEGATE);
            break;
        }
        case TOKEN_OPEN_PAREN: {
            advance();
            term();
            if(!compiler.token.type == TOKEN_CLOSE_PAREN) {
                error(); // TODO fix
            }
            break;
        }
        default: error();
    }

    advance();
}
