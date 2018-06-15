// Copyright (c) 2018 Felix Schoeller
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

static void run_file(const char *file_name) {
    Vm vm;
    init_vm(&vm, false);

    char *source = read_file(file_name);
    InterpretResult result = interpret(&vm, source);

    free_vm(&vm);
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