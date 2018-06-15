#include <stdio.h>
#include "stdlib.h"
#include "../vm/value.h"

Value println(Value *value) {
    print_value(value[0], true);

    return create_nil();
}

Value print(Value *value) {
    print_value(value[0], false);

    return create_nil();
}

// TODO maybe free vm?
Value exit_vm(Value *value) {
    if (value[0].type == NUMBER) {
        int ret_val = (int) value[0].d_value;
        exit(ret_val);
    }

    exit(1);
}

Value str(Value *value, Vm *vm) {
    char s[17];
    snprintf(s, 17, "%.15g", value->d_value);

    // one less, because crispy strings are not null terminated
    return create_object((Object *) new_string(vm, s, 16));
}