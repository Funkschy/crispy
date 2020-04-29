// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/crispy.h"
#include "cli.h"

static void run_file(const char *file_name);

int main(int argc, char **argv) {
    if (argc == 1) {
        run_repl();
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        fprintf(stderr, "Usage: calc");
    }

    return 0;
}

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        exit(-1);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t) ftell(file);
    rewind(file);

    char *buffer = malloc(file_size + 1);

    if (buffer == NULL) {
        fprintf(stderr, "Could not allocate enough memory to open file '%s'\n", path);
        exit(-2);
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);

    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(-3);
    }

    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

static void run_file(const char *file_name) {
    Vm vm;
    vm_init(&vm, false);

    char *source = read_file(file_name);
    InterpretResult result = interpret(&vm, source);

    vm_free(&vm);
    free(source);

    if (result == INTERPRET_RUNTIME_ERROR) {
        printf("Error while interpreting %s\n", file_name);
        exit(42);
    }
    if (result == INTERPRET_COMPILE_ERROR) {
        printf("Error while compiling %s\n", file_name);
        exit(43);
    }
}
