#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

// ==================== Token 定义 ====================
enum class TokenType {
    INT, VOID, IF, ELSE, WHILE, BREAK, CONTINUE, RETURN,
    IDENT, INT_CONST,
    PLUS, MINUS, STAR, SLASH, MOD,
    LT, GT, LE, GE, EQ, NE,
    AND, OR, NOT, ASSIGN,
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, COMMA,
    END_OF_FILE
};

struct Token {
    TokenType type;
    string value;
    int line;
};

// ==================== 词法分析器 ====================
class Lexer {
private:
    string source;
    size_t pos = 0;
    int line = 1;
    vector<Token> tokens;

    char peek(int offset = 0) {
        if (pos + offset >= source.length()) return '\0';
        return source[pos + offset];
    }

    void advance() {
        if (pos < source.length()) {
            if (source[pos] == '\n') line++;
            pos++;
        }
    }

    void skipWhitespace() {
        while (pos < source.length() && isspace(peek())) advance();
    }

    void skipComment() {
        if (peek() == '/' && peek(1) == '/') {
            while (pos < source.length() && peek() != '\n') advance();
        } else if (peek() == '/' && peek(1) == '*') {
            advance(); advance();
            while (pos < source.length() - 1) {
                if (peek() == '*' && peek(1) == '/') {
                    advance(); advance();
                    break;
                }
                advance();
            }
        }
    }

    void addToken(TokenType type, const string& value) {
        tokens.push_back({type, value, line});
    }

public:
    Lexer(const string& src) : source(src) {}

    vector<Token> tokenize() {
        map<string, TokenType> keywords = {
            {"int", TokenType::INT}, {"void", TokenType::VOID},
            {"if", TokenType::IF}, {"else", TokenType::ELSE},
            {"while", TokenType::WHILE}, {"break", TokenType::BREAK},
            {"continue", TokenType::CONTINUE}, {"return", TokenType::RETURN}
        };

        while (pos < source.length()) {
            skipWhitespace();
            if (pos >= source.length()) break;

            if (peek() == '/' && (peek(1) == '/' || peek(1) == '*')) {
                skipComment();
                continue;
            }

            char ch = peek();

            if (isalpha(ch) || ch == '_') {
                string id;
                while (pos < source.length() && (isalnum(peek()) || peek() == '_')) {
                    id += peek();
                    advance();
                }
                addToken(keywords.count(id) ? keywords[id] : TokenType::IDENT, id);
                continue;
            }

            if (isdigit(ch)) {
                string num;
                while (pos < source.length() && isdigit(peek())) {
                    num += peek();
                    advance();
                }
                addToken(TokenType::INT_CONST, num);
                continue;
            }

            if (ch == '<' && peek(1) == '=') { addToken(TokenType::LE, "<="); advance(); advance(); continue; }
            if (ch == '>' && peek(1) == '=') { addToken(TokenType::GE, ">="); advance(); advance(); continue; }
            if (ch == '=' && peek(1) == '=') { addToken(TokenType::EQ, "=="); advance(); advance(); continue; }
            if (ch == '!' && peek(1) == '=') { addToken(TokenType::NE, "!="); advance(); advance(); continue; }
            if (ch == '&' && peek(1) == '&') { addToken(TokenType::AND, "&&"); advance(); advance(); continue; }
            if (ch == '|' && peek(1) == '|') { addToken(TokenType::OR, "||"); advance(); advance(); continue; }

            switch (ch) {
                case '+': addToken(TokenType::PLUS, "+"); break;
                case '-': addToken(TokenType::MINUS, "-"); break;
                case '*': addToken(TokenType::STAR, "*"); break;
                case '/': addToken(TokenType::SLASH, "/"); break;
                case '%': addToken(TokenType::MOD, "%"); break;
                case '<': addToken(TokenType::LT, "<"); break;
                case '>': addToken(TokenType::GT, ">"); break;
                case '=': addToken(TokenType::ASSIGN, "="); break;
                case '!': addToken(TokenType::NOT, "!"); break;
                case '(': addToken(TokenType::LPAREN, "("); break;
                case ')': addToken(TokenType::RPAREN, ")"); break;
                case '{': addToken(TokenType::LBRACE, "{"); break;
                case '}': addToken(TokenType::RBRACE, "}"); break;
                case ';': addToken(TokenType::SEMICOLON, ";"); break;
                case ',': addToken(TokenType::COMMA, ","); break;
            }
            advance();
        }

        addToken(TokenType::END_OF_FILE, "");
        return tokens;
    }
};

