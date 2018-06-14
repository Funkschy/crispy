#include <stdio.h>

#include "cli.h"

#include "../vm/vm.h"

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#define POSIX
#endif

static char *read_line() {
#ifdef POSIX
    char *line = NULL;
    size_t buffer = 0;
    ssize_t result = getline(&line, &buffer, stdin);

    if (result == -1) {
        fprintf(stderr, "Error while reading from stdin.\n");
        exit(1);
    }

    return line;
#else
    fprintf(stderr, "Error while reading from stdin.\n");
    return NULL;
#endif
}

static void repl(Vm *vm) {
    printf(">>> ");
    char *line = read_line();

    interpret(vm, line);

    frames_free(&vm->frames);

    FrameArray frames;
    frames_init(&frames);
    vm->frames = frames;

    CallFrame *call_frame = new_call_frame();
    vm->frame_count = 0;
    frames_write_at(&vm->frames, vm->frame_count++, call_frame);
    free(line);
}

void run_repl() {
    Vm vm;
    init_vm(&vm);

    while (true) {
        repl(&vm);
    }

    free_vm(&vm);
}
