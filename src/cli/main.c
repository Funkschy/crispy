#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../vm/vm.h"
#include "../vm/hashtable.h"

static void run_repl();

static void run_file(const char *file_name);

int main(int argc, char **argv) {
    Vm vm;
    init_vm(&vm);

    HashTable ht;
    ht_init(&ht, HT_KEY_CSTRING, 16, 0.75);

    ObjString *hello = new_string(&vm, "Hello World", 11);
    Value s = create_object((Object *) hello);

    HTItemKey key;
    key.key_c_string = "hello";

    ht_put(&ht, key, s);

    hello = new_string(&vm, "Servus", 6);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    print_value(ht_get(&ht, key), true);

    key.key_c_string = "dU0gnjLbCe1oXf";
    hello = new_string(&vm, "first", 5);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "eu8TmqwgNLIICs1hEBV";
    hello = new_string(&vm, "second", 6);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "F6NnYeb4p";
    hello = new_string(&vm, "third", 5);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    HTItemKey access;
    access.key_c_string = "dU0gnjLbCe1oXf";
    print_value(ht_get(&ht, access), true);

    access.key_c_string = "eu8TmqwgNLIICs1hEBV";
    print_value(ht_get(&ht, access), true);

    access.key_c_string = "F6NnYeb4p";
    print_value(ht_get(&ht, access), true);

    access.key_c_string = "test";
    print_value(ht_get(&ht, access), true);

    key.key_c_string = "uCS";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "OrMtN3Z";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "a";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "RrMA";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "NOGE";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "SCh";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "gWPf";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "YaSkNWPfY";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "FlZr00sD";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "WN";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "BOMt";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "Of";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "XDKGpa";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "a8";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "f4qc";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    key.key_c_string = "VH2Y";
    hello = new_string(&vm, "test", 4);
    s = create_object((Object *) hello);

    ht_put(&ht, key, s);

    access.key_c_string = "hello";
    print_value(ht_get(&ht, access), true);

    ht_free(&ht);
    free_vm(&vm);
    return 0;

    if (argc == 1) {
        run_repl();
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        fprintf(stderr, "Usage: calc");
    }

    return 0;
}

static void run_repl() {
    Vm vm;
    init_vm(&vm);

    char line[1024];
    while (true) {
        printf(">>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(&vm, line);
    }

    free_vm(&vm);
}

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Couldn't open file '%s'\n", path);
        exit(-1);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t) ftell(file);
    rewind(file);

    char *buffer = malloc(file_size + 1);
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

static void run_file(const char *file_name) {
    Vm vm;
    init_vm(&vm);

    char *source = read_file(file_name);
    InterpretResult result = interpret(&vm, source);

    free_vm(&vm);
    free(source);

    if (result == INTERPRET_RUNTIME_ERROR) { exit(42); }
    if (result == INTERPRET_COMPILE_ERROR) { exit(43); }
}