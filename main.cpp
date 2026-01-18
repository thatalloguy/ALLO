#include <fstream>
#include <iostream>
#include <vector>

#include "Allo/Interpreter.h"
#include "Allo/Expr/AstPrinter.h"

struct Field {
    std::string type;
    std::string name;
};

struct TypeDef {
    std::string className;
    std::vector<Field> fields;
};

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

TypeDef parseType(const std::string& line) {
    TypeDef result;
    size_t colonPos = line.find(':');
    result.className = trim(line.substr(0, colonPos));

    std::string fieldList = trim(line.substr(colonPos + 1));

    size_t pos = 0;
    while (pos < fieldList.length()) {
        size_t comma = fieldList.find(',', pos);
        if (comma == std::string::npos) comma = fieldList.length();

        std::string field = trim(fieldList.substr(pos, comma - pos));
        size_t space = field.rfind(' ');

        Field f{};
        f.type = trim(field.substr(0, space));
        f.name = trim(field.substr(space + 1));
        result.fields.push_back(f);

        pos = comma + 1;
    }
    return result;
}

void defineAst(const std::string& outputDir, const std::string& baseName,
               const std::vector<std::string>& types) {
    std::string path = outputDir + baseName + ".h";
    std::ofstream out(path);

    std::vector<TypeDef> typeDefs;
    for (const std::string& t : types) {
        typeDefs.push_back(parseType(t));
    }

    out << "#pragma once\n";
    out << "#include <any>\n";
    out << "#include <memory>\n";
    out << "#include \"../Token.h\"\n\n";

    // Forward declarations
    for (const TypeDef& t : typeDefs) {
        out << "class " << t.className << ";\n";
    }
    out << "\n";

    // Visitor interface
    out << "template<typename R>\n";
    out << "class " << baseName << "Visitor {\n";
    out << "public:\n";
    out << "    virtual ~" << baseName << "Visitor() = default;\n";
    for (const TypeDef& t : typeDefs) {
        out << "    virtual R visit" << t.className << baseName
            << "(" << t.className << "& " << char(tolower(baseName[0]))
            << baseName.substr(1) << ") = 0;\n";
    }
    out << "};\n\n";

    out << "class " << baseName << " {\n";
    out << "public:\n";
    out << "    virtual ~" << baseName << "() = default;\n";
    out << "    virtual std::any accept(" << baseName
        << "Visitor<std::any>& visitor) = 0;\n";
    out << "};\n\n";

    out << "using " << baseName << "Ptr = std::unique_ptr<" << baseName << ">;\n\n";

    for (const TypeDef& t : typeDefs) {
        out << "class " << t.className << " : public " << baseName << " {\n";
        out << "public:\n";

        out << "    " << t.className << "(";
        for (size_t i = 0; i < t.fields.size(); i++) {
            const Field& f = t.fields[i];
            if (f.type == "Expr" || f.type == baseName) {
                out << baseName << "Ptr " << f.name;
            } else if (f.type == "Token") {
                out << "Token " << f.name;
            } else {
                out << "std::any " << f.name;
            }
            if (i < t.fields.size() - 1) out << ", ";
        }
        out << ")\n";

        out << "        : ";
        for (size_t i = 0; i < t.fields.size(); i++) {
            const Field& f = t.fields[i];
            out << f.name << "(std::move(" << f.name << "))";
            if (i < t.fields.size() - 1) out << ", ";
        }
        out << " {}\n\n";

        out << "    std::any accept(" << baseName
            << "Visitor<std::any>& visitor) override {\n";
        out << "        return visitor.visit" << t.className << baseName
            << "(*this);\n";
        out << "    }\n\n";

        for (const Field& f : t.fields) {
            if (f.type == "Expr" || f.type == baseName) {
                out << "    " << baseName << "Ptr " << f.name << "{};\n";
            } else if (f.type == "Token") {
                out << "    Token " << f.name << "{};\n";
            } else {
                out << "    std::any " << f.name << "{};\n";
            }
        }
        out << "};\n\n";
    }

    out.close();
    std::cout << "Generated " << path << "\n";
}

int main(int argc, char *argv[]) {

    /*
    defineAst("../Allo/Expr/", "Expr", {
          "Binary : Expr left, Token op, Expr right",
          "Grouping : Expr expression",
          "Literal : Object value",
          "Unary : Token op, Expr right"
        });*/


    /*if (argc > 2) {
        std::cout << "Usage: ALLO [script or prompt]\n";
        return 64;
    }

    Interpreter interpreter{};

    if (argc == 2) {
        interpreter.runFile(argv[1]);
    } else {
        interpreter.runPrompt();
    }*/

    auto expr = std::make_unique<Binary>(
       std::make_unique<Unary>(
           Token(MINUS, "-", nullptr, 1),
           std::make_unique<Literal>(123.0)
       ),
       Token(STAR, "*", nullptr, 1),
       std::make_unique<Grouping>(
           std::make_unique<Literal>(45.67)
       )
   );

    AstPrinter printer{};
    std::cout << printer.print(*expr) << std::endl;

    return 0;
}