// ==================== AST 节点定义 ====================
struct ASTNode { virtual ~ASTNode() = default; };
struct Expr : ASTNode {};
struct Stmt : ASTNode {};

struct NumberExpr : Expr {
    int value;
    NumberExpr(int v) : value(v) {}
};

struct IdentExpr : Expr {
    string name;
    IdentExpr(const string& n) : name(n) {}
};

struct BinaryExpr : Expr {
    string op;
    unique_ptr<Expr> left, right;
    BinaryExpr(const string& o, unique_ptr<Expr> l, unique_ptr<Expr> r)
        : op(o), left(move(l)), right(move(r)) {}
};

struct UnaryExpr : Expr {
    string op;
    unique_ptr<Expr> operand;
    UnaryExpr(const string& o, unique_ptr<Expr> e) : op(o), operand(move(e)) {}
};

struct CallExpr : Expr {
    string funcName;
    vector<unique_ptr<Expr>> args;
    CallExpr(const string& name) : funcName(name) {}
};

struct BlockStmt : Stmt {
    vector<unique_ptr<Stmt>> stmts;
};

struct VarDeclStmt : Stmt {
    string name;
    unique_ptr<Expr> init;
    VarDeclStmt(const string& n, unique_ptr<Expr> i) : name(n), init(move(i)) {}
};

struct AssignStmt : Stmt {
    string name;
    unique_ptr<Expr> value;
    AssignStmt(const string& n, unique_ptr<Expr> v) : name(n), value(move(v)) {}
};

struct IfStmt : Stmt {
    unique_ptr<Expr> cond;
    unique_ptr<Stmt> thenStmt;
    unique_ptr<Stmt> elseStmt;
};

struct WhileStmt : Stmt {
    unique_ptr<Expr> cond;
    unique_ptr<Stmt> body;
};

struct BreakStmt : Stmt {};
struct ContinueStmt : Stmt {};

struct ReturnStmt : Stmt {
    unique_ptr<Expr> value;
    ReturnStmt(unique_ptr<Expr> v = nullptr) : value(move(v)) {}
};

struct ExprStmt : Stmt {
    unique_ptr<Expr> expr;
    ExprStmt(unique_ptr<Expr> e) : expr(move(e)) {}
};

struct EmptyStmt : Stmt {};

struct Param : ASTNode {
    string name;
    Param(const string& n) : name(n) {}
};

struct FuncDef : ASTNode {
    bool isVoid;
    string name;
    vector<unique_ptr<Param>> params;
    unique_ptr<BlockStmt> body;
};

struct Program : ASTNode {
    vector<unique_ptr<FuncDef>> functions;
};

// ==================== 语法分析器 ====================
class Parser {
private:
    vector<Token> tokens;
    size_t current = 0;

    Token& peek() { return tokens[current]; }
    Token& previous() { return tokens[current - 1]; }
    bool isAtEnd() { return peek().type == TokenType::END_OF_FILE; }
    bool check(TokenType type) { return !isAtEnd() && peek().type == type; }
    bool match(TokenType type) { if (check(type)) { current++; return true; } return false; }
    Token consume(TokenType type) { if (check(type)) return tokens[current++]; throw runtime_error("Parse error"); }

    unique_ptr<Program> parseProgram() {
        auto prog = make_unique<Program>();
        while (!isAtEnd()) prog->functions.push_back(parseFuncDef());
        return prog;
    }

    unique_ptr<FuncDef> parseFuncDef() {
        auto func = make_unique<FuncDef>();
        func->isVoid = check(TokenType::VOID);
        if (!match(TokenType::INT) && !match(TokenType::VOID)) throw runtime_error("Expected type");
        func->name = consume(TokenType::IDENT).value;
        consume(TokenType::LPAREN);
        if (check(TokenType::INT)) {
            do {
                consume(TokenType::INT);
                func->params.push_back(make_unique<Param>(consume(TokenType::IDENT).value));
            } while (match(TokenType::COMMA));
        }
        consume(TokenType::RPAREN);
        func->body = parseBlock();
        return func;
    }

