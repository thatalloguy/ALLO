#include "Token.h"

Token::Token(TokenType type, const std::string &lexeme, std::any literal, int line) {
    this->type = type;
    this->lexeme = lexeme;
    this->literal = literal;
    this->line = line;
}

std::string Token::toString() {
    return TokenTypetoString(type) + " " + lexeme;
}

std::string TokenTypetoString(TokenType type) {
    switch (type) {
        case LEFT_PAREN:    return "LEFT_PAREN";
        case RIGHT_PAREN:   return "RIGHT_PAREN";
        case LEFT_BRACE:    return "LEFT_BRACE";
        case RIGHT_BRACE:   return "RIGHT_BRACE";
        case COMMA:         return "COMMA";
        case DOT:           return "DOT";
        case MINUS:         return "MINUS";
        case PLUS:          return "PLUS";
        case SEMICOLON:     return "SEMICOLON";
        case SLASH:         return "SLASH";
        case STAR:          return "STAR";
        case BANG:          return "BANG";
        case BANG_EQUAL:    return "BANG_EQUAL";
        case EQUAL:         return "EQUAL";
        case EQUAL_EQUAL:   return "EQUAL_EQUAL";
        case GREATER:       return "GREATER";
        case GREATER_EQUAL: return "GREATER_EQUAL";
        case LESS:          return "LESS";
        case LESS_EQUAL:    return "LESS_EQUAL";
        case IDENTIFIER:    return "IDENTIFIER";
        case STRING:        return "STRING";
        case NUMBER:        return "NUMBER";
        case AND:           return "AND";
        case CLASS:         return "CLASS";
        case ELSE:          return "ELSE";
        case FALSE:         return "FALSE";
        case FUN:           return "FUN";
        case FOR:           return "FOR";
        case IF:            return "IF";
        case NIL:           return "NIL";
        case OR:            return "OR";
        case PRINT:         return "PRINT";
        case RETURN:        return "RETURN";
        case SUPER:         return "SUPER";
        case THIS:          return "THIS";
        case TRUE:          return "TRUE";
        case VAR:           return "VAR";
        case WHILE:         return "WHILE";
        case eof:           return "EOF";
        default:            return "UNKNOWN";
    }
}