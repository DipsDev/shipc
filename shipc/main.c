#include <stdio.h>
#include "debug.h"
#include "compiler.h"
#include "vm.h"
#include <stdlib.h>

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

int main() {
    clock_t t;
    t = clock();
	FunctionObj* compiled_func = compile(read_source_code());
	if (compiled_func == NULL) {
		exit(1);
	}
    t = clock() - t;
    printf("Program took %f seconds to compile\n", (double) t / CLOCKS_PER_SEC);
	disassemble_func(compiled_func);

	VM vm;
	init_vm(&vm);
    t = clock();
	interpret(&vm, compiled_func);

	free_vm(&vm);

    t = clock() - t;
    double time_taken = ((double) t / CLOCKS_PER_SEC);
    printf("Program took %f seconds to execute\n", time_taken);



	return 0;
}