    unique_ptr<BlockStmt> parseBlock() {
        auto block = make_unique<BlockStmt>();
        consume(TokenType::LBRACE);
        while (!check(TokenType::RBRACE) && !isAtEnd()) block->stmts.push_back(parseStmt());
        consume(TokenType::RBRACE);
        return block;
    }

    unique_ptr<Stmt> parseStmt() {
        if (check(TokenType::LBRACE)) return parseBlock();
        if (match(TokenType::SEMICOLON)) return make_unique<EmptyStmt>();
        if (match(TokenType::BREAK)) { consume(TokenType::SEMICOLON); return make_unique<BreakStmt>(); }
        if (match(TokenType::CONTINUE)) { consume(TokenType::SEMICOLON); return make_unique<ContinueStmt>(); }
        if (match(TokenType::RETURN)) {
            unique_ptr<Expr> val = nullptr;
            if (!check(TokenType::SEMICOLON)) val = parseExpr();
            consume(TokenType::SEMICOLON);
            return make_unique<ReturnStmt>(move(val));
        }
        if (match(TokenType::IF)) {
            auto stmt = make_unique<IfStmt>();
            consume(TokenType::LPAREN);
            stmt->cond = parseExpr();
            consume(TokenType::RPAREN);
            stmt->thenStmt = parseStmt();
            if (match(TokenType::ELSE)) stmt->elseStmt = parseStmt();
            return stmt;
        }
        if (match(TokenType::WHILE)) {
            auto stmt = make_unique<WhileStmt>();
            consume(TokenType::LPAREN);
            stmt->cond = parseExpr();
            consume(TokenType::RPAREN);
            stmt->body = parseStmt();
            return stmt;
        }
        if (match(TokenType::INT)) {
            string name = consume(TokenType::IDENT).value;
            consume(TokenType::ASSIGN);
            auto init = parseExpr();
            consume(TokenType::SEMICOLON);
            return make_unique<VarDeclStmt>(name, move(init));
        }
        if (check(TokenType::IDENT) && tokens[current + 1].type == TokenType::ASSIGN) {
            string name = consume(TokenType::IDENT).value;
            consume(TokenType::ASSIGN);
            auto val = parseExpr();
            consume(TokenType::SEMICOLON);
            return make_unique<AssignStmt>(name, move(val));
        }
        auto expr = parseExpr();
        consume(TokenType::SEMICOLON);
        return make_unique<ExprStmt>(move(expr));
    }

    unique_ptr<Expr> parseExpr() { return parseLOr(); }

    unique_ptr<Expr> parseLOr() {
        auto left = parseLAnd();
        while (match(TokenType::OR)) left = make_unique<BinaryExpr>("||", move(left), parseLAnd());
        return left;
    }

    unique_ptr<Expr> parseLAnd() {
        auto left = parseRel();
        while (match(TokenType::AND)) left = make_unique<BinaryExpr>("&&", move(left), parseRel());
        return left;
    }

    unique_ptr<Expr> parseRel() {
        auto left = parseAdd();
        while (true) {
            string op;
            if (match(TokenType::LT)) op = "<";
            else if (match(TokenType::GT)) op = ">";
            else if (match(TokenType::LE)) op = "<=";
            else if (match(TokenType::GE)) op = ">=";
            else if (match(TokenType::EQ)) op = "==";
            else if (match(TokenType::NE)) op = "!=";
            else break;
            left = make_unique<BinaryExpr>(op, move(left), parseAdd());
        }
        return left;
    }

    unique_ptr<Expr> parseAdd() {
        auto left = parseMul();
        while (true) {
            string op;
            if (match(TokenType::PLUS)) op = "+";
            else if (match(TokenType::MINUS)) op = "-";
            else break;
            left = make_unique<BinaryExpr>(op, move(left), parseMul());
        }
        return left;
    }

