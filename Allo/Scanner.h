#pragma once


#include <string>
#include <unordered_map>
#include <vector>

#include "Token.h"

class Scanner {
public:
    Scanner(const std::string& str);

    std::vector<Token> scanTokens();
    static std::unordered_map<std::string, TokenType> keywords;
private:
    std::vector<Token> tokens{};
    std::string source{};

    int start{0};
    int current{0};
    int line{1};

    void scanToken();
    void addToken(TokenType type);
    void addToken(TokenType type, std::any literal);

    char advance();
    bool match(char excepted);
    char peek() const;
    char prev() const;
    char peekNext() const;

    void string();
    void number();
    void identifier();

    static bool isDigit(char c);
    static bool isAlpha(char c);
    static bool isAlpheNumeric(char c);


    [[nodiscard]] bool isAtEnd() const { return current >= source.length(); }
};


