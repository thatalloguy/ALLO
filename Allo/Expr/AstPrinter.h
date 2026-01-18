#pragma once
#include <sstream>

#include "Expr.h"

class AstPrinter: public ExprVisitor<std::any> {
public:
    std::string print(Expr& expr) {
        return std::any_cast<std::string>(expr.accept(*this));
    }

    std::any visitBinaryExpr(Binary& expr) override {
        return parenthesize(expr.op.lexeme, {expr.left.get(), expr.right.get()});
    }

    std::any visitGroupingExpr(Grouping& expr) override {
        return parenthesize("group", {expr.expression.get()});
    }

    std::any visitLiteralExpr(Literal& expr) override {
        if (!expr.value.has_value()) {
            return std::string("nil");
        }

        // Handle different literal types
        if (expr.value.type() == typeid(double)) {
            return std::to_string(std::any_cast<double>(expr.value));
        }
        if (expr.value.type() == typeid(std::string)) {
            return std::any_cast<std::string>(expr.value);
        }
        if (expr.value.type() == typeid(bool)) {
            return std::any_cast<bool>(expr.value) ? std::string("true") : std::string("false");
        }

        return std::string("nil");
    }

    std::any visitUnaryExpr(Unary& expr) override {
        return parenthesize(expr.op.lexeme, {expr.right.get()});
    }

private:
    std::string parenthesize(const std::string& name, std::initializer_list<Expr*> exprs) {
        std::ostringstream ss{};
        ss << "(" << name;
        for (Expr* expr : exprs) {
            ss << " ";
            ss << std::any_cast<std::string>(expr->accept(*this));
        }
        ss << ")";
        return ss.str();
    }
};