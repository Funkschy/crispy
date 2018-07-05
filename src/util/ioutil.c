// Copyright (c) 2018 Felix Schoeller
// 
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>

#include "ioutil.h"
#include "common.h"

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

    while (true) {
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

        if (in == EOF) {
            return -1;
        }

        if (in == '\n') {
            string[length++] = '\0';
            break;
        }

        string[length++] = (char) in;
    }

    *line_ptr = string;
    return (ssize_t) length;
}
#endif

ssize_t read_line(char **line) {
    char *temp_line = NULL;

#ifdef POSIX
    size_t buffer = 0;
    ssize_t result = getline(&temp_line, &buffer, stdin);
#else
    ssize_t result = windows_get_line(&temp_line);
#endif

    *line = temp_line;

    return result;
}