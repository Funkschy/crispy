#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../vm/vm.h"

static void repl(Vm *vm);

int main(int argc, char **argv) {
    Vm vm;
    init_vm(&vm);

    if (argc == 1) {
        repl(&vm);
    } else {
        fprintf(stderr, "Usage: calc");
    }

    free_vm(&vm);
    return 0;
}

static void repl(Vm *vm) {
    char line[1024];
    while(true) {
        printf(">>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(vm, line);
    }
}