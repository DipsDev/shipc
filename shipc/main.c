#include <stdio.h>
#include "debug.h"
#include "compiler.h"
#include "vm.h"
#include <stdlib.h>
#include <string.h>

#include <time.h>

static char* read_source_code(char* path) {
    FILE *fptr;
    fptr = fopen(path, "rb");

    if (fptr == NULL) {
        printf("[ERROR] couldn't read file %s.", path);
        exit(1);
    }

    fseek(fptr, 0L, SEEK_END);

    // get the distance between the last and first char of the file
    size_t fileSize = ftell(fptr);
    rewind(fptr);

    // allocate enough bytes
    char* buffer = (char*) malloc(fileSize + 1);
    size_t bytedRead = fread(buffer, sizeof(char), fileSize, fptr);
    buffer[bytedRead] = '\0';

    fclose(fptr);
    return buffer;
}

void run_code(char* path) {
    char* source_code = read_source_code(path);
    FunctionObj* compiled_func = compile(source_code);
    if (compiled_func == NULL) {
        free(source_code);
        exit(1);
    }
#ifdef SHIP_DEBUG
    disassemble_func(compiled_func);
#endif
    free(source_code);

    VM vm;
    init_vm(&vm);
    interpret(&vm, compiled_func);

    free_vm(&vm);

}

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("[ERROR] expected at least one argument.");
        exit(1);
    }

    run_code(argv[1]);
    return 0;
}
