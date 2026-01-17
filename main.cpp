#include <iostream>

#include "Allo/Interpreter.h"


int main(int argc, char *argv[]) {

    if (argc > 2) {
        std::cout << "Usage: ALLO [script or prompt]\n";
        return 64;
    }

    Interpreter interpreter{};

    if (argc == 2) {
        interpreter.runFile(argv[1]);
    } else {
        interpreter.runPrompt();
    }


    return 0;
}
