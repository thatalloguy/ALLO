#include "Interpreter.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "Scanner.h"

bool Interpreter::hadErrors = false;

void Interpreter::runFile(const char *filePath) {
    std::fstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Could not find file: " << filePath << "\n";
        return;
    }
;
    std::string src{};

    std::stringstream buffer;
    buffer << file.rdbuf();
    src = buffer.str();

    std::cout << "src ----------------\n";
    std::cout << src << "\n";
    std::cout << "src ----------------\n";

    run(src);

    if (hadErrors) {
        file.close();
        return;
    }

    file.close();
}

void Interpreter::runPrompt() {

    std::string in{};

    std::cout << "> ";
    std::cin >> in;

    run(in);
    if (in != "quit") {
        runPrompt();
    }
}

void Interpreter::run(const std::string& str) {
    Scanner scanner = Scanner(str);
    std::vector<Token> tokens = scanner.scanTokens();

    for (Token token : tokens) {
        std::cout << token.toString() << "\n";
    }

}

void Interpreter::error(int line, const std::string &message) {
    report(line, "", message);
}

void Interpreter::report(int line, const std::string &where, const std::string &message) {
    std::cerr << "[line " << line << "] Error " << where << ": " << message << "\n";
    hadErrors = true;
}
