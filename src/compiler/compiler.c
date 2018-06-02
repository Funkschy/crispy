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
    Vm *vm;
    Scanner scanner;

    VariableArray variables;
} Compiler;

static void expr(Compiler *compiler);

static void stmt(Compiler *compiler);

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

static void emit_no_arg(Compiler *compiler, OP_CODE op_code) {
    write_code(compiler->vm, op_code);
}

static void emit_byte_arg(Compiler *compiler, OP_CODE op_code, uint8_t arg) {
    write_code(compiler->vm, op_code);
    write_code(compiler->vm, arg);
}

static void emit_short_arg(Compiler *compiler, OP_CODE op_code, uint8_t first, uint8_t second) {
    write_code(compiler->vm, op_code);
    write_code(compiler->vm, first);
    write_code(compiler->vm, second);
}

static uint32_t emit_jump(Compiler *compiler, OP_CODE op_code) {
    emit_short_arg(compiler, op_code, 0xFF, 0xFF);
    return compiler->vm->instruction_count - 2;
}

static void patch_jump_to(Compiler *compiler, uint32_t offset, uint32_t address) {
    if (address > UINT16_MAX) {
        error("Jump too big");
    }

    compiler->vm->code[offset] = (uint8_t) ((address >> 8) & 0xFF);
    compiler->vm->code[offset + 1] = (uint8_t) (address & 0xFF);
}

static void patch_jump(Compiler *compiler, uint32_t offset) {
    patch_jump_to(compiler, offset, compiler->vm->instruction_count);
}

void compile(const char *source, Vm *vm) {
    Scanner scanner;
    init_scanner(&scanner, source);

    Compiler compiler;
    compiler.vm = vm;
    compiler.scanner = scanner;
    compiler.token = scan_token(&compiler.scanner);

    VariableArray variables;
    init_variable_array(&variables);
    compiler.variables = variables;

    do {
        stmt(&compiler);
    } while (compiler.token.type != TOKEN_EOF);

    emit_no_arg(&compiler, OP_RETURN);

    free_variable_array(&compiler.variables);
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
            Token token = compiler->token;
            if (token.length == 1) {
                if (*token.start == '0') {
                    emit_no_arg(compiler, OP_LDC_0);
                    break;
                } else if (*compiler->token.start == '1') {
                    emit_no_arg(compiler, OP_LDC_1);
                    break;
                }
            }

            size_t length = compiler->token.length;
            char str[length + 1];
            memcpy(str, compiler->token.start, length);
            str[length] = '\0';
            char *ptr;
            double res;

            res = strtod(str, &ptr);
            uint16_t pos = (uint16_t) add_constant(compiler->vm, create_number(res));

            if (pos > 255) {
                uint8_t index_1 = (uint8_t) (pos >> 8);
                uint8_t index_2 = (uint8_t) (pos & 0xFF);
                emit_short_arg(compiler, OP_LDC_W, index_1, index_2);
            } else {
                emit_byte_arg(compiler, OP_LDC, (uint8_t) pos);
            }

            break;
        }
        case TOKEN_MINUS: {
            advance(compiler);
            primary(compiler);
            emit_no_arg(compiler, OP_NEGATE);
            return;
        }
        case TOKEN_OPEN_PAREN: {
            advance(compiler);
            expr(compiler);
            if (!compiler->token.type == TOKEN_CLOSE_PAREN) {
                error("Expected ')'");
            }
            break;
        }
        case TOKEN_IDENTIFIER: {
            Variable var = resolve_var(compiler, compiler->token.start, compiler->token.length);
            emit_byte_arg(compiler, OP_LOAD, (uint8_t) var.index);
            break;
        }
        case TOKEN_STRING: {
            ObjString *string = new_string(compiler->vm, compiler->token.start + 1, compiler->token.length - 2);
            uint16_t pos = (uint16_t)add_constant(compiler->vm, create_object((Object *)string));

            if (pos > 255) {
                uint8_t index_1 = (uint8_t) (pos >> 8);
                uint8_t index_2 = (uint8_t) (pos & 0xFF);
                emit_short_arg(compiler, OP_LDC_W, index_1, index_2);
            } else {
                emit_byte_arg(compiler, OP_LDC, (uint8_t) pos);
            }

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
            emit_no_arg(compiler, OP_MUL);
        } else {
            primary(compiler);
            emit_no_arg(compiler, OP_DIV);
        }
    }
}

