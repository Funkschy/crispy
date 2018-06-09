#include "stdlib.h"

void println(Value value) {
    print_value(value, true);
}

void print(Value value) {
    print_value(value, false);
}
