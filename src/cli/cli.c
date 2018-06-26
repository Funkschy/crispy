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

#ifndef POSIX
/**
 * Simple getline implementation, because Windows is not posix compliant.
 * @param line_ptr this pointer will be set to the allocated string.
 * @return the size of the string, which was read from stdin.
 */
static ssize_t windows_get_line(char **line_ptr) {
    size_t buf_size = 256;
    size_t length = 0;

    // needs to be an int, because getchar returns an int (and EOF is an int)
    int in;
    char *string = malloc(buf_size * sizeof(char));

    if (string == NULL) {
        return -1;
    }

    do {
        in = getchar();

        if (length >= buf_size) {
            char *new_string = malloc(2 * buf_size);

            if (new_string == NULL) {
                return -1;
            }

            memcpy(new_string, string, length);
            free(string);
            string = new_string;
        }

        string[length++] = (char) in;

    } while (in != EOF && in != '\0');

    string[length] = '\0';

    *line_ptr = string;
    return (ssize_t) length;
}
#endif

static char *read_line() {
    char *line = NULL;
#ifdef POSIX
    size_t buffer = 0;
    ssize_t result = getline(&line, &buffer, stdin);
#else
    ssize_t result = windows_get_line(&line);
#endif

    if (result == -1) {
        fprintf(stderr, "Error while reading from stdin.\n");
        exit(1);
    }

    return line;
}

static void repl(Vm *vm, LineArray *lines) {
    printf(">>> ");

    char *line = read_line();
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