static void term(Compiler *compiler) {
    factor(compiler);

    while (match(TOKEN_PLUS, compiler) || match(TOKEN_MINUS, compiler)) {
        if (compiler->previous.type == TOKEN_PLUS) {
            factor(compiler);
            emit_no_arg(compiler, OP_ADD);
        } else {
            factor(compiler);
            emit_no_arg(compiler, OP_SUB);
        }
    }
}

static void equality(Compiler *compiler) {
    term(compiler);

    while (match(TOKEN_EQUALS_EQUALS, compiler) || match(TOKEN_BANG_EQUALS, compiler)) {
        if (compiler->previous.type == TOKEN_EQUALS_EQUALS) {
            term(compiler);
            emit_no_arg(compiler, OP_EQUAL);
        } else {
            term(compiler);
            emit_no_arg(compiler, OP_NOT_EQUAL);
        }
    }
}

static void expr(Compiler *compiler) {
    equality(compiler);
}

static void var_decl(Compiler *compiler) {
    advance(compiler);
    Token identifier = consume(compiler, TOKEN_IDENTIFIER, "Expected variable name after 'var'");
    consume(compiler, TOKEN_EQUALS, "Expected '=' after variable name");

    expr(compiler);

    Variable variable = {identifier.start, identifier.length, (int) (compiler->variables.count)};
    uint32_t index = write_variable(&compiler->variables, variable);

    emit_byte_arg(compiler, OP_STORE, (uint8_t) index);
}

static void assignment(Compiler *compiler) {
    Token identifier = consume(compiler, TOKEN_IDENTIFIER, "Expected identifier");
    consume(compiler, TOKEN_EQUALS, "Expected '=' after variable name");
    expr(compiler);

    Variable var = resolve_var(compiler, identifier.start, identifier.length);
    emit_byte_arg(compiler, OP_STORE, (uint8_t) var.index);
}

static void expr_stmt(Compiler *compiler) {
    expr(compiler);
}

static void print_stmt(Compiler *compiler) {
    advance(compiler);
    expr(compiler);
    emit_no_arg(compiler, OP_PRINT);
}

static void block(Compiler *compiler) {
    advance(compiler);
    while (compiler->token.type != TOKEN_CLOSE_BRACE && compiler->token.type != TOKEN_EOF) {
        stmt(compiler);
    }
    advance(compiler);
}

static void while_stmt(Compiler *compiler) {
    advance(compiler);
    uint32_t start_instruction = compiler->vm->instruction_count;
    expr(compiler);

    uint32_t exit_jmp = emit_jump(compiler, OP_JMF);
    block(compiler);

    uint32_t to_start = emit_jump(compiler, OP_JMP);
    patch_jump_to(compiler, to_start, start_instruction);
    patch_jump_to(compiler, exit_jmp, compiler->vm->instruction_count);
}

static void if_stmt(Compiler *compiler) {
    advance(compiler);
    expr(compiler);

    uint32_t false_jump = emit_jump(compiler, OP_JMF);
    block(compiler);

    uint32_t exit_jump = emit_jump(compiler, OP_JMP);
    patch_jump(compiler, false_jump);

    if (compiler->token.type == TOKEN_ELSE) {
        advance(compiler);
        if (compiler->token.type == TOKEN_IF) {
            if_stmt(compiler);
        } else {
            block(compiler);
        }
    }
    patch_jump(compiler, exit_jump);
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

static void stmt(Compiler *compiler) {
    switch (compiler->token.type) {
        case TOKEN_WHILE:
            while_stmt(compiler);
            break;
        case TOKEN_OPEN_BRACE:
            block(compiler);
            break;
        case TOKEN_IF:
            if_stmt(compiler);
            break;
        default:
            simple_stmt(compiler);
            break;
    }
}