    unique_ptr<Expr> parseMul() {
        auto left = parseUnary();
        while (true) {
            string op;
            if (match(TokenType::STAR)) op = "*";
            else if (match(TokenType::SLASH)) op = "/";
            else if (match(TokenType::MOD)) op = "%";
            else break;
            left = make_unique<BinaryExpr>(op, move(left), parseUnary());
        }
        return left;
    }

    unique_ptr<Expr> parseUnary() {
        if (match(TokenType::PLUS)) return make_unique<UnaryExpr>("+", parseUnary());
        if (match(TokenType::MINUS)) return make_unique<UnaryExpr>("-", parseUnary());
        if (match(TokenType::NOT)) return make_unique<UnaryExpr>("!", parseUnary());
        return parsePrimary();
    }

    unique_ptr<Expr> parsePrimary() {
        if (match(TokenType::INT_CONST)) return make_unique<NumberExpr>(stoi(previous().value));
        if (match(TokenType::LPAREN)) {
            auto expr = parseExpr();
            consume(TokenType::RPAREN);
            return expr;
        }
        if (match(TokenType::IDENT)) {
            string name = previous().value;
            if (match(TokenType::LPAREN)) {
                auto call = make_unique<CallExpr>(name);
                if (!check(TokenType::RPAREN)) {
                    do { call->args.push_back(parseExpr()); } while (match(TokenType::COMMA));
                }
                consume(TokenType::RPAREN);
                return call;
            }
            return make_unique<IdentExpr>(name);
        }
        throw runtime_error("Expected expression");
    }

public:
    Parser(const vector<Token>& toks) : tokens(toks) {}
    unique_ptr<Program> parse() { return parseProgram(); }
};

// ==================== 代码生成器（直接生成汇编） ====================
class CodeGenerator {
private:
    ostringstream out;
    int labelCount = 0;
    int stackOffset = 0;
    int frameSize = 0;
    string currentFunc;
    vector<string> breakLabels;
    vector<string> continueLabels;

    // 变量名 -> 栈偏移（相对于s0的负偏移）
    vector<map<string, int>> varScopes;
    // 参数名 -> 参数索引
    map<string, int> paramIndex;

    string newLabel() { return "L" + to_string(labelCount++); }

    void emit(const string& s) { out << "\t" << s << "\n"; }
    void emitLabel(const string& s) { out << s << ":\n"; }

    // 分配栈空间给变量，返回偏移
    int allocVar(const string& name) {
        stackOffset -= 4;
        varScopes.back()[name] = stackOffset;
        return stackOffset;
    }

    // 查找变量的栈偏移，如果是参数返回-1并设置paramIdx
    int lookupVar(const string& name, int& paramIdx) {
        // 先查找局部变量
        for (int i = varScopes.size() - 1; i >= 0; i--) {
            if (varScopes[i].count(name)) {
                paramIdx = -1;
                return varScopes[i][name];
            }
        }
        // 查找参数
        if (paramIndex.count(name)) {
            paramIdx = paramIndex[name];
            return 0;  // 参数存在特定位置
        }
        throw runtime_error("Undefined variable: " + name);
    }

