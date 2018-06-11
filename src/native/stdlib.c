#include "stdlib.h"
#include "../vm/value.h"

void println(Value value) {
    print_value(value, true);
}

void print(Value value) {
    print_value(value, false);
}

void exit_vm(Value value) {
    if (value.type == NUMBER) {
        int ret_val = (int) value.d_value;
        exit(ret_val);
    }

    exit(1);
}
