#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../vm/vm.h"

static void run_repl(Vm *vm);

static void run_file(Vm *vm, const char *file_name);

int main(int argc, char **argv) {
    Vm vm;
    init_vm(&vm);
    if (argc == 1) {
        run_repl(&vm);
    } else if (argc == 2) {
        run_file(&vm ,argv[1]);
    } else {
        fprintf(stderr, "Usage: calc");
    }

    free_vm(&vm);
    return 0;
}

static void run_repl(Vm *vm) {
    char line[1024];
    while (true) {
        printf(">>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(vm, line);
    }
}

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if(file == NULL) {
        fprintf(stderr, "Couldn't open file '%s'\n", path);
        exit(-1);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t) ftell(file);
    rewind(file);

    char *buffer = malloc(file_size + 1);
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

static void run_file(Vm *vm, const char *file_name) {
    char *source = read_file(file_name);
    InterpretResult result = interpret(vm, source);
    free(source);

    if(result == INTERPRET_RUNTIME_ERROR) exit(42);
    if(result == INTERPRET_COMPILE_ERROR) exit(43);
}