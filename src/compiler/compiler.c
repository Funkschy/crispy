#include <string.h>
#include <stdio.h>

#include "compiler.h"
#include "../vm/vm.h"
#include "../vm/opcode.h"
#include "scanner.h"
#include "../vm/debug.h"
#include "../vm/value.h"
#include "native.h"

static void expr(Vm *vm);

static void stmt(Vm *vm);

static void error(Compiler *compiler, const char *err_message) {
    printf("[Line %d] %s\n", compiler->previous.line, err_message);
    exit(44);
}

static inline void advance(Vm *vm) {
    vm->compiler.previous = vm->compiler.token;
    vm->compiler.token = scan_token(&vm->compiler.scanner);
}

static inline void open_scope(Vm *vm) {
    ++vm->compiler.scope_depth;
    init_variable_array(&vm->compiler.scope[vm->compiler.scope_depth]);
}

static void close_scope(Vm *vm) {
    Compiler *compiler = &vm->compiler;

    compiler->vars_in_scope -= compiler->scope[compiler->scope_depth].count;
    free_variable_array(&compiler->scope[compiler->scope_depth]);
    --compiler->scope_depth;
}

static inline bool check(Vm *vm, TokenType type) {
    return vm->compiler.token.type == type;
}

static bool match(Vm *vm, TokenType type) {
    if (check(vm, type)) {
        advance(vm);
        return true;
    }
    return false;
}

static Token consume(Vm *vm, TokenType type, const char *err_message) {
    if (vm->compiler.token.type == type) {
        Token curr = vm->compiler.token;
        advance(vm);
        return curr;
    }

    error(&vm->compiler, err_message);

    // not reachable
    Token token;
    token.type = TOKEN_ERROR;
    return token;
}

static inline void emit_no_arg(Vm *vm, OP_CODE op_code) {
    write_code_buffer(&CURR_FRAME(vm)->code_buffer, op_code);
}

static inline void emit_byte_arg(Vm *vm, OP_CODE op_code, uint8_t arg) {
    write_code_buffer(&CURR_FRAME(vm)->code_buffer, op_code);
    write_code_buffer(&CURR_FRAME(vm)->code_buffer, arg);
}

static inline void emit_short_arg(Vm *vm, OP_CODE op_code, uint8_t first, uint8_t second) {
    write_code_buffer(&CURR_FRAME(vm)->code_buffer, op_code);
    write_code_buffer(&CURR_FRAME(vm)->code_buffer, first);
    write_code_buffer(&CURR_FRAME(vm)->code_buffer, second);
}

static inline uint32_t emit_jump(Vm *vm, OP_CODE op_code) {
    emit_short_arg(vm, op_code, 0xFF, 0xFF);
    return CURR_FRAME(vm)->code_buffer.count - 2;
}

static void patch_jump_to(Vm *vm, uint32_t offset, uint32_t address) {
    if (address > UINT16_MAX) {
        error(&vm->compiler, "Jump too big");
    }

    CURR_FRAME(vm)->code_buffer.code[offset] = (uint8_t) ((address >> 8) & 0xFF);
    CURR_FRAME(vm)->code_buffer.code[offset + 1] = (uint8_t) (address & 0xFF);
}

static inline void patch_jump(Vm *vm, uint32_t offset) {
    patch_jump_to(vm, offset, CURR_FRAME(vm)->code_buffer.count);
}

static void declare_var(Vm *vm, Token var_decl) {
    Variable variable = {var_decl.start, var_decl.length, vm->compiler.vars_in_scope++, (int) vm->frame_count};
    write_variable(&vm->compiler.scope[vm->compiler.scope_depth], variable);
    emit_byte_arg(vm, OP_STORE, (uint8_t) variable.index);
}

void compile(Vm *vm) {
    declare_natives(vm);

    do {
        stmt(vm);
    } while (vm->compiler.token.type != TOKEN_EOF);

    emit_no_arg(vm, OP_RETURN);
}

