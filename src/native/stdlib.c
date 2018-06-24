// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <string.h>

#include "stdlib.h"
#include "../vm/value.h"
#include "../vm/dictionary.h"

CrispyValue println(CrispyValue *value) {
    print_value(value[0], true, false);

    return create_nil();
}

CrispyValue print(CrispyValue *value) {
    print_value(value[0], false, false);

    return create_nil();
}

CrispyValue exit_vm(CrispyValue *value, Vm *vm) {
    vm_free(vm);

    if (value[0].type == NUMBER) {
        int ret_val = (int) value[0].d_value;
        exit(ret_val);
    }

    exit(1);
}

CrispyValue str(CrispyValue *value, Vm *vm) {
    ObjString *string = NULL;

    switch (value->type) {
        case NUMBER: {
            char s[17];
            snprintf(s, 17, "%.15g", value->d_value);
            // one less, because crispy strings are not null terminated
            string = new_string(vm, s, strlen(s));
            break;
        }
        case OBJECT: {
            Object *object = value->o_value;

            switch (object->type) {
                case OBJ_STRING:
                    string = (ObjString *) object;
                    break;
                case OBJ_LAMBDA:
                    // TODO arity
                    string = new_string(vm, "<function>", 10);
                    break;
                case OBJ_NATIVE_FUNC:
                    // TODO arity
                    string = new_string(vm, "<native function>", 17);
                    break;
                case OBJ_DICT: {
                    char *dict_string = dict_to_string((ObjDict *) object);
                    string = new_string(vm, dict_string, strlen(dict_string));
                    free(dict_string);
                    break;
                }
                case OBJ_LIST:
                    string = new_string(vm, "<list>", 6);
                    break;
            }
            break;
        }
        case BOOLEAN: {
            string = value->p_value ? new_string(vm, "true", 4) : new_string(vm, "false", 5);
            break;
        }
        case NIL:
            string = new_string(vm, "nil", 3);
            break;
    }

    return create_object((Object *) string);
}