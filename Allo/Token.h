#pragma once
#include <string>
#include <any>


enum TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER,

    // Keywords.
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    eof,

};

static std::string TokenTypetoString(TokenType type);



class Token {
public:
    Token(TokenType type, const std::string& lexeme, std::any literal, int line);

    std::string toString();


private:
    TokenType type{IDENTIFIER};
    std::string lexeme{};
    std::any literal{nullptr};
    int line{-1};
};
