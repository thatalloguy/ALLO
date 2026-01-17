#include "Scanner.h"

#include <utility>

#include "Interpreter.h"

std::unordered_map<std::string, TokenType> Scanner::keywords = {
    {"and", AND},
    {"class",  CLASS},
    {"else",   ELSE},
    {"false",  FALSE},
    {"for",    FOR},
    {"fun",    FUN},
    {"if",     IF},
    {"nil",    NIL},
    {"or",     OR},
    {"print",  PRINT},
    {"return", RETURN},
    {"super",  SUPER},
    {"this",   THIS},
    {"true",   TRUE},
    {"var",    VAR},
    {"while",  WHILE}
};

Scanner::Scanner(const std::string &str) {
    this->source = str;
}

std::vector<Token> Scanner::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }

    tokens.emplace_back(eof, "", nullptr, line);

    return tokens;
}

void Scanner::scanToken() {
    switch (const char c = advance()) {
        case '(': addToken(LEFT_PAREN); break;
        case ')': addToken(RIGHT_PAREN); break;
        case '{': addToken(LEFT_BRACE); break;
        case '}': addToken(RIGHT_BRACE); break;
        case ',': addToken(COMMA); break;
        case '.': addToken(DOT); break;
        case '-': addToken(MINUS); break;
        case '+': addToken(PLUS); break;
        case ';': addToken(SEMICOLON); break;
        case '*': addToken(STAR); break;
        case '!': addToken(match('=') ? BANG_EQUAL : BANG); break;
        case '=': addToken(match('=') ? EQUAL_EQUAL : EQUAL); break;
        case '<': addToken(match('=') ? LESS_EQUAL : LESS); break;
        case '>': addToken(match('=') ? GREATER_EQUAL : GREATER); break;
        case '/':
            if (match('/')) {
                while (peek() != '\n' && !isAtEnd()) advance();
            } else if (match('*')) {
                while (peek() != '*' && peekNext() != '/' && !isAtEnd()) advance();
                // advance for both the '*' & '/'
                current += 2;
            }else {
                addToken(SLASH);
            }
            break;

        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;

        case '\n':
            line++;
            break;

        case '"': string(); break;
        case 'o': if (match('r')) addToken(OR); break;


        default:
            if (isDigit(c)) number();
            else if (isAlpha(c)) identifier();
            else Interpreter::error(line, "Unexpected character. " + c ); break;
    }
}

void Scanner::addToken(TokenType type) {
    addToken(type, nullptr);
}

void Scanner::addToken(TokenType type, std::any literal) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, std::move(literal), line);
}

char Scanner::advance() {
    if (source[current] == '\n') line++;

    return source[current++];
}

bool Scanner::match(char excepted) {
    if (isAtEnd()) return false;
    if (source.at(current) != excepted) return false;
    current++;

    return true;
}

char Scanner::peek() const {
    if (isAtEnd()) return '\0';

    return source.at(current);
}

char Scanner::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source.at(current + 1);
}

void Scanner::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) {
        Interpreter::error(line, "Unterminated string.");
        return;
    }

    advance();

    std::string value = source.substr(start + 1, current - start);
    addToken(STRING, value);
}

void Scanner::number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the "."
        advance();

        while (isDigit(peek())) advance();
    }

    addToken(NUMBER,
        std::stod(source.substr(start, current - start)));
}

void Scanner::identifier() {
    while (isAlpheNumeric(peek())) advance();
    std::string text = source.substr(start, current - start);
    auto it = keywords.find(text);
    TokenType type = IDENTIFIER;

    if (it != keywords.end()) type = it->second;


    addToken(type);
}

bool Scanner::isAlpha(const char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

bool Scanner::isAlpheNumeric(const char c) {
    return isAlpha(c) || isDigit(c);
}

bool Scanner::isDigit(const char c) {
    return c >= '0' && c <=  '9';
}