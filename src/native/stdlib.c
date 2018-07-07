// Copyright (c) 2018 Felix Schoeller
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <stdio.h>
#include <string.h>

#include "stdlib.h"
#include "../vm/value.h"
#include "../vm/dictionary.h"
#include "../vm/vm.h"
#include "../vm/list.h"
#include "../util/ioutil.h"

CrispyValue std_println(CrispyValue *value) {
    print_value(value[0], true, false);

    return create_nil();
}

CrispyValue std_print(CrispyValue *value) {
    print_value(value[0], false, false);

    return create_nil();
}

CrispyValue std_exit(CrispyValue *value, Vm *vm) {
    vm_free(vm);

    if (value[0].type == NUMBER) {
        int ret_val = (int) value[0].d_value;
        exit(ret_val);
    }

    exit(1);
}

CrispyValue std_str(CrispyValue *value, Vm *vm) {
    ObjString *string = NULL;

    switch (value->type) {
        case NUMBER: {
            char s[23];
            snprintf(s, 23, "%.15g", value->d_value);
            // crispy strings are not null terminated
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

CrispyValue std_len(CrispyValue *value, Vm *vm) {
    if (value->type != OBJECT) {
        vm->err_flag = true;
        // TODO include type
        return create_object((Object *) new_string(vm, "Value has no length", 19));
    }

    Object *obj = value->o_value;

    switch (obj->type) {
        case OBJ_LIST:
            return create_number(((ObjList *) obj)->content.count);
        case OBJ_STRING:
            return create_number(((ObjString *) obj)->length);
        default:
            vm->err_flag = true;
            // TODO include type
            return create_object((Object *) new_string(vm, "Value has no length", 19));
    }
}

CrispyValue std_split(CrispyValue *value, Vm *vm) {
    if (value->type != OBJECT || value->o_value->type != OBJ_STRING) {
        vm->err_flag = true;
        return create_object((Object *) new_string(vm, "Only strings can be splitted", 28));
    }

    if (value[1].type != OBJECT || value[1].o_value->type != OBJ_STRING) {
        vm->err_flag = true;
        return create_object((Object *) new_string(vm, "Only strings can be used as delimiter for 'split'", 49));
    }

    ObjString *string = (ObjString *) value->o_value;
    ObjString *delim = (ObjString *) (value + 1)->o_value;

    if (delim->length > string->length) {
        return create_object((Object *) new_list(vm, 0));
    }

    ObjList *tokens = new_list(vm, 0);
    size_t length = string->length - delim->length;
    uint32_t offset = 0;

    for (uint32_t i = 0; i < length; ++i) {
        if (memcmp(string->start + i, delim->start, delim->length) == 0) {
            i += delim->length;
            ObjString *token = new_string(vm, &string->start[offset], i - offset - delim->length);
            list_append(tokens, create_object((Object *) token));
            offset = i;
        }
    }

    ObjString *token = new_string(vm, &string->start[offset], string->length - offset);
    list_append(tokens, create_object((Object *) token));

    return create_object((Object *) tokens);
}

CrispyValue std_input(CrispyValue *value, Vm *vm) {
    char *line;
    ssize_t length = read_line(&line);

    if (length < 0) {
        vm->err_flag = true;
        return create_object((Object *) new_string(vm, "Error while reading line from stdin", 35));
    }

    // cut off trailing newline
    ObjString *string = new_string(vm, line, (size_t) length - 1);
    free(line);

    return create_object((Object *) string);
}

CrispyValue std_list(CrispyValue *value, Vm *vm) {
    switch (value[0].type) {
        case OBJECT:
            switch (value->o_value->type) {
                case OBJ_LIST:
                    return *value;
                case OBJ_STRING: {
                    ObjString *string = (ObjString *) value->o_value;
                    ObjList *list = new_list(vm, string->length);

                    for (uint32_t i = 0; i < string->length; ++i) {
                        list->content.values[i] =
                                create_object((Object *) new_string(vm, string->start + i, 1));
                    }

                    return create_object((Object *) list);
                }
                default:
                    vm->err_flag = true;
                    return create_object((Object *) new_string(vm, "Invalid type for list()", 23));
            }
        default:
            vm->err_flag = true;
            return create_object((Object *) new_string(vm, "Invalid type for list()", 23));
    }
}

CrispyValue std_num(CrispyValue *value, Vm *vm) {
    if (value[0].type != OBJECT || value[0].o_value->type != OBJ_STRING) {
        vm->err_flag = true;
        return create_object((Object *) new_string(vm, "num() can only be used on strings", 33));
    }

    ObjString *string = (ObjString *) value[0].o_value;
    char temp[string->length + 1];
    memcpy(temp, string->start, string->length);
    temp[string->length] = '\0';

    double res;
    res = strtod(temp, NULL);

    return create_number(res);
}
