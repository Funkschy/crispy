// Copyright (c) 2018 Felix Schoeller
// 
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef CRISPY_IOUTIL_H
#define CRISPY_IOUTIL_H

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define POSIX
#endif

ssize_t read_line(char **line);

#endif //CRISPY_IOUTIL_H
