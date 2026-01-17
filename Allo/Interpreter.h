#pragma once

#include <string>


class Interpreter {
public:
    Interpreter() = default;

    //runs each line of the file
    void runFile(const char* filePath);

    //idk
    void runPrompt();

    //runs a single statement
    void run(const std::string& str);

    static void error(int line, const std::string& message);

    static void report(int line, const std::string& where, const std::string& message);

    static bool successful() { return !hadErrors; }

private:
    static bool hadErrors;
};