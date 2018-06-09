#ifndef CRISPY_STDLIB_H
#define CRISPY_STDLIB_H

#include "../vm/value.h"

void println(Value value);

void print(Value value);

void native_clock(Value value);

#endif //CRISPY_STDLIB_H