    // 生成表达式，结果存入t0
    void genExpr(Expr* expr) {
        if (auto* num = dynamic_cast<NumberExpr*>(expr)) {
            emit("li t0, " + to_string(num->value));
            return;
        }

        if (auto* ident = dynamic_cast<IdentExpr*>(expr)) {
            int paramIdx;
            int offset = lookupVar(ident->name, paramIdx);
            if (paramIdx >= 0) {
                // 从参数存储位置加载
                emit("lw t0, " + to_string(-4 - paramIdx * 4) + "(s0)");
            } else {
                emit("lw t0, " + to_string(offset) + "(s0)");
            }
            return;
        }

        if (auto* unary = dynamic_cast<UnaryExpr*>(expr)) {
            genExpr(unary->operand.get());
            if (unary->op == "-") {
                emit("neg t0, t0");
            } else if (unary->op == "!") {
                emit("seqz t0, t0");
            }
            // +x 不需要处理
            return;
        }

        if (auto* binary = dynamic_cast<BinaryExpr*>(expr)) {
            // 短路求值
            if (binary->op == "&&") {
                string falseLabel = newLabel();
                string endLabel = newLabel();
                genExpr(binary->left.get());
                emit("beqz t0, " + falseLabel);
                genExpr(binary->right.get());
                emit("beqz t0, " + falseLabel);
                emit("li t0, 1");
                emit("j " + endLabel);
                emitLabel(falseLabel);
                emit("li t0, 0");
                emitLabel(endLabel);
                return;
            }

            if (binary->op == "||") {
                string trueLabel = newLabel();
                string endLabel = newLabel();
                genExpr(binary->left.get());
                emit("bnez t0, " + trueLabel);
                genExpr(binary->right.get());
                emit("bnez t0, " + trueLabel);
                emit("li t0, 0");
                emit("j " + endLabel);
                emitLabel(trueLabel);
                emit("li t0, 1");
                emitLabel(endLabel);
                return;
            }

            // 普通二元运算
            genExpr(binary->left.get());
            emit("addi sp, sp, -4");
            emit("sw t0, 0(sp)");
            genExpr(binary->right.get());
            emit("mv t1, t0");
            emit("lw t0, 0(sp)");
            emit("addi sp, sp, 4");

            if (binary->op == "+") emit("add t0, t0, t1");
            else if (binary->op == "-") emit("sub t0, t0, t1");
            else if (binary->op == "*") emit("mul t0, t0, t1");
            else if (binary->op == "/") emit("div t0, t0, t1");
            else if (binary->op == "%") emit("rem t0, t0, t1");
            else if (binary->op == "<") emit("slt t0, t0, t1");
            else if (binary->op == ">") emit("slt t0, t1, t0");
            else if (binary->op == "<=") { emit("slt t0, t1, t0"); emit("xori t0, t0, 1"); }
            else if (binary->op == ">=") { emit("slt t0, t0, t1"); emit("xori t0, t0, 1"); }
            else if (binary->op == "==") { emit("sub t0, t0, t1"); emit("seqz t0, t0"); }
            else if (binary->op == "!=") { emit("sub t0, t0, t1"); emit("snez t0, t0"); }
            return;
        }

        if (auto* call = dynamic_cast<CallExpr*>(expr)) {
            // 保存参数到临时栈空间
            int argCount = call->args.size();
            if (argCount > 0) {
                emit("addi sp, sp, -" + to_string(argCount * 4));
                for (int i = 0; i < argCount; i++) {
                    genExpr(call->args[i].get());
                    emit("sw t0, " + to_string(i * 4) + "(sp)");
                }
                // 加载到参数寄存器
                for (int i = 0; i < argCount && i < 8; i++) {
                    emit("lw a" + to_string(i) + ", " + to_string(i * 4) + "(sp)");
                }
                emit("addi sp, sp, " + to_string(argCount * 4));
            }
            emit("call " + call->funcName);
            emit("mv t0, a0");
            return;
        }
    }

    void genStmt(Stmt* stmt) {
        if (auto* block = dynamic_cast<BlockStmt*>(stmt)) {
            varScopes.push_back({});
            for (auto& s : block->stmts) genStmt(s.get());
            varScopes.pop_back();
            return;
        }

        if (dynamic_cast<EmptyStmt*>(stmt)) return;

        if (auto* varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
            genExpr(varDecl->init.get());
            int offset = allocVar(varDecl->name);
            emit("sw t0, " + to_string(offset) + "(s0)");
            return;
        }

        if (auto* assign = dynamic_cast<AssignStmt*>(stmt)) {
            genExpr(assign->value.get());
            int paramIdx;
            int offset = lookupVar(assign->name, paramIdx);
            if (paramIdx >= 0) {
                emit("sw t0, " + to_string(-4 - paramIdx * 4) + "(s0)");
            } else {
                emit("sw t0, " + to_string(offset) + "(s0)");
            }
            return;
        }

        if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
            string elseLabel = newLabel();
            string endLabel = newLabel();
            genExpr(ifStmt->cond.get());
            emit("beqz t0, " + elseLabel);
            genStmt(ifStmt->thenStmt.get());
            if (ifStmt->elseStmt) emit("j " + endLabel);
            emitLabel(elseLabel);
            if (ifStmt->elseStmt) {
                genStmt(ifStmt->elseStmt.get());
                emitLabel(endLabel);
            }
            return;
        }