static Variable *resolve_name(Vm *vm, const char *name, size_t length) {
    Compiler *compiler = &vm->compiler;

    for (int i = compiler->scope_depth; i >= 0; --i) {
        for (int j = compiler->scope[i].count - 1; j >= 0; --j) {
            Variable *var = &vm->compiler.scope[i].variables[j];
            if (var->length == length && memcmp(name, var->name, length) == 0) {
                return var;
            }
        }
    }

    return NULL;
}

static Variable resolve_var(Vm *vm, const char *name, size_t length) {
    Variable *variable = resolve_name(vm, name, length);

    if (variable != NULL) { return *variable; }

    char message[34 + length];
    sprintf(message, "Could not find variable with name %.*s", (int) length, name);
    error(&vm->compiler, message);

    // not reachable, but clang warnings are annoying
    Variable error;
    error.index = -1;
    error.frame_offset = -1;
    return error;
}

static void primary(Vm *vm) {
    Compiler *compiler = &vm->compiler;

    switch (compiler->token.type) {
        case TOKEN_NUMBER: {
            Token token = compiler->token;
            if (token.length == 1) {
                if (*token.start == '0') {
                    emit_no_arg(vm, OP_LDC_0);
                    break;
                } else if (*compiler->token.start == '1') {
                    emit_no_arg(vm, OP_LDC_1);
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
            uint16_t pos = (uint16_t) add_constant(vm, create_number(res));

            if (pos > 255) {
                uint8_t index_1 = (uint8_t) (pos >> 8);
                uint8_t index_2 = (uint8_t) (pos & 0xFF);
                emit_short_arg(vm, OP_LDC_W, index_1, index_2);
            } else {
                emit_byte_arg(vm, OP_LDC, (uint8_t) pos);
            }

            break;
        }
        case TOKEN_OPEN_PAREN: {
            advance(vm);
            expr(vm);
            if (vm->compiler.token.type != TOKEN_CLOSE_PAREN) {
                error(&vm->compiler, "Expected ')'");
            }
            break;
        }
        case TOKEN_IDENTIFIER: {
            HTItemKey key;
            char *c_str = malloc(compiler->token.length * sizeof(char) + 1);
            memcpy(c_str, compiler->token.start, compiler->token.length);
            c_str[compiler->token.length] = '\0';
            key.key_c_string = c_str;

            Value value = ht_get(&vm->compiler.natives, key);
            free(c_str);

            if (value.type != NIL) {
                // TODO bigger numbers
                uint32_t index = add_constant(vm, value);
                emit_byte_arg(vm, OP_LDC, (uint8_t)index);
                break;
            }

            Variable var = resolve_var(vm, compiler->token.start, compiler->token.length);
            if (var.frame_offset != vm->frame_count) {
                // TODO bigger numbers
                uint8_t offset = (uint8_t) (vm->frame_count - var.frame_offset);
                emit_short_arg(vm, OP_LOAD_SCOPE, offset, (uint8_t) var.index);
                break;
            }
            emit_byte_arg(vm, OP_LOAD, (uint8_t) var.index);
            break;
        }
        case TOKEN_STRING: {
            ObjString *string = new_string(vm, compiler->token.start + 1, compiler->token.length - 2);
            uint16_t pos = (uint16_t) add_constant(vm, create_object((Object *) string));

            if (pos > UINT8_MAX) {
                uint8_t index_1 = (uint8_t) (pos >> 8);
                uint8_t index_2 = (uint8_t) (pos & 0xFF);
                emit_short_arg(vm, OP_LDC_W, index_1, index_2);
            } else {
                emit_byte_arg(vm, OP_LDC, (uint8_t) pos);
            }

            break;
        }
        case TOKEN_FALSE:
            emit_no_arg(vm, OP_FALSE);
            break;
        case TOKEN_TRUE:
            emit_no_arg(vm, OP_TRUE);
            break;
        default:
            break;
    }

    advance(vm);
}

static void primary_expr(Vm *vm) {
    primary(vm);

    if (!check(vm, TOKEN_OPEN_PAREN)) { return; }

    size_t num_args = 0;

    // Call
    while (match(vm, TOKEN_OPEN_PAREN)) {
        if (!match(vm, TOKEN_CLOSE_PAREN)) {
            do {
                expr(vm);
                ++num_args;
            } while (match(vm, TOKEN_COMMA));

            consume(vm, TOKEN_CLOSE_PAREN, "Expected ')' after argument list");
            emit_byte_arg(vm, OP_CALL, (uint8_t) num_args);
        } else {
            emit_byte_arg(vm, OP_CALL, 0);
        }
    }
}

static void factor(Vm *vm) {
    switch (vm->compiler.token.type) {
        case TOKEN_BANG: // TODO implement 'not'
        case TOKEN_MINUS:
            advance(vm);
            primary_expr(vm);
            emit_no_arg(vm, OP_NEGATE);
            break;
        default:
            primary_expr(vm);
            break;
    }
}

static void term(Vm *vm) {
    factor(vm);

    while (match(vm, TOKEN_STAR) || match(vm, TOKEN_SLASH) || match(vm, TOKEN_PERCENT)) {
        if (vm->compiler.previous.type == TOKEN_STAR) {
            factor(vm);
            emit_no_arg(vm, OP_MUL);
        } else if (vm->compiler.previous.type == TOKEN_PERCENT) {
            factor(vm);
            emit_no_arg(vm, OP_MOD);
        } else {
            factor(vm);
            emit_no_arg(vm, OP_DIV);
        }
    }

}

static void arith_expr(Vm *vm) {
    term(vm);

    while (match(vm, TOKEN_PLUS) || match(vm, TOKEN_MINUS)) {
        if (vm->compiler.previous.type == TOKEN_PLUS) {
            term(vm);
            emit_no_arg(vm, OP_ADD);
        } else {
            term(vm);
            emit_no_arg(vm, OP_SUB);
        }
    }
}

static void equality(Vm *vm) {
    arith_expr(vm);

    while (match(vm, TOKEN_EQUALS_EQUALS) || match(vm, TOKEN_BANG_EQUALS)) {
        if (vm->compiler.previous.type == TOKEN_EQUALS_EQUALS) {
            arith_expr(vm);
            emit_no_arg(vm, OP_EQUAL);
        } else {
            arith_expr(vm);
            emit_no_arg(vm, OP_NOT_EQUAL);
        }
    }
}

static void lambda(Vm *vm) {
    advance(vm);

    open_scope(vm);

    CallFrame lambda_frame;
    init_call_frame(&lambda_frame);
    PUSH_FRAME(vm, lambda_frame);

    size_t num_params = 0;

    if (!check(vm, TOKEN_ARROW)) {
        do {
            Token param = consume(vm, TOKEN_IDENTIFIER, "Expected parameter name");
            declare_var(vm, param);
            ++num_params;
        } while (match(vm, TOKEN_COMMA));
    }

    consume(vm, TOKEN_ARROW, "Expected '->' after parameter list");

    // lambda object itself
    emit_no_arg(vm, OP_POP);

    expr(vm);
    emit_no_arg(vm, OP_RETURN);

#if DEBUG_SHOW_DISASSEMBLY
    disassemble_vm(vm, "lamda");
#endif

    lambda_frame = *POP_FRAME(vm);

    ObjLambda *lambda = new_lambda(vm, num_params);
    lambda->call_frame = lambda_frame;
    lambda->call_frame.ip = lambda->call_frame.code_buffer.code;

    uint16_t pos = (uint16_t) add_constant(vm, create_object((Object *) lambda));

    if (pos > UINT8_MAX) {
        uint8_t index_1 = (uint8_t) (pos >> 8);
        uint8_t index_2 = (uint8_t) (pos & 0xFF);
        emit_short_arg(vm, OP_LDC_W, index_1, index_2);
    } else {
        emit_byte_arg(vm, OP_LDC, (uint8_t) pos);
    }

    close_scope(vm);

}

static void block_expr(Vm *vm) {
    advance(vm);
    open_scope(vm);
    while (!check(vm, TOKEN_CLOSE_BRACE) && !check(vm, TOKEN_EOF)) {
        stmt(vm);
    }
    close_scope(vm);
    advance(vm);
}

static void if_expr(Vm *vm) {
    advance(vm);
    expr(vm);

    uint32_t false_jump = emit_jump(vm, OP_JMF);
    block_expr(vm);

    uint32_t exit_jump = emit_jump(vm, OP_JMP);
    patch_jump(vm, false_jump);

    if (vm->compiler.token.type == TOKEN_ELSE) {
        advance(vm);
        if (vm->compiler.token.type == TOKEN_IF) {
            if_expr(vm);
        } else {
            block_expr(vm);
        }
    }
    patch_jump(vm, exit_jump);
}

static void expr(Vm *vm) {
    switch (vm->compiler.token.type) {
        case TOKEN_FUN:
            lambda(vm);
            break;
        case TOKEN_OPEN_BRACE:
            block_expr(vm);
            break;
        case TOKEN_IF:
            if_expr(vm);
            break;
        default:
            equality(vm);
            break;
    }
}

static void var_decl(Vm *vm) {
    advance(vm);
    Token identifier = consume(vm, TOKEN_IDENTIFIER, "Expected variable name after 'var'");
    consume(vm, TOKEN_EQUALS, "Expected '=' after variable name");

    expr(vm);
    consume(vm, TOKEN_SEMICOLON, "Expected ';' after statement");

    declare_var(vm, identifier);
}

static void assignment(Vm *vm) {
    expr(vm);
    Token identifier = vm->compiler.previous;

    if (check(vm, TOKEN_EQUALS)) {
        emit_no_arg(vm, OP_POP);
        consume(vm, TOKEN_EQUALS, "Expected '=' after variable name");
        expr(vm);
        Variable var = resolve_var(vm, identifier.start, identifier.length);
        emit_byte_arg(vm, OP_STORE, (uint8_t) var.index);
    }

    consume(vm, TOKEN_SEMICOLON, "Expected ';' after statement");
}

static void expr_stmt(Vm *vm) {
    expr(vm);
    consume(vm, TOKEN_SEMICOLON, "Expected ';' after statement");
}

static void block_stmt(Vm *vm) {
    advance(vm);
    open_scope(vm);
    while (!check(vm, TOKEN_CLOSE_BRACE) && !check(vm, TOKEN_EOF)) {
        stmt(vm);
    }
    close_scope(vm);
    advance(vm);
}

static void while_stmt(Vm *vm) {
    advance(vm);
    uint32_t start_instruction = CURR_FRAME(vm)->code_buffer.count;
    expr(vm);

    uint32_t exit_jmp = emit_jump(vm, OP_JMF);
    block_stmt(vm);

    uint32_t to_start = emit_jump(vm, OP_JMP);
    patch_jump_to(vm, to_start, start_instruction);
    patch_jump_to(vm, exit_jmp, CURR_FRAME(vm)->code_buffer.count);
}

static void if_stmt(Vm *vm) {
    advance(vm);
    expr(vm);

    uint32_t false_jump = emit_jump(vm, OP_JMF);
    block_stmt(vm);

    uint32_t exit_jump = emit_jump(vm, OP_JMP);
    patch_jump(vm, false_jump);

    if (vm->compiler.token.type == TOKEN_ELSE) {
        advance(vm);
        if (vm->compiler.token.type == TOKEN_IF) {
            if_stmt(vm);
        } else {
            block_stmt(vm);
        }
    }
    patch_jump(vm, exit_jump);
}

static void simple_stmt(Vm *vm) {
    switch (vm->compiler.token.type) {
        case TOKEN_VAR:
            var_decl(vm);
            break;
        case TOKEN_IDENTIFIER:
            assignment(vm);
            break;
        default:
            expr_stmt(vm);
            break;
    }
}

static void stmt(Vm *vm) {
    switch (vm->compiler.token.type) {
        case TOKEN_WHILE:
            while_stmt(vm);
            break;
        case TOKEN_OPEN_BRACE:
            block_stmt(vm);
            break;
        case TOKEN_IF:
            if_stmt(vm);
            break;
        case TOKEN_RETURN:
            advance(vm);

            if (!check(vm, TOKEN_SEMICOLON)) {
                expr(vm);
                consume(vm, TOKEN_SEMICOLON, "Expected ';' after return value");
            } else {
                emit_no_arg(vm, OP_NIL);
            }

            emit_no_arg(vm, OP_RETURN);
            break;
        case TOKEN_EOF:
            break;
        default:
            simple_stmt(vm);
            break;
    }
}
