#include "native.h"

#include "../native/stdlib.h"

static void make_native(Vm *vm, const char *name, void (*fn_ptr)(Value)) {
    void (*fn)(Value) = fn_ptr;
    ObjNativeFunc *println = new_native_func(vm, fn);
    Value value = create_object((Object *) println);

    HTItemKey key;
    key.key_c_string = name;
    ht_put(&vm->compiler.natives, key, value);
}

void declare_natives(Vm *vm) {
    make_native(vm, "println", &println);
    make_native(vm, "print", &print);
    make_native(vm, "exit", &exit_vm);
}
