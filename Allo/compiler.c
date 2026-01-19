#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError{false};
    bool panicMode{false};
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
  } Precedence;


Parser parser;
Chunk* compilingChunk;

static Chunk* current_chunk() {
    return compilingChunk;
}

static void error_at(Token* token, const char* message) {
    if (parser.panicMode) return;
    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        //nothin
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s", message);
    parser.hadError = true;
}

static void error(const char* message) {
    error_at(&parser.previous, message);
}

static void error_at_current(const char* msg) {
    error_at(&parser.current, msg);
}


static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance_compiler();
        return;
    }

    error_at_current(message);
}

static void emit_byte(uint8_t byte) {
    write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
}

static uint8_t make_constant(Value value) {
    int constant = add_constant(current_chunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}


static void emit_constant(Value value) {
    emit_bytes(OP_CONSTANT, make_constant(value));
}

static void parse_precedence(Precedence precedence) {

}

static void expression() {
    parse_precedence(PREC_ASSIGNMENT);
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emit_constant(value);
}

static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

static void unary() {
    TokenType opType = parser.previous.type;

    parse_precedence(PREC_UNARY);
    switch (opType) {
        case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
        default: return;
    }
}

static void end_compiler() {
    emit_byte(OP_RETURN);
}

static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = get_rule(operatorType);
    parse_precedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_PLUS:          emit_byte(OP_ADD); break;
        case TOKEN_MINUS:         emit_byte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emit_byte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emit_byte(OP_DIVIDE); break;
        default: return; // Unreachable.
    }
}

bool compile(const char *source, Chunk *chunk) {
    init_scanner(source);

    parser.hadError = false;
    parser.panicMode = false;

    compilingChunk = chunk;

    advance_compiler();
    expression();
    consume(TOKEN_EOF, "Excepted end of expression");

    end_compiler();
    return !parser.hadError;
    int line = -1;
    for (;;) {
        Token token = scan_token();
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("    | ");
        }

        printf("%2d '%.*s'\n ", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF) break;
    }

    return true;
}

void advance_compiler() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scan_token();
        if (parser.current.type != TOKEN_ERROR) break;

        error_at_current(parser.current.start);
    }
}
