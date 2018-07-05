// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#include "cli.h"

#include "../vm/vm.h"
#include "../vm/memory.h"
#include "../util/ioutil.h"

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

static void repl(Vm *vm, LineArray *lines) {
    printf(">>> ");

    char *line;
    ssize_t result = read_line(&line);

    if (result < 0) {
        fprintf(stderr, "Bye.\n");
        exit(0);
    }

    interpret_interactive(vm, line);

    write_line(lines, line);
}

void run_repl() {
    Vm vm;
    vm_init(&vm, true);

    LineArray lines;
    init_line_array(&lines);

    while (true) {
        repl(&vm, &lines);
    }
}
