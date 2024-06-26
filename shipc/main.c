#include <stdio.h>
#include "debug.h"
#include "compiler.h"
#include "vm.h"
#include <stdlib.h>
#include <string.h>

#include <time.h>


static char* read_source_code() {
    // open the file
    FILE *fptr;
    fptr = fopen("../main.ship", "r");

    // if couldn't open file, throw an error
    if (fptr == NULL) {
        printf("[ERROR] couldn't read source code.");
        exit(1);
    }

    // set the handle to the end of the file
    fseek(fptr, 0L, SEEK_END);

    // get the distance between the last and first char of the file
    size_t fileSize = ftell(fptr);
    rewind(fptr);

    // allocate enough bytes
    char* buffer = (char*) malloc(fileSize + 1);
    size_t bytedRead = fread(buffer, sizeof(char), fileSize, fptr);
    buffer[bytedRead] = '\0';

    // close the file
    fclose(fptr);
    return buffer;
}

void run_code() {
    char* source_code = read_source_code();
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
    run_code();
	return 0;
}
