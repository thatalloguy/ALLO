

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Allo/chunk.h"
#include "Allo/virtual_machine.h"

#define VALIDATE_FILE_OP(condition, message, path) if (!(condition)) { fprintf(stderr, message, path); exit(SOURCE_FILE_READING_ERROR); }

char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");

    VALIDATE_FILE_OP(file != NULL, "Could not open file \"%s\" \n", path);

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);

    VALIDATE_FILE_OP(buffer != NULL, "Not enough memory to read \"%s\" \n", path);

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    VALIDATE_FILE_OP(bytesRead < fileSize, "Could not read file \"%s\" \n", path);
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

void repl() {
    char line[1024];
    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret_code(line);
    }
}

void run_file(const char* path) {
    char* source = read_file(path);
    InterpretResult result = interpret_code(source);
    free(source);

    if (result == INTERPRET_RUNTIME_ERROR) exit(RUNTIME_ERROR);
    if (result == INTERPRET_COMPILE_ERROR) exit(COMPILER_ERROR);
}

int main(int argc, const char* argv[]) {
    init_vm();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        fprintf(stderr, "Usage: allo [path]\n");
        exit(INVALID_CMD_ARGUMENTS);
    }


    free_vm();

    return 0;
}