        if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
            string condLabel = newLabel();
            string endLabel = newLabel();
            breakLabels.push_back(endLabel);
            continueLabels.push_back(condLabel);
            emitLabel(condLabel);
            genExpr(whileStmt->cond.get());
            emit("beqz t0, " + endLabel);
            genStmt(whileStmt->body.get());
            emit("j " + condLabel);
            emitLabel(endLabel);
            breakLabels.pop_back();
            continueLabels.pop_back();
            return;
        }

        if (dynamic_cast<BreakStmt*>(stmt)) {
            emit("j " + breakLabels.back());
            return;
        }

        if (dynamic_cast<ContinueStmt*>(stmt)) {
            emit("j " + continueLabels.back());
            return;
        }

        if (auto* ret = dynamic_cast<ReturnStmt*>(stmt)) {
            if (ret->value) {
                genExpr(ret->value.get());
                emit("mv a0, t0");
            }
            emit("lw ra, " + to_string(frameSize - 4) + "(sp)");
            emit("lw s0, " + to_string(frameSize - 8) + "(sp)");
            emit("addi sp, sp, " + to_string(frameSize));
            emit("ret");
            return;
        }

        if (auto* exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
            genExpr(exprStmt->expr.get());
            return;
        }
    }

    // 计算函数需要的栈空间（遍历所有变量声明）
    int countLocalVars(Stmt* stmt) {
        int count = 0;
        if (auto* block = dynamic_cast<BlockStmt*>(stmt)) {
            for (auto& s : block->stmts) count += countLocalVars(s.get());
        } else if (dynamic_cast<VarDeclStmt*>(stmt)) {
            count = 1;
        } else if (auto* ifStmt = dynamic_cast<IfStmt*>(stmt)) {
            count = countLocalVars(ifStmt->thenStmt.get());
            if (ifStmt->elseStmt) count += countLocalVars(ifStmt->elseStmt.get());
        } else if (auto* whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
            count = countLocalVars(whileStmt->body.get());
        }
        return count;
    }

    void genFunc(FuncDef* func) {
        currentFunc = func->name;
        stackOffset = 0;
        paramIndex.clear();
        varScopes.clear();
        varScopes.push_back({});  // 函数作用域

        // 计算需要的栈空间
        int paramCount = func->params.size();
        int localVarCount = countLocalVars(func->body.get());

        // ra + s0 + 参数存储 + 局部变量 + 临时空间
        int neededSpace = 8 + paramCount * 4 + localVarCount * 4 + 128;
        frameSize = ((neededSpace + 15) / 16) * 16;

        // 设置参数索引
        for (int i = 0; i < paramCount; i++) {
            paramIndex[func->params[i]->name] = i;
        }

        // 函数标签
        out << ".globl " << func->name << "\n";
        emitLabel(func->name);
        emitLabel("prologue_" + func->name);

        // 序言
        emit("addi sp, sp, -" + to_string(frameSize));
        emit("sw ra, " + to_string(frameSize - 4) + "(sp)");
        emit("sw s0, " + to_string(frameSize - 8) + "(sp)");
        emit("addi s0, sp, " + to_string(frameSize));

        // 保存参数
        for (int i = 0; i < paramCount && i < 8; i++) {
            emit("sw a" + to_string(i) + ", " + to_string(-4 - i * 4) + "(s0)");
        }

        // 设置局部变量起始偏移（在参数之后）
        stackOffset = -4 - paramCount * 4;

        // 生成函数体
        for (auto& stmt : func->body->stmts) {
            genStmt(stmt.get());
        }

        out << "\n";
    }

public:
    string generate(Program* prog) {
        out << ".text\n\n";
        for (auto& func : prog->functions) {
            genFunc(func.get());
        }
        return out.str();
    }
};

// ==================== 主函数 ====================
int main(int argc, char* argv[]) {
    string source, line;
    while (getline(cin, line)) source += line + "\n";

    try {
        Lexer lexer(source);
        vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        auto ast = parser.parse();

        CodeGenerator codeGen;
        cout << codeGen.generate(ast.get());

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
