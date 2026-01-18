#pragma once
#include <any>
#include <memory>
#include "../Token.h"

class Binary;
class Grouping;
class Literal;
class Unary;

template<typename R>
class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual R visitBinaryExpr(Binary& expr) = 0;
    virtual R visitGroupingExpr(Grouping& expr) = 0;
    virtual R visitLiteralExpr(Literal& expr) = 0;
    virtual R visitUnaryExpr(Unary& expr) = 0;
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual std::any accept(ExprVisitor<std::any>& visitor) = 0;
};

using ExprPtr = std::unique_ptr<Expr>;

class Binary : public Expr {
public:
    Binary(ExprPtr left, Token op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    std::any accept(ExprVisitor<std::any>& visitor) override {
        return visitor.visitBinaryExpr(*this);
    }

    ExprPtr left{};
    Token op{};
    ExprPtr right{};
};

class Grouping : public Expr {
public:
    Grouping(ExprPtr expression)
        : expression(std::move(expression)) {}

    std::any accept(ExprVisitor<std::any>& visitor) override {
        return visitor.visitGroupingExpr(*this);
    }

    ExprPtr expression{};
};

class Literal : public Expr {
public:
    Literal(std::any value)
        : value(std::move(value)) {}

    std::any accept(ExprVisitor<std::any>& visitor) override {
        return visitor.visitLiteralExpr(*this);
    }

    std::any value{};
};

class Unary : public Expr {
public:
    Unary(Token op, ExprPtr right)
        : op(std::move(op)), right(std::move(right)) {}

    std::any accept(ExprVisitor<std::any>& visitor) override {
        return visitor.visitUnaryExpr(*this);
    }

    Token op{};
    ExprPtr right{};
};

