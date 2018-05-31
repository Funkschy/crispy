#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "../cli/common.h"
#include "scanner.h"
#include "../vm/opcode.h"
#include "variables.h"

typedef struct {
    Token token;
    Token previous;
    Chunk *chunk;
    Scanner scanner;

    VariableArray variables;
} Compiler;

static void term(Compiler *compiler);

static void simple_stmt(Compiler *compiler);

static void error(const char *err_message) {
    printf("%s\n", err_message);
    exit(-1);
}

static void advance(Compiler *compiler) {
    compiler->previous = compiler->token;
    compiler->token = scan_token(&compiler->scanner);
}

static bool match(TokenType type, Compiler *compiler) {
    if (compiler->token.type == type) {
        advance(compiler);
        return true;
    }
    return false;
}

static Token consume(Compiler *compiler, TokenType type, const char *err_message) {
    if (compiler->token.type == type) {
        Token curr = compiler->token;
        advance(compiler);
        return curr;
    }

    error(err_message);
}

void compile(const char *source, Chunk *chunk) {
    Scanner scanner;
    init_scanner(&scanner, source);

    Compiler compiler;
    compiler.chunk = chunk;
    compiler.scanner = scanner;
    compiler.token = scan_token(&compiler.scanner);

    VariableArray variables;
    init_variable_array(&variables);
    compiler.variables = variables;

    do {
        simple_stmt(&compiler);
    } while (compiler.token.type != TOKEN_EOF);

    write_chunk(chunk, OP_RETURN);

    free_variable_array(&compiler.variables);
    compiler.chunk = NULL;
}

static int resolve_name(Compiler *compiler, const char *name, size_t length) {
    for (int i = compiler->variables.count - 1; i >= 0; --i) {
        Variable var = compiler->variables.variables[i];
        if (var.length == length && memcmp(name, var.name, length) == 0) {
            return i;
        }
    }

    return -1;
}

static Variable resolve_var(Compiler *compiler, const char *name, size_t length) {
    Variable variable;
    variable.index = resolve_name(compiler, name, length);
    variable.name = name;
    variable.length = length;

    if (variable.index != -1) return variable;

    char message[34 + length];
    sprintf(message, "Could not find variable with name %s", name);
    error(message);
}

static void primary(Compiler *compiler) {
    switch (compiler->token.type) {
        case TOKEN_NUMBER: {
            size_t length = compiler->token.length;
            char str[length + 1];
            memcpy(str, compiler->token.start, length);
            str[length] = '\0';
            char *ptr;
            double res;

            res = strtod(str, &ptr);
            uint16_t pos = (uint16_t) add_constant(compiler->chunk, res);

            if (pos > 255) {
                uint8_t index_1 = (uint8_t) (pos >> 8);
                uint8_t index_2 = (uint8_t) (pos & 0xFF);
                write_chunk(compiler->chunk, OP_LDC_W);
                write_chunk(compiler->chunk, index_1);
                write_chunk(compiler->chunk, index_2);
            } else {
                write_chunk(compiler->chunk, OP_LDC);
                write_chunk(compiler->chunk, (uint8_t) pos);
            }

            break;
        }
        case TOKEN_MINUS: {
            primary(compiler);
            write_chunk(compiler->chunk, OP_NEGATE);
            break;
        }
        case TOKEN_OPEN_PAREN: {
            advance(compiler);
            term(compiler);
            if (!compiler->token.type == TOKEN_CLOSE_PAREN) {
                error("Expected ')'");
            }
            break;
        }
        case TOKEN_IDENTIFIER: {
            Variable var = resolve_var(compiler, compiler->token.start, compiler->token.length);
            write_chunk(compiler->chunk, OP_LOAD);
            write_chunk(compiler->chunk, (uint8_t) var.index);
            break;
        }
        default:
            error("Unexpected Token in primary");
    }

    advance(compiler);
}

static void factor(Compiler *compiler) {
    primary(compiler);

    while (match(TOKEN_STAR, compiler) || match(TOKEN_SLASH, compiler)) {
        if (compiler->previous.type == TOKEN_STAR) {
            primary(compiler);
            write_chunk(compiler->chunk, OP_MUL);
        } else {
            primary(compiler);
            write_chunk(compiler->chunk, OP_DIV);
        }
    }
}

static void term(Compiler *compiler) {
    factor(compiler);

    while (match(TOKEN_PLUS, compiler) || match(TOKEN_MINUS, compiler)) {
        if (compiler->previous.type == TOKEN_PLUS) {
            factor(compiler);
            write_chunk(compiler->chunk, OP_ADD);
        } else {
            factor(compiler);
            write_chunk(compiler->chunk, OP_SUB);
        }
    }
}

static void expr(Compiler *compiler) {
    term(compiler);
}

static void var_decl(Compiler *compiler) {
    advance(compiler);
    Token identifier = consume(compiler, TOKEN_IDENTIFIER, "Expected variable name after 'var'");
    consume(compiler, TOKEN_EQUALS, "Expected '=' after variable name");

    term(compiler);

    Variable variable = {identifier.start, identifier.length, (int) (compiler->variables.count)};
    uint32_t index = write_variable(&compiler->variables, variable);

    write_chunk(compiler->chunk, OP_STORE);
    write_chunk(compiler->chunk, (uint8_t) index);
}

static void assignment(Compiler *compiler) {
    Token identifier = consume(compiler, TOKEN_IDENTIFIER, "Expected identifier");
    consume(compiler, TOKEN_EQUALS, "Expected '=' after variable name");
    expr(compiler);

    Variable var = resolve_var(compiler, identifier.start, identifier.length);
    write_chunk(compiler->chunk, OP_STORE);
    write_chunk(compiler->chunk, (uint8_t) var.index);
}

static void expr_stmt(Compiler *compiler) {
    expr(compiler);
}

static void print_stmt(Compiler *compiler) {
    advance(compiler);
    expr(compiler);
    write_chunk(compiler->chunk, OP_PRINT);
}

static void simple_stmt(Compiler *compiler) {
    switch (compiler->token.type) {
        case TOKEN_VAR:
            var_decl(compiler);
            break;
        case TOKEN_IDENTIFIER:
            assignment(compiler);
            break;
        case TOKEN_PRINT:
            print_stmt(compiler);
            break;
        default:
            expr_stmt(compiler);
            break;
    }

    consume(compiler, TOKEN_SEMICOLON, "Expected ';' after statement");
}
