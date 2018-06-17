// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

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
    for (uint32_t i = 0; i < line_array->count; ++i) {
        free(line_array->lines[i]);
    }

    FREE_ARR(line_array->lines);
    init_line_array(line_array);
}

void write_line(LineArray *line_array, char *line) {
    if (line_array->count >= line_array->cap) {
        line_array->cap = GROW_CAP(line_array->cap);
        line_array->lines = GROW_ARR(line_array->lines, char *, line_array->cap);
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
    fprintf(stderr, "No shell on windows sorry :(.\n");
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
