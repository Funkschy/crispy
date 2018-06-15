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

#include "cli.h"

#include "../vm/vm.h"
#include "../vm/memory.h"

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define POSIX
#endif

typedef struct {
    uint32_t count;
    uint32_t cap;
    char **lines;
} LineArray;

void init_line_array(LineArray *line_array) {
    line_array->cap = 0;
    line_array->count = 0;
    line_array->lines = NULL;
}

void free_line_array(LineArray *line_array) {
    FREE_ARR(line_array->lines);
    init_line_array(line_array);
}

void write_line(LineArray *line_array, char *line) {
    if (line_array->count >= line_array->cap) {
        line_array->cap = GROW_CAP(line_array->cap);
        line_array->lines = GROW_ARR(line_array->lines, Value, line_array->cap);
    }

    line_array->lines[line_array->count++] = line;
}

static char *read_line() {
#ifdef POSIX
    char *line = NULL;
    size_t buffer = 0;
    ssize_t result = getline(&line, &buffer, stdin);

    if (result == -1) {
        fprintf(stderr, "Error while reading from stdin.\n");
        exit(1);
    }

    return line;
#else
    fprintf(stderr, "Error while reading from stdin.\n");
    return NULL;
#endif
}

static void repl(Vm *vm, LineArray *lines) {
    printf(">>> ");

    char *line = read_line();
    interpret_interactive(vm, line);

    write_line(lines, line);
}

void run_repl() {
    Vm vm;
    init_vm(&vm, true);

    LineArray lines;
    init_line_array(&lines);

    while (true) {
        repl(&vm, &lines);
    }

    // TODO execute
    free_line_array(&lines);
    free_vm(&vm);
}
