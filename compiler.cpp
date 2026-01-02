#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <cctype>
#include <cmath>
#include <tuple>
#include <algorithm>

using namespace std;

// 全局优化开关
bool g_optimize = false;

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
        // 优化：使用静态 unordered_map，只初始化一次，O(1) 查找
        static const unordered_map<string, TokenType> keywords = {
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
                auto it = keywords.find(id);
                addToken(it != keywords.end() ? it->second : TokenType::IDENT, id);
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
// 优化：使用枚举类型标识节点，避免 dynamic_cast 的 RTTI 开销
enum class ExprKind { NUMBER, IDENT, BINARY, UNARY, CALL };
enum class StmtKind { BLOCK, VARDECL, ASSIGN, IF, WHILE, BREAK, CONTINUE, RETURN, EXPR, EMPTY };

struct ASTNode { virtual ~ASTNode() = default; };

struct Expr : ASTNode {
    ExprKind kind;
    Expr(ExprKind k) : kind(k) {}
};

struct Stmt : ASTNode {
    StmtKind kind;
    Stmt(StmtKind k) : kind(k) {}
};

struct NumberExpr : Expr {
    int value;
    NumberExpr(int v) : Expr(ExprKind::NUMBER), value(v) {}
};

struct IdentExpr : Expr {
    string name;
    IdentExpr(const string& n) : Expr(ExprKind::IDENT), name(n) {}
};

struct BinaryExpr : Expr {
    string op;
    unique_ptr<Expr> left, right;
    BinaryExpr(const string& o, unique_ptr<Expr> l, unique_ptr<Expr> r)
        : Expr(ExprKind::BINARY), op(o), left(move(l)), right(move(r)) {}
};

struct UnaryExpr : Expr {
    string op;
    unique_ptr<Expr> operand;
    UnaryExpr(const string& o, unique_ptr<Expr> e) : Expr(ExprKind::UNARY), op(o), operand(move(e)) {}
};

struct CallExpr : Expr {
    string funcName;
    vector<unique_ptr<Expr>> args;
    CallExpr(const string& name) : Expr(ExprKind::CALL), funcName(name) {}
};

struct BlockStmt : Stmt {
    vector<unique_ptr<Stmt>> stmts;
    BlockStmt() : Stmt(StmtKind::BLOCK) {}
};

struct VarDeclStmt : Stmt {
    string name;
    unique_ptr<Expr> init;
    VarDeclStmt(const string& n, unique_ptr<Expr> i) : Stmt(StmtKind::VARDECL), name(n), init(move(i)) {}
};

struct AssignStmt : Stmt {
    string name;
    unique_ptr<Expr> value;
    AssignStmt(const string& n, unique_ptr<Expr> v) : Stmt(StmtKind::ASSIGN), name(n), value(move(v)) {}
};

struct IfStmt : Stmt {
    unique_ptr<Expr> cond;
    unique_ptr<Stmt> thenStmt;
    unique_ptr<Stmt> elseStmt;
    IfStmt() : Stmt(StmtKind::IF) {}
};

struct WhileStmt : Stmt {
    unique_ptr<Expr> cond;
    unique_ptr<Stmt> body;
    WhileStmt() : Stmt(StmtKind::WHILE) {}
};

struct BreakStmt : Stmt {
    BreakStmt() : Stmt(StmtKind::BREAK) {}
};

struct ContinueStmt : Stmt {
    ContinueStmt() : Stmt(StmtKind::CONTINUE) {}
};

struct ReturnStmt : Stmt {
    unique_ptr<Expr> value;
    ReturnStmt(unique_ptr<Expr> v = nullptr) : Stmt(StmtKind::RETURN), value(move(v)) {}
};

struct ExprStmt : Stmt {
    unique_ptr<Expr> expr;
    ExprStmt(unique_ptr<Expr> e) : Stmt(StmtKind::EXPR), expr(move(e)) {}
};

struct EmptyStmt : Stmt {
    EmptyStmt() : Stmt(StmtKind::EMPTY) {}
};

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
        while (!isAtEnd()) {
            if (check(TokenType::INT) || check(TokenType::VOID)) {
                prog->functions.push_back(parseFuncDef());
                continue;
            }
            throw runtime_error("Expected function definition");
        }
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
        if (check(TokenType::IDENT) && current + 1 < tokens.size() && tokens[current + 1].type == TokenType::ASSIGN) {
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

// ==================== 优化器（-opt 启用时使用） ====================
class Optimizer {
private:
    // 常量传播表：变量名 -> 常量值
    map<string, int> constVars;
    // 复制传播表：变量名 -> 源变量名
    map<string, string> copyVars;
    // 变量使用计数
    map<string, int> varUseCount;
    // 当前函数名（用于尾递归检测）
    string currentFunc;
    // 当前函数参数
    vector<string> currentParams;
    // 函数表（用于内联）
    map<string, FuncDef*> funcTable;
    // 函数调用计数
    map<string, int> funcCallCount;
    // 内联计数器（用于生成唯一变量名）
    int inlineCount = 0;

    // 检查表达式是否为常量
    bool isConstExpr(Expr* expr) {
        return expr->kind == ExprKind::NUMBER;
    }

    // 获取常量值
    int getConstValue(Expr* expr) {
        return static_cast<NumberExpr*>(expr)->value;
    }

    // 检查是否是2的幂
    bool isPowerOfTwo(int n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    // 获取log2值
    int log2Int(int n) {
        int r = 0;
        while (n > 1) { n >>= 1; r++; }
        return r;
    }

    // 深拷贝表达式
    unique_ptr<Expr> cloneExpr(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return make_unique<NumberExpr>(static_cast<NumberExpr*>(expr)->value);
        case ExprKind::IDENT:
            return make_unique<IdentExpr>(static_cast<IdentExpr*>(expr)->name);
        case ExprKind::UNARY: {
            auto* u = static_cast<UnaryExpr*>(expr);
            return make_unique<UnaryExpr>(u->op, cloneExpr(u->operand.get()));
        }
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return make_unique<BinaryExpr>(b->op, cloneExpr(b->left.get()), cloneExpr(b->right.get()));
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            auto newCall = make_unique<CallExpr>(c->funcName);
            for (auto& arg : c->args) {
                newCall->args.push_back(cloneExpr(arg.get()));
            }
            return newCall;
        }
        }
        return nullptr;
    }

    // 表达式转字符串（用于公共子表达式消除）
    string exprToString(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return to_string(static_cast<NumberExpr*>(expr)->value);
        case ExprKind::IDENT:
            return static_cast<IdentExpr*>(expr)->name;
        case ExprKind::UNARY: {
            auto* u = static_cast<UnaryExpr*>(expr);
            return "(" + u->op + exprToString(u->operand.get()) + ")";
        }
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return "(" + exprToString(b->left.get()) + b->op + exprToString(b->right.get()) + ")";
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            string s = c->funcName + "(";
            for (size_t i = 0; i < c->args.size(); i++) {
                if (i > 0) s += ",";
                s += exprToString(c->args[i].get());
            }
            return s + ")";
        }
        }
        return "";
    }

    // 检查表达式是否使用了某个变量
    bool exprUsesVar(Expr* expr, const string& var) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return false;
        case ExprKind::IDENT:
            return static_cast<IdentExpr*>(expr)->name == var;
        case ExprKind::UNARY:
            return exprUsesVar(static_cast<UnaryExpr*>(expr)->operand.get(), var);
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return exprUsesVar(b->left.get(), var) || exprUsesVar(b->right.get(), var);
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            for (auto& arg : c->args) {
                if (exprUsesVar(arg.get(), var)) return true;
            }
            return false;
        }
        }
        return false;
    }

    // 检查表达式是否包含函数调用（有副作用）
    bool hasCallExpr(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
        case ExprKind::IDENT:
            return false;
        case ExprKind::UNARY:
            return hasCallExpr(static_cast<UnaryExpr*>(expr)->operand.get());
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return hasCallExpr(b->left.get()) || hasCallExpr(b->right.get());
        }
        case ExprKind::CALL:
            return true;
        }
        return false;
    }

    // 统计变量使用
    void countVarUse(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::IDENT:
            varUseCount[static_cast<IdentExpr*>(expr)->name]++;
            break;
        case ExprKind::UNARY:
            countVarUse(static_cast<UnaryExpr*>(expr)->operand.get());
            break;
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            countVarUse(b->left.get());
            countVarUse(b->right.get());
            break;
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            for (auto& arg : c->args) countVarUse(arg.get());
            break;
        }
        default:
            break;
        }
    }

    void countVarUseInStmt(Stmt* stmt) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) countVarUseInStmt(s.get());
            break;
        }
        case StmtKind::VARDECL: {
            auto* v = static_cast<VarDeclStmt*>(stmt);
            countVarUse(v->init.get());
            break;
        }
        case StmtKind::ASSIGN: {
            auto* a = static_cast<AssignStmt*>(stmt);
            countVarUse(a->value.get());
            break;
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            countVarUse(i->cond.get());
            countVarUseInStmt(i->thenStmt.get());
            if (i->elseStmt) countVarUseInStmt(i->elseStmt.get());
            break;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            countVarUse(w->cond.get());
            countVarUseInStmt(w->body.get());
            break;
        }
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) countVarUse(r->value.get());
            break;
        }
        case StmtKind::EXPR: {
            auto* e = static_cast<ExprStmt*>(stmt);
            countVarUse(e->expr.get());
            break;
        }
        default:
            break;
        }
    }

    // 常量折叠 + 常量传播 + 复制传播 + 代数简化 + 强度削减
    unique_ptr<Expr> foldExpr(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return make_unique<NumberExpr>(static_cast<NumberExpr*>(expr)->value);

        case ExprKind::IDENT: {
            string name = static_cast<IdentExpr*>(expr)->name;
            // 常量传播
            if (constVars.count(name)) {
                return make_unique<NumberExpr>(constVars[name]);
            }
            // 复制传播
            if (copyVars.count(name)) {
                string src = copyVars[name];
                // 传递性复制传播
                while (copyVars.count(src)) src = copyVars[src];
                if (constVars.count(src)) {
                    return make_unique<NumberExpr>(constVars[src]);
                }
                return make_unique<IdentExpr>(src);
            }
            return make_unique<IdentExpr>(name);
        }

        case ExprKind::UNARY: {
            auto* unary = static_cast<UnaryExpr*>(expr);
            auto operand = foldExpr(unary->operand.get());

            if (operand->kind == ExprKind::NUMBER) {
                int val = static_cast<NumberExpr*>(operand.get())->value;
                if (unary->op == "-") return make_unique<NumberExpr>(-val);
                if (unary->op == "!") return make_unique<NumberExpr>(val == 0 ? 1 : 0);
                if (unary->op == "+") return make_unique<NumberExpr>(val);
            }
            // 双重否定消除: --x = x, !!x = (x != 0)
            if (unary->op == "-" && operand->kind == ExprKind::UNARY) {
                auto* inner = static_cast<UnaryExpr*>(operand.get());
                if (inner->op == "-") {
                    return cloneExpr(inner->operand.get());
                }
            }
            return make_unique<UnaryExpr>(unary->op, move(operand));
        }

        case ExprKind::BINARY: {
            auto* binary = static_cast<BinaryExpr*>(expr);
            auto left = foldExpr(binary->left.get());
            auto right = foldExpr(binary->right.get());

            // 常量折叠
            if (left->kind == ExprKind::NUMBER && right->kind == ExprKind::NUMBER) {
                int l = static_cast<NumberExpr*>(left.get())->value;
                int r = static_cast<NumberExpr*>(right.get())->value;
                int result = 0;

                if (binary->op == "+") result = l + r;
                else if (binary->op == "-") result = l - r;
                else if (binary->op == "*") result = l * r;
                else if (binary->op == "/" && r != 0) result = l / r;
                else if (binary->op == "%" && r != 0) result = l % r;
                else if (binary->op == "<") result = l < r ? 1 : 0;
                else if (binary->op == ">") result = l > r ? 1 : 0;
                else if (binary->op == "<=") result = l <= r ? 1 : 0;
                else if (binary->op == ">=") result = l >= r ? 1 : 0;
                else if (binary->op == "==") result = l == r ? 1 : 0;
                else if (binary->op == "!=") result = l != r ? 1 : 0;
                else if (binary->op == "&&") result = (l && r) ? 1 : 0;
                else if (binary->op == "||") result = (l || r) ? 1 : 0;
                else {
                    return make_unique<BinaryExpr>(binary->op, move(left), move(right));
                }
                return make_unique<NumberExpr>(result);
            }

            // 代数简化
            if (binary->op == "+") {
                // x + 0 = x
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 0)
                    return left;
                // 0 + x = x
                if (left->kind == ExprKind::NUMBER && getConstValue(left.get()) == 0)
                    return right;
                // x + x = 2 * x (但保持原样让后端处理)
            }
            if (binary->op == "-") {
                // x - 0 = x
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 0)
                    return left;
                // 0 - x = -x
                if (left->kind == ExprKind::NUMBER && getConstValue(left.get()) == 0)
                    return make_unique<UnaryExpr>("-", move(right));
                // x - x = 0
                if (left->kind == ExprKind::IDENT && right->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(left.get())->name == static_cast<IdentExpr*>(right.get())->name)
                    return make_unique<NumberExpr>(0);
            }
            if (binary->op == "*") {
                // x * 0 = 0
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 0)
                    return make_unique<NumberExpr>(0);
                if (left->kind == ExprKind::NUMBER && getConstValue(left.get()) == 0)
                    return make_unique<NumberExpr>(0);
                // x * 1 = x
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 1)
                    return left;
                if (left->kind == ExprKind::NUMBER && getConstValue(left.get()) == 1)
                    return right;
                // x * -1 = -x
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == -1)
                    return make_unique<UnaryExpr>("-", move(left));
                if (left->kind == ExprKind::NUMBER && getConstValue(left.get()) == -1)
                    return make_unique<UnaryExpr>("-", move(right));
            }
            if (binary->op == "/") {
                // x / 1 = x
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 1)
                    return left;
                // x / -1 = -x
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == -1)
                    return make_unique<UnaryExpr>("-", move(left));
                // x / x = 1 (assuming x != 0)
                if (left->kind == ExprKind::IDENT && right->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(left.get())->name == static_cast<IdentExpr*>(right.get())->name)
                    return make_unique<NumberExpr>(1);
                // 0 / x = 0 (assuming x != 0)
                if (left->kind == ExprKind::NUMBER && getConstValue(left.get()) == 0)
                    return make_unique<NumberExpr>(0);
            }
            if (binary->op == "%") {
                // x % 1 = 0
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 1)
                    return make_unique<NumberExpr>(0);
                // x % x = 0 (assuming x != 0)
                if (left->kind == ExprKind::IDENT && right->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(left.get())->name == static_cast<IdentExpr*>(right.get())->name)
                    return make_unique<NumberExpr>(0);
                // 0 % x = 0
                if (left->kind == ExprKind::NUMBER && getConstValue(left.get()) == 0)
                    return make_unique<NumberExpr>(0);
            }
            // 比较优化
            if (binary->op == "<" || binary->op == ">") {
                // x < x = 0, x > x = 0
                if (left->kind == ExprKind::IDENT && right->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(left.get())->name == static_cast<IdentExpr*>(right.get())->name)
                    return make_unique<NumberExpr>(0);
            }
            if (binary->op == "<=" || binary->op == ">=") {
                // x <= x = 1, x >= x = 1
                if (left->kind == ExprKind::IDENT && right->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(left.get())->name == static_cast<IdentExpr*>(right.get())->name)
                    return make_unique<NumberExpr>(1);
            }
            if (binary->op == "==") {
                // x == x = 1
                if (left->kind == ExprKind::IDENT && right->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(left.get())->name == static_cast<IdentExpr*>(right.get())->name)
                    return make_unique<NumberExpr>(1);
            }
            if (binary->op == "!=") {
                // x != x = 0
                if (left->kind == ExprKind::IDENT && right->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(left.get())->name == static_cast<IdentExpr*>(right.get())->name)
                    return make_unique<NumberExpr>(0);
            }
            // 短路求值优化
            if (binary->op == "&&") {
                if (left->kind == ExprKind::NUMBER) {
                    if (getConstValue(left.get()) == 0) return make_unique<NumberExpr>(0);
                    else return right;
                }
                if (right->kind == ExprKind::NUMBER) {
                    if (getConstValue(right.get()) == 0) return make_unique<NumberExpr>(0);
                    // 1 && x = x (需要保证x被求值，但结果就是x)
                }
            }
            if (binary->op == "||") {
                if (left->kind == ExprKind::NUMBER) {
                    if (getConstValue(left.get()) != 0) return make_unique<NumberExpr>(1);
                    else return right;
                }
                if (right->kind == ExprKind::NUMBER) {
                    if (getConstValue(right.get()) != 0) return make_unique<NumberExpr>(1);
                    // 0 || x = x
                }
            }

            return make_unique<BinaryExpr>(binary->op, move(left), move(right));
        }

        case ExprKind::CALL: {
            auto* call = static_cast<CallExpr*>(expr);
            auto newCall = make_unique<CallExpr>(call->funcName);
            for (auto& arg : call->args) {
                newCall->args.push_back(foldExpr(arg.get()));
            }
            return newCall;
        }
        }
        return nullptr;
    }

    // 检查语句是否是尾递归调用
    bool isTailRecursiveReturn(Stmt* stmt) {
        if (stmt->kind != StmtKind::RETURN) return false;
        auto* ret = static_cast<ReturnStmt*>(stmt);
        if (!ret->value || ret->value->kind != ExprKind::CALL) return false;
        auto* call = static_cast<CallExpr*>(ret->value.get());
        return call->funcName == currentFunc;
    }

    // 优化语句列表，返回是否有修改
    bool optimizeStmtList(vector<unique_ptr<Stmt>>& stmts) {
        bool changed = false;

        // 死代码消除：删除 return 后的语句
        for (size_t i = 0; i < stmts.size(); i++) {
            if (stmts[i]->kind == StmtKind::RETURN ||
                stmts[i]->kind == StmtKind::BREAK ||
                stmts[i]->kind == StmtKind::CONTINUE) {
                if (i + 1 < stmts.size()) {
                    stmts.erase(stmts.begin() + i + 1, stmts.end());
                    changed = true;
                }
                break;
            }
        }

        // 优化每条语句
        for (auto it = stmts.begin(); it != stmts.end(); ) {
            Stmt* stmt = it->get();

            switch (stmt->kind) {
            case StmtKind::BLOCK: {
                auto* block = static_cast<BlockStmt*>(stmt);
                optimizeStmtList(block->stmts);
                // 空块消除
                if (block->stmts.empty()) {
                    it = stmts.erase(it);
                    changed = true;
                    continue;
                }
                break;
            }
            case StmtKind::EMPTY:
                it = stmts.erase(it);
                changed = true;
                continue;

            case StmtKind::VARDECL: {
                auto* varDecl = static_cast<VarDeclStmt*>(stmt);
                varDecl->init = foldExpr(varDecl->init.get());
                // 记录常量
                if (varDecl->init->kind == ExprKind::NUMBER) {
                    constVars[varDecl->name] = getConstValue(varDecl->init.get());
                }
                // 记录复制
                else if (varDecl->init->kind == ExprKind::IDENT) {
                    copyVars[varDecl->name] = static_cast<IdentExpr*>(varDecl->init.get())->name;
                }
                break;
            }
            case StmtKind::ASSIGN: {
                auto* assign = static_cast<AssignStmt*>(stmt);
                assign->value = foldExpr(assign->value.get());
                // 更新常量表
                constVars.erase(assign->name);
                copyVars.erase(assign->name);
                if (assign->value->kind == ExprKind::NUMBER) {
                    constVars[assign->name] = getConstValue(assign->value.get());
                } else if (assign->value->kind == ExprKind::IDENT) {
                    copyVars[assign->name] = static_cast<IdentExpr*>(assign->value.get())->name;
                }
                // 自赋值消除: x = x
                if (assign->value->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(assign->value.get())->name == assign->name) {
                    it = stmts.erase(it);
                    changed = true;
                    continue;
                }
                break;
            }
            case StmtKind::IF: {
                auto* ifStmt = static_cast<IfStmt*>(stmt);
                ifStmt->cond = foldExpr(ifStmt->cond.get());

                // 条件为常量：死代码消除
                if (ifStmt->cond->kind == ExprKind::NUMBER) {
                    int val = getConstValue(ifStmt->cond.get());
                    if (val != 0) {
                        // if(true) -> 只保留 then 分支
                        *it = move(ifStmt->thenStmt);
                        changed = true;
                    } else {
                        // if(false) -> 只保留 else 分支或删除
                        if (ifStmt->elseStmt) {
                            *it = move(ifStmt->elseStmt);
                            changed = true;
                        } else {
                            it = stmts.erase(it);
                            changed = true;
                            continue;
                        }
                    }
                } else {
                    // 清除条件分支中的常量信息（保守分析）
                    map<string, int> savedConst = constVars;
                    map<string, string> savedCopy = copyVars;
                    if (ifStmt->thenStmt->kind == StmtKind::BLOCK) {
                        optimizeStmtList(static_cast<BlockStmt*>(ifStmt->thenStmt.get())->stmts);
                    }
                    constVars = savedConst;
                    copyVars = savedCopy;
                    if (ifStmt->elseStmt) {
                        if (ifStmt->elseStmt->kind == StmtKind::BLOCK) {
                            optimizeStmtList(static_cast<BlockStmt*>(ifStmt->elseStmt.get())->stmts);
                        }
                    }
                    constVars = savedConst;
                    copyVars = savedCopy;
                }
                break;
            }
            case StmtKind::WHILE: {
                auto* whileStmt = static_cast<WhileStmt*>(stmt);

                // 先收集循环体内修改的变量，从常量表中移除
                set<string> modifiedInLoop;
                collectModifiedVars(whileStmt->body.get(), modifiedInLoop);
                for (const auto& var : modifiedInLoop) {
                    constVars.erase(var);
                    copyVars.erase(var);
                }

                // 现在再折叠条件
                whileStmt->cond = foldExpr(whileStmt->cond.get());

                // while(0) 消除
                if (whileStmt->cond->kind == ExprKind::NUMBER &&
                    getConstValue(whileStmt->cond.get()) == 0) {
                    it = stmts.erase(it);
                    changed = true;
                    continue;
                }

                // 循环内的优化（保守：清除所有常量信息）
                constVars.clear();
                copyVars.clear();
                if (whileStmt->body->kind == StmtKind::BLOCK) {
                    optimizeStmtList(static_cast<BlockStmt*>(whileStmt->body.get())->stmts);
                }
                break;
            }
            case StmtKind::RETURN: {
                auto* ret = static_cast<ReturnStmt*>(stmt);
                if (ret->value) ret->value = foldExpr(ret->value.get());
                break;
            }
            case StmtKind::EXPR: {
                auto* exprStmt = static_cast<ExprStmt*>(stmt);
                exprStmt->expr = foldExpr(exprStmt->expr.get());
                // 删除无副作用的表达式语句
                if (!hasCallExpr(exprStmt->expr.get())) {
                    it = stmts.erase(it);
                    changed = true;
                    continue;
                }
                break;
            }
            default:
                break;
            }
            ++it;
        }

        return changed;
    }

    // 尾递归优化
    void optimizeTailRecursion(FuncDef* func) {
        if (func->isVoid) return;  // void 函数不处理

        currentFunc = func->name;
        currentParams.clear();
        for (auto& p : func->params) {
            currentParams.push_back(p->name);
        }

        auto& stmts = func->body->stmts;

        // 查找尾递归 return
        for (size_t i = 0; i < stmts.size(); i++) {
            if (isTailRecursiveReturn(stmts[i].get())) {
                // 转换为循环
                auto* ret = static_cast<ReturnStmt*>(stmts[i].get());
                auto* call = static_cast<CallExpr*>(ret->value.get());

                // 创建循环体
                auto loopBody = make_unique<BlockStmt>();

                // 创建参数更新语句
                vector<unique_ptr<Expr>> newArgs;
                for (auto& arg : call->args) {
                    newArgs.push_back(cloneExpr(arg.get()));
                }

                // 使用临时变量避免覆盖
                for (size_t j = 0; j < currentParams.size() && j < newArgs.size(); j++) {
                    string tmpName = "__tmp_" + to_string(j);
                    loopBody->stmts.push_back(make_unique<VarDeclStmt>(tmpName, move(newArgs[j])));
                }
                for (size_t j = 0; j < currentParams.size() && j < call->args.size(); j++) {
                    string tmpName = "__tmp_" + to_string(j);
                    loopBody->stmts.push_back(make_unique<AssignStmt>(currentParams[j],
                        make_unique<IdentExpr>(tmpName)));
                }

                // 创建 while(1) 循环
                auto whileStmt = make_unique<WhileStmt>();
                whileStmt->cond = make_unique<NumberExpr>(1);

                // 将原来的语句移到循环体开头（除了尾递归 return）
                auto outerBody = make_unique<BlockStmt>();
                for (size_t j = 0; j < i; j++) {
                    outerBody->stmts.push_back(move(stmts[j]));
                }
                // 添加 continue
                loopBody->stmts.push_back(make_unique<ContinueStmt>());

                // 组合循环体：反向遍历以保持原始顺序
                for (auto it = outerBody->stmts.rbegin(); it != outerBody->stmts.rend(); ++it) {
                    static_cast<BlockStmt*>(loopBody.get())->stmts.insert(
                        static_cast<BlockStmt*>(loopBody.get())->stmts.begin(),
                        move(*it));
                }

                whileStmt->body = move(loopBody);

                // 重构函数体
                stmts.clear();
                stmts.push_back(move(whileStmt));
                break;
            }
        }
    }

    // 收集循环内被修改的变量
    void collectModifiedVars(Stmt* stmt, set<string>& modified) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) collectModifiedVars(s.get(), modified);
            break;
        }
        case StmtKind::VARDECL:
            modified.insert(static_cast<VarDeclStmt*>(stmt)->name);
            break;
        case StmtKind::ASSIGN:
            modified.insert(static_cast<AssignStmt*>(stmt)->name);
            break;
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            collectModifiedVars(i->thenStmt.get(), modified);
            if (i->elseStmt) collectModifiedVars(i->elseStmt.get(), modified);
            break;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            collectModifiedVars(w->body.get(), modified);
            break;
        }
        default:
            break;
        }
    }

    // 检查表达式是否是循环不变量
    bool isLoopInvariant(Expr* expr, const set<string>& modifiedVars) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return true;
        case ExprKind::IDENT:
            return modifiedVars.find(static_cast<IdentExpr*>(expr)->name) == modifiedVars.end();
        case ExprKind::UNARY:
            return isLoopInvariant(static_cast<UnaryExpr*>(expr)->operand.get(), modifiedVars);
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return isLoopInvariant(b->left.get(), modifiedVars) &&
                   isLoopInvariant(b->right.get(), modifiedVars);
        }
        case ExprKind::CALL:
            return false;  // 函数调用不是循环不变量（可能有副作用）
        }
        return false;
    }

    // 循环不变量外提
    void hoistLoopInvariants(WhileStmt* whileStmt, vector<unique_ptr<Stmt>>& hoisted) {
        if (whileStmt->body->kind != StmtKind::BLOCK) return;
        auto* body = static_cast<BlockStmt*>(whileStmt->body.get());

        // 收集循环内被修改的变量
        set<string> modifiedVars;
        collectModifiedVars(whileStmt->body.get(), modifiedVars);

        // 遍历循环体，提取可外提的语句
        for (auto it = body->stmts.begin(); it != body->stmts.end(); ) {
            Stmt* stmt = it->get();

            if (stmt->kind == StmtKind::VARDECL) {
                auto* varDecl = static_cast<VarDeclStmt*>(stmt);
                // 如果初始化表达式是循环不变量，且变量在循环中不被再次赋值
                if (isLoopInvariant(varDecl->init.get(), modifiedVars)) {
                    // 检查这个变量是否在循环体其他地方被修改
                    set<string> otherMods;
                    for (auto& s : body->stmts) {
                        if (s.get() != stmt) {
                            collectModifiedVars(s.get(), otherMods);
                        }
                    }
                    if (otherMods.find(varDecl->name) == otherMods.end()) {
                        hoisted.push_back(move(*it));
                        it = body->stmts.erase(it);
                        continue;
                    }
                }
            }
            ++it;
        }
    }

    // 公共子表达式消除：表达式 -> 临时变量名
    map<string, string> cseMap;
    int cseTempCount = 0;

    // 对表达式进行 CSE，返回优化后的表达式
    unique_ptr<Expr> cseExpr(Expr* expr, vector<unique_ptr<Stmt>>& preStmts) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return make_unique<NumberExpr>(static_cast<NumberExpr*>(expr)->value);
        case ExprKind::IDENT:
            return make_unique<IdentExpr>(static_cast<IdentExpr*>(expr)->name);
        case ExprKind::UNARY: {
            auto* u = static_cast<UnaryExpr*>(expr);
            auto operand = cseExpr(u->operand.get(), preStmts);
            return make_unique<UnaryExpr>(u->op, move(operand));
        }
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            auto left = cseExpr(b->left.get(), preStmts);
            auto right = cseExpr(b->right.get(), preStmts);

            // 构建表达式签名
            auto newExpr = make_unique<BinaryExpr>(b->op, move(left), move(right));
            string sig = exprToString(newExpr.get());

            // 检查是否已经计算过（只对复杂表达式进行 CSE）
            if (sig.length() > 5 && !hasCallExpr(newExpr.get())) {
                if (cseMap.count(sig)) {
                    return make_unique<IdentExpr>(cseMap[sig]);
                }
                // 创建临时变量
                string tmpName = "__cse_" + to_string(cseTempCount++);
                cseMap[sig] = tmpName;
                preStmts.push_back(make_unique<VarDeclStmt>(tmpName, move(newExpr)));
                return make_unique<IdentExpr>(tmpName);
            }
            return newExpr;
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            auto newCall = make_unique<CallExpr>(c->funcName);
            for (auto& arg : c->args) {
                newCall->args.push_back(cseExpr(arg.get(), preStmts));
            }
            return newCall;
        }
        }
        return nullptr;
    }

    // 对语句列表进行 CSE
    void cseStmtList(vector<unique_ptr<Stmt>>& stmts) {
        vector<unique_ptr<Stmt>> newStmts;

        for (auto& stmt : stmts) {
            vector<unique_ptr<Stmt>> preStmts;

            switch (stmt->kind) {
            case StmtKind::VARDECL: {
                auto* v = static_cast<VarDeclStmt*>(stmt.get());
                v->init = cseExpr(v->init.get(), preStmts);
                // 变量赋值后，包含该变量的 CSE 项失效
                for (auto it = cseMap.begin(); it != cseMap.end(); ) {
                    if (it->first.find(v->name) != string::npos) {
                        it = cseMap.erase(it);
                    } else {
                        ++it;
                    }
                }
                break;
            }
            case StmtKind::ASSIGN: {
                auto* a = static_cast<AssignStmt*>(stmt.get());
                a->value = cseExpr(a->value.get(), preStmts);
                // 变量赋值后，包含该变量的 CSE 项失效
                for (auto it = cseMap.begin(); it != cseMap.end(); ) {
                    if (it->first.find(a->name) != string::npos) {
                        it = cseMap.erase(it);
                    } else {
                        ++it;
                    }
                }
                break;
            }
            case StmtKind::RETURN: {
                auto* r = static_cast<ReturnStmt*>(stmt.get());
                if (r->value) r->value = cseExpr(r->value.get(), preStmts);
                break;
            }
            case StmtKind::IF: {
                auto* i = static_cast<IfStmt*>(stmt.get());
                i->cond = cseExpr(i->cond.get(), preStmts);
                // 分支内不进行CSE（保守策略，避免作用域问题）
                // 清理可能在分支内被修改的变量的 CSE 项
                set<string> modifiedVars;
                collectModifiedVars(i->thenStmt.get(), modifiedVars);
                if (i->elseStmt) collectModifiedVars(i->elseStmt.get(), modifiedVars);
                for (auto it = cseMap.begin(); it != cseMap.end(); ) {
                    bool shouldErase = false;
                    for (const auto& var : modifiedVars) {
                        if (it->first.find(var) != string::npos) {
                            shouldErase = true;
                            break;
                        }
                    }
                    if (shouldErase) {
                        it = cseMap.erase(it);
                    } else {
                        ++it;
                    }
                }
                break;
            }
            case StmtKind::WHILE: {
                auto* w = static_cast<WhileStmt*>(stmt.get());
                // 循环内不进行CSE（保守策略，避免作用域问题）
                cseMap.clear();
                break;
            }
            case StmtKind::EXPR: {
                auto* e = static_cast<ExprStmt*>(stmt.get());
                e->expr = cseExpr(e->expr.get(), preStmts);
                break;
            }
            default:
                break;
            }

            // 插入预生成的语句
            for (auto& pre : preStmts) {
                newStmts.push_back(move(pre));
            }
            newStmts.push_back(move(stmt));
        }

        stmts = move(newStmts);
    }

    // 死变量消除：收集所有被使用的变量
    void collectUsedVars(Expr* expr, set<string>& used) {
        switch (expr->kind) {
        case ExprKind::IDENT:
            used.insert(static_cast<IdentExpr*>(expr)->name);
            break;
        case ExprKind::UNARY:
            collectUsedVars(static_cast<UnaryExpr*>(expr)->operand.get(), used);
            break;
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            collectUsedVars(b->left.get(), used);
            collectUsedVars(b->right.get(), used);
            break;
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            for (auto& arg : c->args) collectUsedVars(arg.get(), used);
            break;
        }
        default:
            break;
        }
    }

    void collectUsedVarsInStmt(Stmt* stmt, set<string>& used) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* b = static_cast<BlockStmt*>(stmt);
            for (auto& s : b->stmts) collectUsedVarsInStmt(s.get(), used);
            break;
        }
        case StmtKind::VARDECL:
            collectUsedVars(static_cast<VarDeclStmt*>(stmt)->init.get(), used);
            break;
        case StmtKind::ASSIGN: {
            auto* a = static_cast<AssignStmt*>(stmt);
            collectUsedVars(a->value.get(), used);
            break;
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            collectUsedVars(i->cond.get(), used);
            collectUsedVarsInStmt(i->thenStmt.get(), used);
            if (i->elseStmt) collectUsedVarsInStmt(i->elseStmt.get(), used);
            break;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            collectUsedVars(w->cond.get(), used);
            collectUsedVarsInStmt(w->body.get(), used);
            break;
        }
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) collectUsedVars(r->value.get(), used);
            break;
        }
        case StmtKind::EXPR:
            collectUsedVars(static_cast<ExprStmt*>(stmt)->expr.get(), used);
            break;
        default:
            break;
        }
    }

    // 死变量消除
    bool eliminateDeadVars(vector<unique_ptr<Stmt>>& stmts) {
        bool changed = false;

        // 收集所有使用的变量（在表达式右边被引用的变量）
        set<string> usedVars;
        for (auto& stmt : stmts) {
            collectUsedVarsInStmt(stmt.get(), usedVars);
        }

        // 删除未使用变量的声明和赋值
        for (auto it = stmts.begin(); it != stmts.end(); ) {
            if ((*it)->kind == StmtKind::VARDECL) {
                auto* v = static_cast<VarDeclStmt*>(it->get());
                if (usedVars.find(v->name) == usedVars.end() &&
                    !hasCallExpr(v->init.get())) {
                    it = stmts.erase(it);
                    changed = true;
                    continue;
                }
            } else if ((*it)->kind == StmtKind::ASSIGN) {
                // 如果赋值目标变量从未被使用（读取），删除该赋值
                auto* a = static_cast<AssignStmt*>(it->get());
                if (usedVars.find(a->name) == usedVars.end() &&
                    !hasCallExpr(a->value.get())) {
                    it = stmts.erase(it);
                    changed = true;
                    continue;
                }
            }
            ++it;
        }

        // 递归处理嵌套块
        for (auto& stmt : stmts) {
            if (stmt->kind == StmtKind::BLOCK) {
                if (eliminateDeadVars(static_cast<BlockStmt*>(stmt.get())->stmts))
                    changed = true;
            } else if (stmt->kind == StmtKind::IF) {
                auto* i = static_cast<IfStmt*>(stmt.get());
                if (i->thenStmt->kind == StmtKind::BLOCK) {
                    if (eliminateDeadVars(static_cast<BlockStmt*>(i->thenStmt.get())->stmts))
                        changed = true;
                }
                if (i->elseStmt && i->elseStmt->kind == StmtKind::BLOCK) {
                    if (eliminateDeadVars(static_cast<BlockStmt*>(i->elseStmt.get())->stmts))
                        changed = true;
                }
            } else if (stmt->kind == StmtKind::WHILE) {
                auto* w = static_cast<WhileStmt*>(stmt.get());
                if (w->body->kind == StmtKind::BLOCK) {
                    if (eliminateDeadVars(static_cast<BlockStmt*>(w->body.get())->stmts))
                        changed = true;
                }
            }
        }

        return changed;
    }

    // 归纳变量强度削减：将 i * c 转换为增量加法
    struct InductionVar {
        string varName;      // 归纳变量名
        int step;            // 步长
    };

    // 检测归纳变量：形如 i = i + c 的变量
    void detectInductionVars(Stmt* stmt, map<string, int>& inductionVars) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) detectInductionVars(s.get(), inductionVars);
            break;
        }
        case StmtKind::ASSIGN: {
            auto* assign = static_cast<AssignStmt*>(stmt);
            // 检查是否是 i = i + c 或 i = i - c 的形式
            if (assign->value->kind == ExprKind::BINARY) {
                auto* binary = static_cast<BinaryExpr*>(assign->value.get());
                if ((binary->op == "+" || binary->op == "-") &&
                    binary->left->kind == ExprKind::IDENT &&
                    binary->right->kind == ExprKind::NUMBER) {
                    auto* ident = static_cast<IdentExpr*>(binary->left.get());
                    if (ident->name == assign->name) {
                        int step = static_cast<NumberExpr*>(binary->right.get())->value;
                        if (binary->op == "-") step = -step;
                        inductionVars[assign->name] = step;
                    }
                }
            }
            break;
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            detectInductionVars(i->thenStmt.get(), inductionVars);
            if (i->elseStmt) detectInductionVars(i->elseStmt.get(), inductionVars);
            break;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            detectInductionVars(w->body.get(), inductionVars);
            break;
        }
        default:
            break;
        }
    }

    // 检查表达式是否是 inductionVar * constant 的形式
    bool isInductionMul(Expr* expr, const map<string, int>& inductionVars,
                        string& varName, int& multiplier) {
        if (expr->kind != ExprKind::BINARY) return false;
        auto* binary = static_cast<BinaryExpr*>(expr);
        if (binary->op != "*") return false;

        // 检查 var * const 形式
        if (binary->left->kind == ExprKind::IDENT &&
            binary->right->kind == ExprKind::NUMBER) {
            auto* ident = static_cast<IdentExpr*>(binary->left.get());
            if (inductionVars.count(ident->name)) {
                varName = ident->name;
                multiplier = static_cast<NumberExpr*>(binary->right.get())->value;
                return true;
            }
        }
        // 检查 const * var 形式
        if (binary->left->kind == ExprKind::NUMBER &&
            binary->right->kind == ExprKind::IDENT) {
            auto* ident = static_cast<IdentExpr*>(binary->right.get());
            if (inductionVars.count(ident->name)) {
                varName = ident->name;
                multiplier = static_cast<NumberExpr*>(binary->left.get())->value;
                return true;
            }
        }
        return false;
    }

    // 在表达式中进行强度削减替换
    int strengthReductionCount = 0;
    map<string, string> strengthReductionMap;  // "i*4" -> "__sr_0"

    unique_ptr<Expr> applyStrengthReduction(Expr* expr, const map<string, int>& inductionVars,
                                            vector<unique_ptr<Stmt>>& preLoop,
                                            vector<unique_ptr<Stmt>>& inLoop,
                                            const map<string, int>& initVals) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return make_unique<NumberExpr>(static_cast<NumberExpr*>(expr)->value);
        case ExprKind::IDENT:
            return make_unique<IdentExpr>(static_cast<IdentExpr*>(expr)->name);
        case ExprKind::UNARY: {
            auto* u = static_cast<UnaryExpr*>(expr);
            return make_unique<UnaryExpr>(u->op,
                applyStrengthReduction(u->operand.get(), inductionVars, preLoop, inLoop, initVals));
        }
        case ExprKind::BINARY: {
            auto* binary = static_cast<BinaryExpr*>(expr);
            string varName;
            int multiplier;

            // 检查是否可以进行强度削减
            if (isInductionMul(expr, inductionVars, varName, multiplier)) {
                string key = varName + "*" + to_string(multiplier);

                if (!strengthReductionMap.count(key)) {
                    // 创建新的辅助变量
                    string newVar = "__sr_" + to_string(strengthReductionCount++);
                    strengthReductionMap[key] = newVar;

                    // 计算初始值：init_i * multiplier
                    int initVal = initVals.count(varName) ? initVals.at(varName) : 0;
                    preLoop.push_back(make_unique<VarDeclStmt>(newVar,
                        make_unique<NumberExpr>(initVal * multiplier)));

                    // 添加循环内的增量：newVar = newVar + step * multiplier
                    int step = inductionVars.at(varName);
                    int increment = step * multiplier;
                    inLoop.push_back(make_unique<AssignStmt>(newVar,
                        make_unique<BinaryExpr>("+",
                            make_unique<IdentExpr>(newVar),
                            make_unique<NumberExpr>(increment))));
                }

                return make_unique<IdentExpr>(strengthReductionMap[key]);
            }

            // 递归处理子表达式
            return make_unique<BinaryExpr>(binary->op,
                applyStrengthReduction(binary->left.get(), inductionVars, preLoop, inLoop, initVals),
                applyStrengthReduction(binary->right.get(), inductionVars, preLoop, inLoop, initVals));
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            auto newCall = make_unique<CallExpr>(c->funcName);
            for (auto& arg : c->args) {
                newCall->args.push_back(
                    applyStrengthReduction(arg.get(), inductionVars, preLoop, inLoop, initVals));
            }
            return newCall;
        }
        }
        return nullptr;
    }

    // 对语句应用强度削减
    void applyStrengthReductionToStmt(Stmt* stmt, const map<string, int>& inductionVars,
                                      vector<unique_ptr<Stmt>>& preLoop,
                                      vector<unique_ptr<Stmt>>& inLoop,
                                      const map<string, int>& initVals) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) {
                applyStrengthReductionToStmt(s.get(), inductionVars, preLoop, inLoop, initVals);
            }
            break;
        }
        case StmtKind::VARDECL: {
            auto* v = static_cast<VarDeclStmt*>(stmt);
            v->init = applyStrengthReduction(v->init.get(), inductionVars, preLoop, inLoop, initVals);
            break;
        }
        case StmtKind::ASSIGN: {
            auto* a = static_cast<AssignStmt*>(stmt);
            // 不处理归纳变量自身的赋值
            if (!inductionVars.count(a->name)) {
                a->value = applyStrengthReduction(a->value.get(), inductionVars, preLoop, inLoop, initVals);
            }
            break;
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            i->cond = applyStrengthReduction(i->cond.get(), inductionVars, preLoop, inLoop, initVals);
            applyStrengthReductionToStmt(i->thenStmt.get(), inductionVars, preLoop, inLoop, initVals);
            if (i->elseStmt) {
                applyStrengthReductionToStmt(i->elseStmt.get(), inductionVars, preLoop, inLoop, initVals);
            }
            break;
        }
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) {
                r->value = applyStrengthReduction(r->value.get(), inductionVars, preLoop, inLoop, initVals);
            }
            break;
        }
        case StmtKind::EXPR: {
            auto* e = static_cast<ExprStmt*>(stmt);
            e->expr = applyStrengthReduction(e->expr.get(), inductionVars, preLoop, inLoop, initVals);
            break;
        }
        default:
            break;
        }
    }

    // 对循环进行归纳变量强度削减
    void optimizeLoopStrengthReduction(vector<unique_ptr<Stmt>>& stmts) {
        for (size_t i = 0; i < stmts.size(); i++) {
            if (stmts[i]->kind == StmtKind::WHILE) {
                auto* whileStmt = static_cast<WhileStmt*>(stmts[i].get());

                // 检测归纳变量
                map<string, int> inductionVars;
                detectInductionVars(whileStmt->body.get(), inductionVars);

                if (inductionVars.empty()) continue;

                // 收集归纳变量的初始值（从循环前的语句中）
                map<string, int> initVals;
                for (size_t j = 0; j < i; j++) {
                    if (stmts[j]->kind == StmtKind::VARDECL) {
                        auto* v = static_cast<VarDeclStmt*>(stmts[j].get());
                        if (inductionVars.count(v->name) && v->init->kind == ExprKind::NUMBER) {
                            initVals[v->name] = static_cast<NumberExpr*>(v->init.get())->value;
                        }
                    } else if (stmts[j]->kind == StmtKind::ASSIGN) {
                        auto* a = static_cast<AssignStmt*>(stmts[j].get());
                        if (inductionVars.count(a->name) && a->value->kind == ExprKind::NUMBER) {
                            initVals[a->name] = static_cast<NumberExpr*>(a->value.get())->value;
                        }
                    }
                }

                // 重置强度削减状态
                strengthReductionMap.clear();

                // 存储需要添加的语句
                vector<unique_ptr<Stmt>> preLoop;
                vector<unique_ptr<Stmt>> inLoopEnd;

                // 应用强度削减
                if (whileStmt->body->kind == StmtKind::BLOCK) {
                    auto* body = static_cast<BlockStmt*>(whileStmt->body.get());
                    for (auto& s : body->stmts) {
                        applyStrengthReductionToStmt(s.get(), inductionVars, preLoop, inLoopEnd, initVals);
                    }

                    // 将增量语句添加到循环体末尾（在归纳变量更新之前）
                    for (auto& s : inLoopEnd) {
                        // 找到归纳变量更新的位置，在其前面插入
                        bool inserted = false;
                        for (size_t k = body->stmts.size(); k > 0; k--) {
                            if (body->stmts[k-1]->kind == StmtKind::ASSIGN) {
                                auto* assign = static_cast<AssignStmt*>(body->stmts[k-1].get());
                                if (inductionVars.count(assign->name)) {
                                    body->stmts.insert(body->stmts.begin() + k - 1, move(s));
                                    inserted = true;
                                    break;
                                }
                            }
                        }
                        if (!inserted) {
                            body->stmts.push_back(move(s));
                        }
                    }
                }

                // 在循环前插入初始化语句
                for (auto it = preLoop.rbegin(); it != preLoop.rend(); ++it) {
                    stmts.insert(stmts.begin() + i, move(*it));
                    i++;
                }
            }

            // 递归处理嵌套结构
            if (stmts[i]->kind == StmtKind::IF) {
                auto* ifStmt = static_cast<IfStmt*>(stmts[i].get());
                if (ifStmt->thenStmt->kind == StmtKind::BLOCK) {
                    optimizeLoopStrengthReduction(static_cast<BlockStmt*>(ifStmt->thenStmt.get())->stmts);
                }
                if (ifStmt->elseStmt && ifStmt->elseStmt->kind == StmtKind::BLOCK) {
                    optimizeLoopStrengthReduction(static_cast<BlockStmt*>(ifStmt->elseStmt.get())->stmts);
                }
            } else if (stmts[i]->kind == StmtKind::BLOCK) {
                optimizeLoopStrengthReduction(static_cast<BlockStmt*>(stmts[i].get())->stmts);
            }
        }
    }

    // 计算循环体语句数
    int countStmtsInBody(Stmt* stmt) {
        if (stmt->kind != StmtKind::BLOCK) return 1;
        auto* block = static_cast<BlockStmt*>(stmt);
        int count = 0;
        for (auto& s : block->stmts) {
            if (s->kind == StmtKind::BLOCK) {
                count += countStmtsInBody(s.get());
            } else {
                count++;
            }
        }
        return count;
    }

    // 克隆语句
    unique_ptr<Stmt> cloneStmt(Stmt* stmt) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            auto newBlock = make_unique<BlockStmt>();
            for (auto& s : block->stmts) {
                newBlock->stmts.push_back(cloneStmt(s.get()));
            }
            return newBlock;
        }
        case StmtKind::VARDECL: {
            auto* v = static_cast<VarDeclStmt*>(stmt);
            return make_unique<VarDeclStmt>(v->name, cloneExpr(v->init.get()));
        }
        case StmtKind::ASSIGN: {
            auto* a = static_cast<AssignStmt*>(stmt);
            return make_unique<AssignStmt>(a->name, cloneExpr(a->value.get()));
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            auto newIf = make_unique<IfStmt>();
            newIf->cond = cloneExpr(i->cond.get());
            newIf->thenStmt = cloneStmt(i->thenStmt.get());
            if (i->elseStmt) newIf->elseStmt = cloneStmt(i->elseStmt.get());
            return newIf;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            auto newWhile = make_unique<WhileStmt>();
            newWhile->cond = cloneExpr(w->cond.get());
            newWhile->body = cloneStmt(w->body.get());
            return newWhile;
        }
        case StmtKind::BREAK:
            return make_unique<BreakStmt>();
        case StmtKind::CONTINUE:
            return make_unique<ContinueStmt>();
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) return make_unique<ReturnStmt>(cloneExpr(r->value.get()));
            return make_unique<ReturnStmt>();
        }
        case StmtKind::EXPR: {
            auto* e = static_cast<ExprStmt*>(stmt);
            return make_unique<ExprStmt>(cloneExpr(e->expr.get()));
        }
        case StmtKind::EMPTY:
            return make_unique<EmptyStmt>();
        }
        return nullptr;
    }

    // ==================== 激进循环完全展开 ====================
    // 最大完全展开迭代次数
    static const int MAX_FULL_UNROLL_ITERS = 64;
    // 最大部分展开因子
    static const int PARTIAL_UNROLL_FACTOR = 8;
    // 最大展开后语句数
    static const int MAX_UNROLLED_STMTS = 512;

    // 替换表达式中的变量为常量值
    unique_ptr<Expr> substituteVarInExpr(Expr* expr, const string& varName, int value) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return make_unique<NumberExpr>(static_cast<NumberExpr*>(expr)->value);
        case ExprKind::IDENT: {
            auto* ident = static_cast<IdentExpr*>(expr);
            if (ident->name == varName) {
                return make_unique<NumberExpr>(value);
            }
            return make_unique<IdentExpr>(ident->name);
        }
        case ExprKind::UNARY: {
            auto* u = static_cast<UnaryExpr*>(expr);
            return make_unique<UnaryExpr>(u->op, substituteVarInExpr(u->operand.get(), varName, value));
        }
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return make_unique<BinaryExpr>(b->op,
                substituteVarInExpr(b->left.get(), varName, value),
                substituteVarInExpr(b->right.get(), varName, value));
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            auto newCall = make_unique<CallExpr>(c->funcName);
            for (auto& arg : c->args) {
                newCall->args.push_back(substituteVarInExpr(arg.get(), varName, value));
            }
            return newCall;
        }
        }
        return nullptr;
    }

    // 替换语句中的变量为常量值（用于完全展开）
    unique_ptr<Stmt> substituteVarInStmt(Stmt* stmt, const string& varName, int value, bool skipInductionUpdate) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            auto newBlock = make_unique<BlockStmt>();
            for (auto& s : block->stmts) {
                auto newS = substituteVarInStmt(s.get(), varName, value, skipInductionUpdate);
                if (newS->kind != StmtKind::EMPTY) {
                    newBlock->stmts.push_back(move(newS));
                }
            }
            return newBlock;
        }
        case StmtKind::VARDECL: {
            auto* v = static_cast<VarDeclStmt*>(stmt);
            return make_unique<VarDeclStmt>(v->name, substituteVarInExpr(v->init.get(), varName, value));
        }
        case StmtKind::ASSIGN: {
            auto* a = static_cast<AssignStmt*>(stmt);
            // 如果是归纳变量的更新，跳过
            if (skipInductionUpdate && a->name == varName) {
                return make_unique<EmptyStmt>();
            }
            return make_unique<AssignStmt>(a->name, substituteVarInExpr(a->value.get(), varName, value));
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            auto newIf = make_unique<IfStmt>();
            newIf->cond = substituteVarInExpr(i->cond.get(), varName, value);
            newIf->thenStmt = substituteVarInStmt(i->thenStmt.get(), varName, value, skipInductionUpdate);
            if (i->elseStmt) {
                newIf->elseStmt = substituteVarInStmt(i->elseStmt.get(), varName, value, skipInductionUpdate);
            }
            return newIf;
        }
        case StmtKind::WHILE: {
            // 内层循环不替换
            return cloneStmt(stmt);
        }
        case StmtKind::BREAK:
            return make_unique<BreakStmt>();
        case StmtKind::CONTINUE:
            return make_unique<ContinueStmt>();
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) {
                return make_unique<ReturnStmt>(substituteVarInExpr(r->value.get(), varName, value));
            }
            return make_unique<ReturnStmt>();
        }
        case StmtKind::EXPR: {
            auto* e = static_cast<ExprStmt*>(stmt);
            return make_unique<ExprStmt>(substituteVarInExpr(e->expr.get(), varName, value));
        }
        case StmtKind::EMPTY:
            return make_unique<EmptyStmt>();
        }
        return nullptr;
    }

    // 检查循环是否包含break/continue（用于判断是否可以安全展开）
    bool loopHasBreakContinue(Stmt* stmt) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) {
                if (loopHasBreakContinue(s.get())) return true;
            }
            return false;
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            if (loopHasBreakContinue(i->thenStmt.get())) return true;
            if (i->elseStmt && loopHasBreakContinue(i->elseStmt.get())) return true;
            return false;
        }
        case StmtKind::WHILE:
            return false; // 内层循环的break/continue不影响外层
        case StmtKind::BREAK:
        case StmtKind::CONTINUE:
            return true;
        default:
            return false;
        }
    }

    // 分析循环：提取归纳变量、边界、步长
    struct LoopInfo {
        bool valid = false;
        string inductionVar;
        int initVal = 0;
        int boundVal = 0;
        int step = 0;
        bool isLessThan = true;  // < vs <=
        size_t initStmtIdx = SIZE_MAX;  // 初始化语句在stmts中的索引，SIZE_MAX表示未找到
    };

    LoopInfo analyzeLoop(vector<unique_ptr<Stmt>>& stmts, size_t whileIdx) {
        LoopInfo info;
        if (whileIdx == 0) return info;

        auto* whileStmt = static_cast<WhileStmt*>(stmts[whileIdx].get());

        // 检查条件是否是 i < N 或 i <= N 形式
        if (whileStmt->cond->kind != ExprKind::BINARY) return info;
        auto* cond = static_cast<BinaryExpr*>(whileStmt->cond.get());

        if (cond->op != "<" && cond->op != "<=" && cond->op != ">" && cond->op != ">=") return info;

        // 只处理 i < N 和 i <= N 形式
        if (cond->op == ">" || cond->op == ">=") return info;

        if (cond->left->kind != ExprKind::IDENT || cond->right->kind != ExprKind::NUMBER) return info;

        info.inductionVar = static_cast<IdentExpr*>(cond->left.get())->name;
        info.boundVal = static_cast<NumberExpr*>(cond->right.get())->value;
        info.isLessThan = (cond->op == "<");

        // 向前查找初始化语句 int i = N（必须紧邻while循环）
        if (stmts[whileIdx - 1]->kind == StmtKind::VARDECL) {
            auto* v = static_cast<VarDeclStmt*>(stmts[whileIdx - 1].get());
            if (v->name == info.inductionVar && v->init->kind == ExprKind::NUMBER) {
                info.initVal = static_cast<NumberExpr*>(v->init.get())->value;
                info.initStmtIdx = whileIdx - 1;
            }
        }

        // 如果没有找到紧邻的初始化语句，返回无效
        if (info.initStmtIdx == SIZE_MAX) return info;

        // 检查循环体中是否有 i = i + step 形式的更新
        if (whileStmt->body->kind != StmtKind::BLOCK) return info;
        auto* body = static_cast<BlockStmt*>(whileStmt->body.get());

        for (auto& s : body->stmts) {
            if (s->kind == StmtKind::ASSIGN) {
                auto* assign = static_cast<AssignStmt*>(s.get());
                if (assign->name == info.inductionVar && assign->value->kind == ExprKind::BINARY) {
                    auto* binary = static_cast<BinaryExpr*>(assign->value.get());
                    if (binary->left->kind == ExprKind::IDENT && binary->right->kind == ExprKind::NUMBER) {
                        if (static_cast<IdentExpr*>(binary->left.get())->name == info.inductionVar) {
                            int stepVal = static_cast<NumberExpr*>(binary->right.get())->value;
                            if (binary->op == "+") info.step = stepVal;
                            else if (binary->op == "-") info.step = -stepVal;
                        }
                    }
                }
            }
        }

        if (info.step == 0) return info;

        // 计算迭代次数
        int endVal = info.isLessThan ? info.boundVal : info.boundVal + 1;
        int iters = (endVal - info.initVal + info.step - 1) / info.step;
        if (iters < 0) iters = 0;

        // 检查是否包含break/continue
        if (loopHasBreakContinue(whileStmt->body.get())) return info;

        info.valid = (iters > 0 && iters <= MAX_FULL_UNROLL_ITERS);
        return info;
    }

    // 完全展开循环
    bool fullyUnrollLoop(vector<unique_ptr<Stmt>>& stmts, size_t whileIdx, const LoopInfo& info) {
        auto* whileStmt = static_cast<WhileStmt*>(stmts[whileIdx].get());
        auto* body = static_cast<BlockStmt*>(whileStmt->body.get());

        int endVal = info.isLessThan ? info.boundVal : info.boundVal + 1;
        int iters = (endVal - info.initVal) / info.step;

        // 检查展开后语句数是否过多
        int bodySize = countStmtsInBody(whileStmt->body.get());
        if (bodySize * iters > MAX_UNROLLED_STMTS) return false;

        vector<unique_ptr<Stmt>> unrolled;

        // 保留归纳变量的声明，初始化为最终值（循环结束后的值）
        int finalVal = info.initVal + iters * info.step;
        unrolled.push_back(make_unique<VarDeclStmt>(info.inductionVar, make_unique<NumberExpr>(finalVal)));

        for (int iter = 0; iter < iters; iter++) {
            int currentVal = info.initVal + iter * info.step;

            for (auto& s : body->stmts) {
                auto newStmt = substituteVarInStmt(s.get(), info.inductionVar, currentVal, true);
                if (newStmt->kind != StmtKind::EMPTY) {
                    unrolled.push_back(move(newStmt));
                }
            }
        }

        // 删除初始化语句和while循环
        stmts.erase(stmts.begin() + info.initStmtIdx, stmts.begin() + whileIdx + 1);

        // 插入展开后的语句
        for (size_t i = 0; i < unrolled.size(); i++) {
            stmts.insert(stmts.begin() + info.initStmtIdx + i, move(unrolled[i]));
        }

        return true;
    }

    // 部分展开循环（用于迭代次数过多的情况）
    bool partiallyUnrollLoop(vector<unique_ptr<Stmt>>& stmts, size_t whileIdx, int unrollFactor) {
        auto* whileStmt = static_cast<WhileStmt*>(stmts[whileIdx].get());
        if (whileStmt->body->kind != StmtKind::BLOCK) return false;

        auto* body = static_cast<BlockStmt*>(whileStmt->body.get());

        // 创建展开后的循环体
        auto newBody = make_unique<BlockStmt>();

        for (int u = 0; u < unrollFactor; u++) {
            for (auto& s : body->stmts) {
                newBody->stmts.push_back(cloneStmt(s.get()));
            }
        }

        whileStmt->body = move(newBody);
        return true;
    }

    // 激进循环展开主函数
    bool aggressiveLoopUnroll(vector<unique_ptr<Stmt>>& stmts) {
        bool changed = false;

        for (size_t i = 0; i < stmts.size(); i++) {
            // 递归处理嵌套结构
            if (stmts[i]->kind == StmtKind::BLOCK) {
                if (aggressiveLoopUnroll(static_cast<BlockStmt*>(stmts[i].get())->stmts)) {
                    changed = true;
                }
            } else if (stmts[i]->kind == StmtKind::IF) {
                auto* ifStmt = static_cast<IfStmt*>(stmts[i].get());
                if (ifStmt->thenStmt->kind == StmtKind::BLOCK) {
                    if (aggressiveLoopUnroll(static_cast<BlockStmt*>(ifStmt->thenStmt.get())->stmts)) {
                        changed = true;
                    }
                }
                if (ifStmt->elseStmt && ifStmt->elseStmt->kind == StmtKind::BLOCK) {
                    if (aggressiveLoopUnroll(static_cast<BlockStmt*>(ifStmt->elseStmt.get())->stmts)) {
                        changed = true;
                    }
                }
            } else if (stmts[i]->kind == StmtKind::WHILE) {
                // 先递归处理内层循环
                auto* whileStmt = static_cast<WhileStmt*>(stmts[i].get());
                if (whileStmt->body->kind == StmtKind::BLOCK) {
                    if (aggressiveLoopUnroll(static_cast<BlockStmt*>(whileStmt->body.get())->stmts)) {
                        changed = true;
                    }
                }

                // 分析并展开当前循环
                LoopInfo info = analyzeLoop(stmts, i);
                if (info.valid) {
                    if (fullyUnrollLoop(stmts, i, info)) {
                        changed = true;
                        i = info.initStmtIdx; // 重新从展开位置开始
                        continue;
                    }
                }
            }
        }

        return changed;
    }

    // 统计函数调用次数
    void countFuncCalls(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::UNARY:
            countFuncCalls(static_cast<UnaryExpr*>(expr)->operand.get());
            break;
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            countFuncCalls(b->left.get());
            countFuncCalls(b->right.get());
            break;
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            funcCallCount[c->funcName]++;
            for (auto& arg : c->args) {
                countFuncCalls(arg.get());
            }
            break;
        }
        default:
            break;
        }
    }

    void countFuncCallsInStmt(Stmt* stmt) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) countFuncCallsInStmt(s.get());
            break;
        }
        case StmtKind::VARDECL:
            countFuncCalls(static_cast<VarDeclStmt*>(stmt)->init.get());
            break;
        case StmtKind::ASSIGN:
            countFuncCalls(static_cast<AssignStmt*>(stmt)->value.get());
            break;
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            countFuncCalls(i->cond.get());
            countFuncCallsInStmt(i->thenStmt.get());
            if (i->elseStmt) countFuncCallsInStmt(i->elseStmt.get());
            break;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            countFuncCalls(w->cond.get());
            countFuncCallsInStmt(w->body.get());
            break;
        }
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) countFuncCalls(r->value.get());
            break;
        }
        case StmtKind::EXPR:
            countFuncCalls(static_cast<ExprStmt*>(stmt)->expr.get());
            break;
        default:
            break;
        }
    }

    // 检查函数是否递归
    bool isRecursive(FuncDef* func) {
        return checkRecursive(func->body.get(), func->name);
    }

    bool checkRecursive(Stmt* stmt, const string& funcName) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) {
                if (checkRecursive(s.get(), funcName)) return true;
            }
            return false;
        }
        case StmtKind::VARDECL:
            return checkRecursiveExpr(static_cast<VarDeclStmt*>(stmt)->init.get(), funcName);
        case StmtKind::ASSIGN:
            return checkRecursiveExpr(static_cast<AssignStmt*>(stmt)->value.get(), funcName);
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            if (checkRecursiveExpr(i->cond.get(), funcName)) return true;
            if (checkRecursive(i->thenStmt.get(), funcName)) return true;
            if (i->elseStmt && checkRecursive(i->elseStmt.get(), funcName)) return true;
            return false;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            if (checkRecursiveExpr(w->cond.get(), funcName)) return true;
            return checkRecursive(w->body.get(), funcName);
        }
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) return checkRecursiveExpr(r->value.get(), funcName);
            return false;
        }
        case StmtKind::EXPR:
            return checkRecursiveExpr(static_cast<ExprStmt*>(stmt)->expr.get(), funcName);
        default:
            return false;
        }
    }

    bool checkRecursiveExpr(Expr* expr, const string& funcName) {
        switch (expr->kind) {
        case ExprKind::UNARY:
            return checkRecursiveExpr(static_cast<UnaryExpr*>(expr)->operand.get(), funcName);
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return checkRecursiveExpr(b->left.get(), funcName) ||
                   checkRecursiveExpr(b->right.get(), funcName);
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            if (c->funcName == funcName) return true;
            for (auto& arg : c->args) {
                if (checkRecursiveExpr(arg.get(), funcName)) return true;
            }
            return false;
        }
        default:
            return false;
        }
    }

    // 计算函数体语句数
    int countFuncStmts(FuncDef* func) {
        return countStmtsInBody(func->body.get());
    }

    // 判断函数是否可内联 - 暂时禁用
    // ==================== 激进函数内联 ====================
    // 内联阈值配置
    static const int MAX_INLINE_STMTS = 30;      // 最大内联函数语句数
    static const int MAX_INLINE_DEPTH = 5;       // 最大内联深度
    static const int ALWAYS_INLINE_STMTS = 10;   // 总是内联的小函数语句数

    // 检查函数是否包含递归调用
    bool hasRecursiveCall(Stmt* stmt, const string& funcName) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* b = static_cast<BlockStmt*>(stmt);
            for (auto& s : b->stmts) {
                if (hasRecursiveCall(s.get(), funcName)) return true;
            }
            return false;
        }
        case StmtKind::VARDECL:
            return hasRecursiveCallInExpr(static_cast<VarDeclStmt*>(stmt)->init.get(), funcName);
        case StmtKind::ASSIGN:
            return hasRecursiveCallInExpr(static_cast<AssignStmt*>(stmt)->value.get(), funcName);
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            if (hasRecursiveCallInExpr(i->cond.get(), funcName)) return true;
            if (hasRecursiveCall(i->thenStmt.get(), funcName)) return true;
            if (i->elseStmt && hasRecursiveCall(i->elseStmt.get(), funcName)) return true;
            return false;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            if (hasRecursiveCallInExpr(w->cond.get(), funcName)) return true;
            return hasRecursiveCall(w->body.get(), funcName);
        }
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            return r->value && hasRecursiveCallInExpr(r->value.get(), funcName);
        }
        case StmtKind::EXPR:
            return hasRecursiveCallInExpr(static_cast<ExprStmt*>(stmt)->expr.get(), funcName);
        default:
            return false;
        }
    }

    bool hasRecursiveCallInExpr(Expr* expr, const string& funcName) {
        switch (expr->kind) {
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            if (c->funcName == funcName) return true;
            for (auto& arg : c->args) {
                if (hasRecursiveCallInExpr(arg.get(), funcName)) return true;
            }
            return false;
        }
        case ExprKind::UNARY:
            return hasRecursiveCallInExpr(static_cast<UnaryExpr*>(expr)->operand.get(), funcName);
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return hasRecursiveCallInExpr(b->left.get(), funcName) ||
                   hasRecursiveCallInExpr(b->right.get(), funcName);
        }
        default:
            return false;
        }
    }

    // 判断函数是否可以内联
    bool canInline(FuncDef* func) {
        if (!func) return false;

        // 不内联main函数
        if (func->name == "main") return false;

        // 检查是否递归
        if (hasRecursiveCall(func->body.get(), func->name)) return false;

        // 检查语句数
        int stmtCount = countStmtsInBody(func->body.get());
        if (stmtCount > MAX_INLINE_STMTS) return false;

        return true;
    }

    // 在表达式中进行变量重命名
    unique_ptr<Expr> renameVarsInExpr(Expr* expr, const map<string, string>& renameMap) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
            return make_unique<NumberExpr>(static_cast<NumberExpr*>(expr)->value);
        case ExprKind::IDENT: {
            auto* ident = static_cast<IdentExpr*>(expr);
            if (renameMap.count(ident->name)) {
                return make_unique<IdentExpr>(renameMap.at(ident->name));
            }
            return make_unique<IdentExpr>(ident->name);
        }
        case ExprKind::UNARY: {
            auto* u = static_cast<UnaryExpr*>(expr);
            return make_unique<UnaryExpr>(u->op, renameVarsInExpr(u->operand.get(), renameMap));
        }
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return make_unique<BinaryExpr>(b->op,
                renameVarsInExpr(b->left.get(), renameMap),
                renameVarsInExpr(b->right.get(), renameMap));
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            auto newCall = make_unique<CallExpr>(c->funcName);
            for (auto& arg : c->args) {
                newCall->args.push_back(renameVarsInExpr(arg.get(), renameMap));
            }
            return newCall;
        }
        }
        return nullptr;
    }

    // 收集语句中定义的变量
    void collectDefinedVars(Stmt* stmt, set<string>& vars) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) collectDefinedVars(s.get(), vars);
            break;
        }
        case StmtKind::VARDECL:
            vars.insert(static_cast<VarDeclStmt*>(stmt)->name);
            break;
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            collectDefinedVars(i->thenStmt.get(), vars);
            if (i->elseStmt) collectDefinedVars(i->elseStmt.get(), vars);
            break;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            collectDefinedVars(w->body.get(), vars);
            break;
        }
        default:
            break;
        }
    }

    // 在语句中进行变量重命名
    unique_ptr<Stmt> renameVarsInStmt(Stmt* stmt, map<string, string>& renameMap, const string& prefix) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            auto newBlock = make_unique<BlockStmt>();
            for (auto& s : block->stmts) {
                newBlock->stmts.push_back(renameVarsInStmt(s.get(), renameMap, prefix));
            }
            return newBlock;
        }
        case StmtKind::VARDECL: {
            auto* v = static_cast<VarDeclStmt*>(stmt);
            string newName = prefix + v->name;
            renameMap[v->name] = newName;
            return make_unique<VarDeclStmt>(newName, renameVarsInExpr(v->init.get(), renameMap));
        }
        case StmtKind::ASSIGN: {
            auto* a = static_cast<AssignStmt*>(stmt);
            string name = renameMap.count(a->name) ? renameMap[a->name] : a->name;
            return make_unique<AssignStmt>(name, renameVarsInExpr(a->value.get(), renameMap));
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            auto newIf = make_unique<IfStmt>();
            newIf->cond = renameVarsInExpr(i->cond.get(), renameMap);
            newIf->thenStmt = renameVarsInStmt(i->thenStmt.get(), renameMap, prefix);
            if (i->elseStmt) newIf->elseStmt = renameVarsInStmt(i->elseStmt.get(), renameMap, prefix);
            return newIf;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            auto newWhile = make_unique<WhileStmt>();
            newWhile->cond = renameVarsInExpr(w->cond.get(), renameMap);
            newWhile->body = renameVarsInStmt(w->body.get(), renameMap, prefix);
            return newWhile;
        }
        case StmtKind::BREAK:
            return make_unique<BreakStmt>();
        case StmtKind::CONTINUE:
            return make_unique<ContinueStmt>();
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) return make_unique<ReturnStmt>(renameVarsInExpr(r->value.get(), renameMap));
            return make_unique<ReturnStmt>();
        }
        case StmtKind::EXPR: {
            auto* e = static_cast<ExprStmt*>(stmt);
            return make_unique<ExprStmt>(renameVarsInExpr(e->expr.get(), renameMap));
        }
        case StmtKind::EMPTY:
            return make_unique<EmptyStmt>();
        }
        return nullptr;
    }

    // 内联单个调用
    // 返回：展开后的语句列表 + 结果变量名
    pair<vector<unique_ptr<Stmt>>, string> inlineCall(CallExpr* call, FuncDef* func) {
        vector<unique_ptr<Stmt>> stmts;
        string prefix = "__inline_" + to_string(inlineCount++) + "_";
        string resultVar = prefix + "result";

        // 创建重命名映射
        map<string, string> renameMap;

        // 为参数创建临时变量
        for (size_t i = 0; i < func->params.size() && i < call->args.size(); i++) {
            string paramName = func->params[i]->name;
            string newName = prefix + paramName;
            renameMap[paramName] = newName;
            stmts.push_back(make_unique<VarDeclStmt>(newName, cloneExpr(call->args[i].get())));
        }

        // 创建结果变量
        if (!func->isVoid) {
            stmts.push_back(make_unique<VarDeclStmt>(resultVar, make_unique<NumberExpr>(0)));
        }

        // 复制函数体（处理 return）
        for (auto& s : func->body->stmts) {
            if (s->kind == StmtKind::RETURN) {
                auto* ret = static_cast<ReturnStmt*>(s.get());
                if (ret->value && !func->isVoid) {
                    // 将 return expr 转换为 result = expr
                    stmts.push_back(make_unique<AssignStmt>(resultVar,
                        renameVarsInExpr(ret->value.get(), renameMap)));
                }
                // 不生成 return 语句
            } else {
                stmts.push_back(renameVarsInStmt(s.get(), renameMap, prefix));
            }
        }

        return {move(stmts), resultVar};
    }

    // 在表达式中内联函数调用
    // 返回：是否修改，展开后的语句列表，新表达式
    tuple<bool, vector<unique_ptr<Stmt>>, unique_ptr<Expr>> inlineExpr(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::NUMBER: {
            vector<unique_ptr<Stmt>> empty;
            return make_tuple(false, move(empty), make_unique<NumberExpr>(static_cast<NumberExpr*>(expr)->value));
        }
        case ExprKind::IDENT: {
            vector<unique_ptr<Stmt>> empty;
            return make_tuple(false, move(empty), make_unique<IdentExpr>(static_cast<IdentExpr*>(expr)->name));
        }
        case ExprKind::UNARY: {
            auto* u = static_cast<UnaryExpr*>(expr);
            auto [changed, stmts, newOperand] = inlineExpr(u->operand.get());
            return make_tuple(changed, move(stmts), make_unique<UnaryExpr>(u->op, move(newOperand)));
        }
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            auto [leftChanged, leftStmts, newLeft] = inlineExpr(b->left.get());
            auto [rightChanged, rightStmts, newRight] = inlineExpr(b->right.get());

            vector<unique_ptr<Stmt>> allStmts;
            for (auto& s : leftStmts) allStmts.push_back(move(s));
            for (auto& s : rightStmts) allStmts.push_back(move(s));

            return make_tuple(leftChanged || rightChanged, move(allStmts),
                    make_unique<BinaryExpr>(b->op, move(newLeft), move(newRight)));
        }
        case ExprKind::CALL: {
            auto* call = static_cast<CallExpr*>(expr);

            // 先递归处理参数
            vector<unique_ptr<Stmt>> preStmts;
            auto newCall = make_unique<CallExpr>(call->funcName);
            bool argsChanged = false;
            for (auto& arg : call->args) {
                auto [changed, stmts, newArg] = inlineExpr(arg.get());
                if (changed) argsChanged = true;
                for (auto& s : stmts) preStmts.push_back(move(s));
                newCall->args.push_back(move(newArg));
            }

            // 检查是否可以内联
            if (funcTable.count(call->funcName) && canInline(funcTable[call->funcName])) {
                FuncDef* func = funcTable[call->funcName];
                auto [inlinedStmts, resultVar] = inlineCall(call, func);
                for (auto& s : inlinedStmts) preStmts.push_back(move(s));
                return make_tuple(true, move(preStmts), make_unique<IdentExpr>(resultVar));
            }

            return make_tuple(argsChanged, move(preStmts), move(newCall));
        }
        }
        vector<unique_ptr<Stmt>> empty;
        return make_tuple(false, move(empty), unique_ptr<Expr>(nullptr));
    }

    // 在语句中内联函数调用
    bool inlineStmtList(vector<unique_ptr<Stmt>>& stmts) {
        bool changed = false;
        vector<unique_ptr<Stmt>> newStmts;

        for (auto& stmt : stmts) {
            vector<unique_ptr<Stmt>> preStmts;

            switch (stmt->kind) {
            case StmtKind::BLOCK: {
                auto* block = static_cast<BlockStmt*>(stmt.get());
                if (inlineStmtList(block->stmts)) changed = true;
                newStmts.push_back(move(stmt));
                break;
            }
            case StmtKind::VARDECL: {
                auto* v = static_cast<VarDeclStmt*>(stmt.get());
                auto [exprChanged, stmtsList, newInit] = inlineExpr(v->init.get());
                if (exprChanged) {
                    changed = true;
                    for (auto& s : stmtsList) newStmts.push_back(move(s));
                    newStmts.push_back(make_unique<VarDeclStmt>(v->name, move(newInit)));
                } else {
                    newStmts.push_back(move(stmt));
                }
                break;
            }
            case StmtKind::ASSIGN: {
                auto* a = static_cast<AssignStmt*>(stmt.get());
                auto [exprChanged, stmtsList, newValue] = inlineExpr(a->value.get());
                if (exprChanged) {
                    changed = true;
                    for (auto& s : stmtsList) newStmts.push_back(move(s));
                    newStmts.push_back(make_unique<AssignStmt>(a->name, move(newValue)));
                } else {
                    newStmts.push_back(move(stmt));
                }
                break;
            }
            case StmtKind::IF: {
                auto* i = static_cast<IfStmt*>(stmt.get());
                auto [condChanged, condStmts, newCond] = inlineExpr(i->cond.get());
                if (condChanged) changed = true;
                for (auto& s : condStmts) newStmts.push_back(move(s));

                if (i->thenStmt->kind == StmtKind::BLOCK) {
                    if (inlineStmtList(static_cast<BlockStmt*>(i->thenStmt.get())->stmts))
                        changed = true;
                }
                if (i->elseStmt && i->elseStmt->kind == StmtKind::BLOCK) {
                    if (inlineStmtList(static_cast<BlockStmt*>(i->elseStmt.get())->stmts))
                        changed = true;
                }

                if (condChanged) {
                    auto newIf = make_unique<IfStmt>();
                    newIf->cond = move(newCond);
                    newIf->thenStmt = move(i->thenStmt);
                    newIf->elseStmt = move(i->elseStmt);
                    newStmts.push_back(move(newIf));
                } else {
                    newStmts.push_back(move(stmt));
                }
                break;
            }
            case StmtKind::WHILE: {
                auto* w = static_cast<WhileStmt*>(stmt.get());
                // 循环条件内的函数调用不要内联（可能被多次执行）
                if (w->body->kind == StmtKind::BLOCK) {
                    if (inlineStmtList(static_cast<BlockStmt*>(w->body.get())->stmts))
                        changed = true;
                }
                newStmts.push_back(move(stmt));
                break;
            }
            case StmtKind::RETURN: {
                auto* r = static_cast<ReturnStmt*>(stmt.get());
                if (r->value) {
                    auto [exprChanged, stmtsList, newValue] = inlineExpr(r->value.get());
                    if (exprChanged) {
                        changed = true;
                        for (auto& s : stmtsList) newStmts.push_back(move(s));
                        newStmts.push_back(make_unique<ReturnStmt>(move(newValue)));
                    } else {
                        newStmts.push_back(move(stmt));
                    }
                } else {
                    newStmts.push_back(move(stmt));
                }
                break;
            }
            case StmtKind::EXPR: {
                auto* e = static_cast<ExprStmt*>(stmt.get());
                auto [exprChanged, stmtsList, newExpr] = inlineExpr(e->expr.get());
                if (exprChanged) {
                    changed = true;
                    for (auto& s : stmtsList) newStmts.push_back(move(s));
                    // 只有函数调用表达式需要保留
                    if (newExpr->kind == ExprKind::CALL) {
                        newStmts.push_back(make_unique<ExprStmt>(move(newExpr)));
                    }
                } else {
                    newStmts.push_back(move(stmt));
                }
                break;
            }
            default:
                newStmts.push_back(move(stmt));
                break;
            }
        }

        stmts = move(newStmts);
        return changed;
    }

    // 函数内联主入口
    void inlineFunctions(Program* prog) {
        // 构建函数表
        funcTable.clear();
        funcCallCount.clear();
        for (auto& func : prog->functions) {
            funcTable[func->name] = func.get();
        }

        // 统计函数调用次数
        for (auto& func : prog->functions) {
            countFuncCallsInStmt(func->body.get());
        }

        // 对每个函数进行内联
        for (auto& func : prog->functions) {
            inlineStmtList(func->body->stmts);
        }
    }

public:
    void optimize(Program* prog) {
        // 循环展开
        for (auto& func : prog->functions) {
            aggressiveLoopUnroll(func->body->stmts);
        }

        // 基础优化
        for (int round = 0; round < 10; round++) {
            bool changed = false;
            for (auto& func : prog->functions) {
                constVars.clear();
                copyVars.clear();

                for (auto& p : func->params) {
                    constVars.erase(p->name);
                }

                if (optimizeStmtList(func->body->stmts)) {
                    changed = true;
                }
            }
            if (!changed) break;
        }

        // 函数内联阶段
        inlineFunctions(prog);
        // 内联后运行基础优化
        for (int round = 0; round < 5; round++) {
            bool changed = false;
            for (auto& func : prog->functions) {
                constVars.clear();
                copyVars.clear();
                if (optimizeStmtList(func->body->stmts)) {
                    changed = true;
                }
            }
            if (!changed) break;
        }

        // 再次循环展开（处理内联后产生的新循环）
        for (auto& func : prog->functions) {
            aggressiveLoopUnroll(func->body->stmts);
        }
        // 展开后再次运行基础优化
        for (int round = 0; round < 5; round++) {
            bool changed = false;
            for (auto& func : prog->functions) {
                constVars.clear();
                copyVars.clear();
                if (optimizeStmtList(func->body->stmts)) {
                    changed = true;
                }
            }
            if (!changed) break;
        }

        // 第二阶段：循环不变量外提
        for (auto& func : prog->functions) {
            auto& stmts = func->body->stmts;
            for (size_t i = 0; i < stmts.size(); i++) {
                if (stmts[i]->kind == StmtKind::WHILE) {
                    vector<unique_ptr<Stmt>> hoisted;
                    hoistLoopInvariants(static_cast<WhileStmt*>(stmts[i].get()), hoisted);
                    // 插入到循环前
                    for (auto it = hoisted.rbegin(); it != hoisted.rend(); ++it) {
                        stmts.insert(stmts.begin() + i, move(*it));
                        i++;
                    }
                }
            }
        }

        // 第三阶段：归纳变量强度削减
        for (auto& func : prog->functions) {
            strengthReductionCount = 0;
            optimizeLoopStrengthReduction(func->body->stmts);
        }

        // 第四阶段：公共子表达式消除
        for (auto& func : prog->functions) {
            cseMap.clear();
            cseTempCount = 0;
            cseStmtList(func->body->stmts);
        }

        // 第五阶段：尾递归优化
        for (auto& func : prog->functions) {
            optimizeTailRecursion(func.get());
        }

        // 第六阶段：死变量消除
        for (int round = 0; round < 3; round++) {
            bool changed = false;
            for (auto& func : prog->functions) {
                if (eliminateDeadVars(func->body->stmts)) {
                    changed = true;
                }
            }
            if (!changed) break;
        }

        // 最后阶段：再次运行基础优化
        for (int round = 0; round < 5; round++) {
            bool changed = false;
            for (auto& func : prog->functions) {
                constVars.clear();
                copyVars.clear();
                if (optimizeStmtList(func->body->stmts)) {
                    changed = true;
                }
            }
            if (!changed) break;
        }
    }
};

// ==================== 代码生成器（直接生成汇编） ====================
class CodeGenerator {
private:
    ostringstream out;
    int labelCount = 0;
    int stackOffset = 0;
    int frameSize = 0;
    vector<string> breakLabels;
    vector<string> continueLabels;

    // 变量名 -> 栈偏移（相对于s0的负偏移）
    vector<map<string, int>> varScopes;
    // 参数名 -> 参数索引
    map<string, int> paramIndex;

    // ========== 运行时优化：函数分析信息 ==========
    bool currentFuncIsLeaf = true;      // 当前函数是否为叶函数（无call）
    int spDeltaBytes = 0;               // 当前sp相对于prologue后的偏移（用于16B对齐）

    // 局部变量寄存器化：变量名 -> 寄存器名
    map<string, string> varToReg;
    // 已使用的 callee-saved 寄存器
    vector<string> usedSRegs;
    // 变量使用频率统计
    map<string, int> varUseFreq;

    // ========== 寄存器分配器 ==========
    // 可用寄存器栈：t0-t6 共7个
    const char* tempRegs[7] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6"};

    // 获取log2值（辅助函数）
    int log2Int(int n) {
        int r = 0;
        while (n > 1) { n >>= 1; r++; }
        return r;
    }
    int regStackTop = 0;  // 当前使用的寄存器数量

    // 分配一个寄存器，返回寄存器名
    string allocReg() {
        if (g_optimize && regStackTop < 7) {
            return tempRegs[regStackTop++];
        }
        // 回退到 t0
        return "t0";
    }

    // 释放最后分配的寄存器
    void freeReg() {
        if (g_optimize && regStackTop > 0) {
            regStackTop--;
        }
    }

    // 获取当前结果寄存器
    string currentReg() {
        if (g_optimize && regStackTop > 0) {
            return tempRegs[regStackTop - 1];
        }
        return "t0";
    }

    // 检查是否需要溢出到栈
    bool needSpill() {
        return !g_optimize || regStackTop >= 7;
    }

    string newLabel() { return "L" + to_string(labelCount++); }

    void emit(const string& s) { out << "\t" << s << "\n"; }
    void emitLabel(const string& s) { out << s << ":\n"; }

    // ========== 函数预分析：检测叶函数、统计变量使用 ==========
    // 检查表达式是否包含函数调用
    bool exprHasCall(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
        case ExprKind::IDENT:
            return false;
        case ExprKind::UNARY:
            return exprHasCall(static_cast<UnaryExpr*>(expr)->operand.get());
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            return exprHasCall(b->left.get()) || exprHasCall(b->right.get());
        }
        case ExprKind::CALL:
            return true;
        }
        return false;
    }

    // 检查语句是否包含函数调用
    bool stmtHasCall(Stmt* stmt) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) {
                if (stmtHasCall(s.get())) return true;
            }
            return false;
        }
        case StmtKind::VARDECL:
            return exprHasCall(static_cast<VarDeclStmt*>(stmt)->init.get());
        case StmtKind::ASSIGN:
            return exprHasCall(static_cast<AssignStmt*>(stmt)->value.get());
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            if (exprHasCall(i->cond.get())) return true;
            if (stmtHasCall(i->thenStmt.get())) return true;
            if (i->elseStmt && stmtHasCall(i->elseStmt.get())) return true;
            return false;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            if (exprHasCall(w->cond.get())) return true;
            return stmtHasCall(w->body.get());
        }
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) return exprHasCall(r->value.get());
            return false;
        }
        case StmtKind::EXPR:
            return exprHasCall(static_cast<ExprStmt*>(stmt)->expr.get());
        default:
            return false;
        }
    }

    // 统计表达式中变量的使用次数
    void countVarUseInExpr(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::IDENT:
            varUseFreq[static_cast<IdentExpr*>(expr)->name]++;
            break;
        case ExprKind::UNARY:
            countVarUseInExpr(static_cast<UnaryExpr*>(expr)->operand.get());
            break;
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            countVarUseInExpr(b->left.get());
            countVarUseInExpr(b->right.get());
            break;
        }
        case ExprKind::CALL: {
            auto* c = static_cast<CallExpr*>(expr);
            for (auto& arg : c->args) countVarUseInExpr(arg.get());
            break;
        }
        default:
            break;
        }
    }

    // 统计语句中变量的使用次数
    void countVarUseInStmtGen(Stmt* stmt) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) countVarUseInStmtGen(s.get());
            break;
        }
        case StmtKind::VARDECL: {
            auto* v = static_cast<VarDeclStmt*>(stmt);
            countVarUseInExpr(v->init.get());
            varUseFreq[v->name]++;  // 声明也算一次使用
            break;
        }
        case StmtKind::ASSIGN: {
            auto* a = static_cast<AssignStmt*>(stmt);
            countVarUseInExpr(a->value.get());
            varUseFreq[a->name]++;  // 赋值目标也算一次使用
            break;
        }
        case StmtKind::IF: {
            auto* i = static_cast<IfStmt*>(stmt);
            countVarUseInExpr(i->cond.get());
            countVarUseInStmtGen(i->thenStmt.get());
            if (i->elseStmt) countVarUseInStmtGen(i->elseStmt.get());
            break;
        }
        case StmtKind::WHILE: {
            auto* w = static_cast<WhileStmt*>(stmt);
            // 循环内的变量使用权重更高
            countVarUseInExpr(w->cond.get());
            varUseFreq[static_cast<IdentExpr*>(w->cond.get())->name] += 10;  // 条件变量权重增加
            countVarUseInStmtGen(w->body.get());
            // 循环体内所有变量权重翻倍
            break;
        }
        case StmtKind::RETURN: {
            auto* r = static_cast<ReturnStmt*>(stmt);
            if (r->value) countVarUseInExpr(r->value.get());
            break;
        }
        case StmtKind::EXPR:
            countVarUseInExpr(static_cast<ExprStmt*>(stmt)->expr.get());
            break;
        default:
            break;
        }
    }

    // 分析函数，收集优化所需信息
    void analyzeFunction(FuncDef* func) {
        // 重置分析状态
        currentFuncIsLeaf = true;
        varUseFreq.clear();
        varToReg.clear();
        usedSRegs.clear();

        // 检查是否为叶函数
        for (auto& stmt : func->body->stmts) {
            if (stmtHasCall(stmt.get())) {
                currentFuncIsLeaf = false;
                break;
            }
        }

        // 统计变量使用频率
        for (auto& stmt : func->body->stmts) {
            countVarUseInStmtGen(stmt.get());
        }

        // 局部变量寄存器化：选择使用频率最高的变量分配到 s1-s6
        if (g_optimize && !varUseFreq.empty()) {
            // 排序变量按使用频率
            vector<pair<string, int>> sortedVars(varUseFreq.begin(), varUseFreq.end());
            sort(sortedVars.begin(), sortedVars.end(),
                 [](const pair<string, int>& a, const pair<string, int>& b) {
                     return a.second > b.second;
                 });

            // 分配 s1-s6 给使用频率最高的局部变量（不包括参数）
            const char* sRegs[] = {"s1", "s2", "s3", "s4", "s5", "s6"};
            int regIdx = 0;
            for (auto& [varName, freq] : sortedVars) {
                if (regIdx >= 6) break;
                // 只分配给局部变量，不分配给参数
                if (paramIndex.find(varName) == paramIndex.end() && freq >= 3) {
                    varToReg[varName] = sRegs[regIdx];
                    usedSRegs.push_back(sRegs[regIdx]);
                    regIdx++;
                }
            }
        }
    }

    // ========== SU编号：用于表达式寄存器分配 ==========
    // 计算表达式的 Sethi-Ullman 寄存器需求数
    int computeSU(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::NUMBER:
        case ExprKind::IDENT:
            return 1;
        case ExprKind::UNARY:
            return computeSU(static_cast<UnaryExpr*>(expr)->operand.get());
        case ExprKind::BINARY: {
            auto* b = static_cast<BinaryExpr*>(expr);
            int leftSU = computeSU(b->left.get());
            int rightSU = computeSU(b->right.get());
            if (leftSU == rightSU) return leftSU + 1;
            return max(leftSU, rightSU);
        }
        case ExprKind::CALL:
            return 10;  // 保守值，强制 spill
        }
        return 1;
    }

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
        // 查找全局变量
        throw runtime_error("Undefined variable: " + name);
    }

    // 生成表达式到指定寄存器（用于多寄存器分配）
    void genExprToReg(Expr* expr, const string& targetReg) {
        switch (expr->kind) {
        case ExprKind::NUMBER: {
            auto* num = static_cast<NumberExpr*>(expr);
            emit("li " + targetReg + ", " + to_string(num->value));
            break;
        }
        case ExprKind::IDENT: {
            auto* ident = static_cast<IdentExpr*>(expr);
            // 检查是否在寄存器中
            if (g_optimize && varToReg.count(ident->name)) {
                if (varToReg[ident->name] != targetReg) {
                    emit("mv " + targetReg + ", " + varToReg[ident->name]);
                }
                break;
            }
            int paramIdx;
            int offset = lookupVar(ident->name, paramIdx);
            if (paramIdx >= 0) {
                if (paramIdx < 8) {
                    emit("lw " + targetReg + ", " + to_string(-12 - paramIdx * 4) + "(s0)");
                } else {
                    emit("lw " + targetReg + ", " + to_string((paramIdx - 8) * 4) + "(s0)");
                }
            } else {
                emit("lw " + targetReg + ", " + to_string(offset) + "(s0)");
            }
            break;
        }
        default:
            // 对于复杂表达式，先计算到t0，再移动到目标寄存器
            genExpr(expr);
            if (targetReg != "t0") {
                emit("mv " + targetReg + ", t0");
            }
            break;
        }
    }

    // 生成表达式，结果存入t0
    // 优化：使用 switch + static_cast 替代 dynamic_cast，避免 RTTI 开销
    void genExpr(Expr* expr) {
        switch (expr->kind) {
        case ExprKind::NUMBER: {
            auto* num = static_cast<NumberExpr*>(expr);
            emit("li t0, " + to_string(num->value));
            break;
        }
        case ExprKind::IDENT: {
            auto* ident = static_cast<IdentExpr*>(expr);
            // 检查是否在寄存器中
            if (g_optimize && varToReg.count(ident->name)) {
                emit("mv t0, " + varToReg[ident->name]);
                break;
            }
            int paramIdx;
            int offset = lookupVar(ident->name, paramIdx);
            if (paramIdx >= 0) {
                if (paramIdx < 8) {
                    emit("lw t0, " + to_string(-12 - paramIdx * 4) + "(s0)");
                } else {
                    emit("lw t0, " + to_string((paramIdx - 8) * 4) + "(s0)");
                }
            } else {
                emit("lw t0, " + to_string(offset) + "(s0)");
            }
            break;
        }
        case ExprKind::UNARY: {
            auto* unary = static_cast<UnaryExpr*>(expr);
            genExpr(unary->operand.get());
            if (unary->op == "-") {
                emit("neg t0, t0");
            } else if (unary->op == "!") {
                emit("seqz t0, t0");
            }
            break;
        }
        case ExprKind::BINARY: {
            auto* binary = static_cast<BinaryExpr*>(expr);
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
                break;
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
                break;
            }
            // ========== 乘法优化 ==========
            if (g_optimize && binary->op == "*") {
                // 检查右操作数是否为常量
                if (binary->right->kind == ExprKind::NUMBER) {
                    int val = static_cast<NumberExpr*>(binary->right.get())->value;

                    // 2的幂 -> 移位
                    if (val > 0 && (val & (val - 1)) == 0) {
                        genExpr(binary->left.get());
                        int shift = 0;
                        int tmp = val;
                        while (tmp > 1) { tmp >>= 1; shift++; }
                        emit("slli t0, t0, " + to_string(shift));
                        break;
                    }

                    // x * 3 = (x << 1) + x
                    if (val == 3) {
                        genExpr(binary->left.get());
                        emit("slli t1, t0, 1");
                        emit("add t0, t1, t0");
                        break;
                    }

                    // x * 5 = (x << 2) + x
                    if (val == 5) {
                        genExpr(binary->left.get());
                        emit("slli t1, t0, 2");
                        emit("add t0, t1, t0");
                        break;
                    }

                    // x * 6 = (x << 2) + (x << 1)
                    if (val == 6) {
                        genExpr(binary->left.get());
                        emit("slli t1, t0, 2");
                        emit("slli t2, t0, 1");
                        emit("add t0, t1, t2");
                        break;
                    }

                    // x * 7 = (x << 3) - x
                    if (val == 7) {
                        genExpr(binary->left.get());
                        emit("slli t1, t0, 3");
                        emit("sub t0, t1, t0");
                        break;
                    }

                    // x * 9 = (x << 3) + x
                    if (val == 9) {
                        genExpr(binary->left.get());
                        emit("slli t1, t0, 3");
                        emit("add t0, t1, t0");
                        break;
                    }

                    // x * 10 = (x << 3) + (x << 1)
                    if (val == 10) {
                        genExpr(binary->left.get());
                        emit("slli t1, t0, 3");
                        emit("slli t2, t0, 1");
                        emit("add t0, t1, t2");
                        break;
                    }

                    // x * 15 = (x << 4) - x
                    if (val == 15) {
                        genExpr(binary->left.get());
                        emit("slli t1, t0, 4");
                        emit("sub t0, t1, t0");
                        break;
                    }
                }

                // 检查左操作数是否为常量（交换律）
                if (binary->left->kind == ExprKind::NUMBER) {
                    int val = static_cast<NumberExpr*>(binary->left.get())->value;

                    // 2的幂 -> 移位
                    if (val > 0 && (val & (val - 1)) == 0) {
                        genExpr(binary->right.get());
                        int shift = 0;
                        int tmp = val;
                        while (tmp > 1) { tmp >>= 1; shift++; }
                        emit("slli t0, t0, " + to_string(shift));
                        break;
                    }

                    // 3 * x = (x << 1) + x
                    if (val == 3) {
                        genExpr(binary->right.get());
                        emit("slli t1, t0, 1");
                        emit("add t0, t1, t0");
                        break;
                    }

                    // 5 * x = (x << 2) + x
                    if (val == 5) {
                        genExpr(binary->right.get());
                        emit("slli t1, t0, 2");
                        emit("add t0, t1, t0");
                        break;
                    }

                    // 6 * x = (x << 2) + (x << 1)
                    if (val == 6) {
                        genExpr(binary->right.get());
                        emit("slli t1, t0, 2");
                        emit("slli t2, t0, 1");
                        emit("add t0, t1, t2");
                        break;
                    }

                    // 7 * x = (x << 3) - x
                    if (val == 7) {
                        genExpr(binary->right.get());
                        emit("slli t1, t0, 3");
                        emit("sub t0, t1, t0");
                        break;
                    }

                    // 9 * x = (x << 3) + x
                    if (val == 9) {
                        genExpr(binary->right.get());
                        emit("slli t1, t0, 3");
                        emit("add t0, t1, t0");
                        break;
                    }

                    // 10 * x = (x << 3) + (x << 1)
                    if (val == 10) {
                        genExpr(binary->right.get());
                        emit("slli t1, t0, 3");
                        emit("slli t2, t0, 1");
                        emit("add t0, t1, t2");
                        break;
                    }

                    // 15 * x = (x << 4) - x
                    if (val == 15) {
                        genExpr(binary->right.get());
                        emit("slli t1, t0, 4");
                        emit("sub t0, t1, t0");
                        break;
                    }
                }
            }

            // ========== 除法优化 ==========
            if (g_optimize && binary->op == "/" && binary->right->kind == ExprKind::NUMBER) {
                int val = static_cast<NumberExpr*>(binary->right.get())->value;
                if (val > 0 && (val & (val - 1)) == 0) {
                    genExpr(binary->left.get());
                    int shift = 0;
                    int tmp = val;
                    while (tmp > 1) { tmp >>= 1; shift++; }

                    // 有符号除法需要处理负数舍入
                    if (shift > 0) {
                        emit("srai t1, t0, 31");                       // 符号位扩展
                        emit("srli t1, t1, " + to_string(32 - shift)); // 调整值
                        emit("add t0, t0, t1");                        // 加偏移
                    }
                    emit("srai t0, t0, " + to_string(shift));
                    break;
                }
            }

            // ========== 取模优化 ==========
            if (g_optimize && binary->op == "%" && binary->right->kind == ExprKind::NUMBER) {
                int val = static_cast<NumberExpr*>(binary->right.get())->value;
                if (val > 0 && (val & (val - 1)) == 0) {
                    if (val == 1) {
                        // x % 1 = 0
                        emit("li t0, 0");
                        break;
                    }
                    genExpr(binary->left.get());
                    // 有符号取模需要特殊处理
                    // x % n = x - (x / n) * n, 对于2的幂可以简化
                    // 简化版本：对于非负数，直接用 andi
                    // 完整版本：需要处理负数情况
                    int shift = log2Int(val);
                    emit("srai t1, t0, 31");                       // 符号位
                    emit("srli t1, t1, " + to_string(32 - shift)); // 调整值
                    emit("add t2, t0, t1");                        // 调整后的被除数
                    int mask = ~(val - 1);
                    if (mask >= -2048 && mask <= 2047) {
                        emit("andi t2, t2, " + to_string(mask));   // 对齐到val的倍数
                    } else {
                        emit("li t1, " + to_string(mask));         // 掩码超出12位范围，用寄存器
                        emit("and t2, t2, t1");
                    }
                    emit("sub t0, t0, t2");                        // 原值减去对齐值
                    break;
                }
            }

            // 普通二元运算 - 使用多寄存器分配器优化
            // 只有当两个操作数都是简单表达式时才使用多寄存器
            bool leftSimple = (binary->left->kind == ExprKind::NUMBER || binary->left->kind == ExprKind::IDENT);
            bool rightSimple = (binary->right->kind == ExprKind::NUMBER || binary->right->kind == ExprKind::IDENT);

            if (g_optimize && leftSimple && rightSimple) {
                // 两个操作数都是简单表达式，直接使用不同寄存器
                genExprToReg(binary->left.get(), "t1");
                genExprToReg(binary->right.get(), "t2");

                if (binary->op == "+") emit("add t0, t1, t2");
                else if (binary->op == "-") emit("sub t0, t1, t2");
                else if (binary->op == "*") emit("mul t0, t1, t2");
                else if (binary->op == "/") emit("div t0, t1, t2");
                else if (binary->op == "%") emit("rem t0, t1, t2");
                else if (binary->op == "<") emit("slt t0, t1, t2");
                else if (binary->op == ">") emit("slt t0, t2, t1");
                else if (binary->op == "<=") {
                    emit("slt t0, t2, t1");
                    emit("xori t0, t0, 1");
                }
                else if (binary->op == ">=") {
                    emit("slt t0, t1, t2");
                    emit("xori t0, t0, 1");
                }
                else if (binary->op == "==") {
                    emit("sub t0, t1, t2");
                    emit("seqz t0, t0");
                }
                else if (binary->op == "!=") {
                    emit("sub t0, t1, t2");
                    emit("snez t0, t0");
                }
                break;
            }

            // 回退到栈保存方式（复杂表达式时）
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
            break;
        }
        case ExprKind::CALL: {
            auto* call = static_cast<CallExpr*>(expr);
            int argCount = call->args.size();
            int stackArgs = (argCount > 8) ? (argCount - 8) : 0;

            if (argCount > 0) {
                int tempSpace = argCount * 4;
                int stackArgsSpace = stackArgs * 4;
                emit("addi sp, sp, -" + to_string(tempSpace + stackArgsSpace));

                for (int i = 0; i < argCount; i++) {
                    genExpr(call->args[i].get());
                    emit("sw t0, " + to_string(stackArgsSpace + i * 4) + "(sp)");
                }

                for (int i = 0; i < argCount && i < 8; i++) {
                    emit("lw a" + to_string(i) + ", " + to_string(stackArgsSpace + i * 4) + "(sp)");
                }

                for (int i = 8; i < argCount; i++) {
                    emit("lw t0, " + to_string(stackArgsSpace + i * 4) + "(sp)");
                    emit("sw t0, " + to_string((i - 8) * 4) + "(sp)");
                }

                emit("addi sp, sp, " + to_string(tempSpace));
            }

            emit("call " + call->funcName);

            if (stackArgs > 0) {
                emit("addi sp, sp, " + to_string(stackArgs * 4));
            }
            emit("mv t0, a0");
            break;
        }
        }
    }

    // 优化：使用 switch + static_cast 替代 dynamic_cast
    void genStmt(Stmt* stmt) {
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            varScopes.push_back({});
            for (auto& s : block->stmts) genStmt(s.get());
            varScopes.pop_back();
            break;
        }
        case StmtKind::EMPTY:
            break;
        case StmtKind::VARDECL: {
            auto* varDecl = static_cast<VarDeclStmt*>(stmt);
            genExpr(varDecl->init.get());
            int offset = allocVar(varDecl->name);
            emit("sw t0, " + to_string(offset) + "(s0)");
            break;
        }
        case StmtKind::ASSIGN: {
            auto* assign = static_cast<AssignStmt*>(stmt);
            genExpr(assign->value.get());
            int paramIdx;
            int offset = lookupVar(assign->name, paramIdx);
            if (paramIdx >= 0) {
                if (paramIdx < 8) {
                    emit("sw t0, " + to_string(-12 - paramIdx * 4) + "(s0)");
                } else {
                    emit("sw t0, " + to_string((paramIdx - 8) * 4) + "(s0)");
                }
            } else {
                emit("sw t0, " + to_string(offset) + "(s0)");
            }
            break;
        }
        case StmtKind::IF: {
            auto* ifStmt = static_cast<IfStmt*>(stmt);
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
            break;
        }
        case StmtKind::WHILE: {
            auto* whileStmt = static_cast<WhileStmt*>(stmt);
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
            break;
        }
        case StmtKind::BREAK:
            emit("j " + breakLabels.back());
            break;
        case StmtKind::CONTINUE:
            emit("j " + continueLabels.back());
            break;
        case StmtKind::RETURN: {
            auto* ret = static_cast<ReturnStmt*>(stmt);
            if (ret->value) {
                genExpr(ret->value.get());
                emit("mv a0, t0");
            }
            emit("lw ra, " + to_string(frameSize - 4) + "(sp)");
            emit("lw s0, " + to_string(frameSize - 8) + "(sp)");
            emit("addi sp, sp, " + to_string(frameSize));
            emit("ret");
            break;
        }
        case StmtKind::EXPR: {
            auto* exprStmt = static_cast<ExprStmt*>(stmt);
            genExpr(exprStmt->expr.get());
            break;
        }
        }
    }

    // 计算函数需要的栈空间（遍历所有变量声明）
    int countLocalVars(Stmt* stmt) {
        int count = 0;
        switch (stmt->kind) {
        case StmtKind::BLOCK: {
            auto* block = static_cast<BlockStmt*>(stmt);
            for (auto& s : block->stmts) count += countLocalVars(s.get());
            break;
        }
        case StmtKind::VARDECL:
            count = 1;
            break;
        case StmtKind::IF: {
            auto* ifStmt = static_cast<IfStmt*>(stmt);
            count = countLocalVars(ifStmt->thenStmt.get());
            if (ifStmt->elseStmt) count += countLocalVars(ifStmt->elseStmt.get());
            break;
        }
        case StmtKind::WHILE: {
            auto* whileStmt = static_cast<WhileStmt*>(stmt);
            count = countLocalVars(whileStmt->body.get());
            break;
        }
        default:
            break;
        }
        return count;
    }

    void genFunc(FuncDef* func) {
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

        // 保存参数（在 ra 和 s0 之后，从 s0-12 开始）
        // 栈布局: s0-4=ra, s0-8=s0, s0-12=param0, s0-16=param1, ...
        for (int i = 0; i < paramCount && i < 8; i++) {
            emit("sw a" + to_string(i) + ", " + to_string(-12 - i * 4) + "(s0)");
        }

        // 设置局部变量起始偏移（跳过 ra、s0 和参数）
        stackOffset = -8 - paramCount * 4;

        // 生成函数体
        for (auto& stmt : func->body->stmts) {
            genStmt(stmt.get());
        }

        // void 函数可自然结束，补一个返回
        if (func->isVoid) {
            emit("lw ra, " + to_string(frameSize - 4) + "(sp)");
            emit("lw s0, " + to_string(frameSize - 8) + "(sp)");
            emit("addi sp, sp, " + to_string(frameSize));
            emit("ret");
        }

        out << "\n";
    }

    // ========== 窥孔优化 ==========
    // 解析指令，返回操作码和操作数
    tuple<string, vector<string>> parseInstruction(const string& line) {
        string trimmed = line;
        // 去除前导空白
        size_t start = trimmed.find_first_not_of(" \t");
        if (start == string::npos) return {"", {}};
        trimmed = trimmed.substr(start);

        // 跳过标签
        if (trimmed.back() == ':' || trimmed[0] == '.') return {"", {}};

        // 分割操作码和操作数
        size_t space = trimmed.find_first_of(" \t");
        if (space == string::npos) return {trimmed, {}};

        string opcode = trimmed.substr(0, space);
        string rest = trimmed.substr(space + 1);

        // 分割操作数
        vector<string> operands;
        stringstream ss(rest);
        string operand;
        while (getline(ss, operand, ',')) {
            // 去除空白
            size_t s = operand.find_first_not_of(" \t");
            size_t e = operand.find_last_not_of(" \t");
            if (s != string::npos && e != string::npos) {
                operands.push_back(operand.substr(s, e - s + 1));
            }
        }

        return {opcode, operands};
    }

    // 窥孔优化主函数
    string peepholeOptimize(const string& code) {
        if (!g_optimize) return code;

        vector<string> lines;
        stringstream ss(code);
        string line;
        while (getline(ss, line)) {
            lines.push_back(line);
        }

        bool changed = true;
        while (changed) {
            changed = false;
            vector<string> newLines;

            for (size_t i = 0; i < lines.size(); i++) {
                auto [op, operands] = parseInstruction(lines[i]);

                // 模式1: mv reg, reg (相同寄存器) -> 删除
                if (op == "mv" && operands.size() == 2 && operands[0] == operands[1]) {
                    changed = true;
                    continue;
                }

                // 模式2: addi reg, reg, 0 -> 删除
                if (op == "addi" && operands.size() == 3 && operands[0] == operands[1] && operands[2] == "0") {
                    changed = true;
                    continue;
                }

                // 模式3: slli reg, reg, 0 -> 删除 (左移0位)
                if (op == "slli" && operands.size() == 3 && operands[2] == "0") {
                    changed = true;
                    continue;
                }

                // 模式4: srai reg, reg, 0 -> 删除 (右移0位)
                if (op == "srai" && operands.size() == 3 && operands[2] == "0") {
                    changed = true;
                    continue;
                }

                // 模式5: j L 后面紧跟 L: -> 删除 j
                if (op == "j" && operands.size() == 1 && i + 1 < lines.size()) {
                    string nextLine = lines[i + 1];
                    size_t start = nextLine.find_first_not_of(" \t");
                    if (start != string::npos) {
                        string label = nextLine.substr(start);
                        if (label == operands[0] + ":") {
                            changed = true;
                            continue;
                        }
                    }
                }

                // 模式6: 连续两条 li 到同一寄存器 -> 保留后一条
                if (op == "li" && operands.size() == 2 && i + 1 < lines.size()) {
                    auto [nextOp, nextOperands] = parseInstruction(lines[i + 1]);
                    if (nextOp == "li" && nextOperands.size() == 2 && nextOperands[0] == operands[0]) {
                        changed = true;
                        continue;  // 删除当前行
                    }
                }

                // 模式7: sw reg, X(sp); lw reg, X(sp) -> 删除 lw
                if (op == "sw" && operands.size() == 2 && i + 1 < lines.size()) {
                    auto [nextOp, nextOperands] = parseInstruction(lines[i + 1]);
                    if (nextOp == "lw" && nextOperands.size() == 2 &&
                        nextOperands[0] == operands[0] && nextOperands[1] == operands[1]) {
                        newLines.push_back(lines[i]);
                        i++;  // 跳过下一条 lw
                        changed = true;
                        continue;
                    }
                }

                // 模式8: addi sp, sp, -N; addi sp, sp, N 连续出现 -> 删除两条
                if (op == "addi" && operands.size() == 3 && operands[0] == "sp" &&
                    operands[1] == "sp" && i + 1 < lines.size()) {
                    auto [nextOp, nextOperands] = parseInstruction(lines[i + 1]);
                    if (nextOp == "addi" && nextOperands.size() == 3 &&
                        nextOperands[0] == "sp" && nextOperands[1] == "sp") {
                        // 检查是否是相反的偏移
                        try {
                            int offset1 = stoi(operands[2]);
                            int offset2 = stoi(nextOperands[2]);
                            if (offset1 + offset2 == 0) {
                                i++;  // 跳过下一条
                                changed = true;
                                continue;
                            }
                        } catch (...) {}
                    }
                }

                // 模式9: li t0, 0; add t0, t0, t1 -> mv t0, t1
                if (op == "li" && operands.size() == 2 && operands[1] == "0" && i + 1 < lines.size()) {
                    auto [nextOp, nextOperands] = parseInstruction(lines[i + 1]);
                    if (nextOp == "add" && nextOperands.size() == 3 &&
                        nextOperands[0] == operands[0] && nextOperands[1] == operands[0]) {
                        newLines.push_back("\tmv " + nextOperands[0] + ", " + nextOperands[2]);
                        i++;  // 跳过下一条
                        changed = true;
                        continue;
                    }
                    // 也检查 add t0, t1, t0 的情况
                    if (nextOp == "add" && nextOperands.size() == 3 &&
                        nextOperands[0] == operands[0] && nextOperands[2] == operands[0]) {
                        newLines.push_back("\tmv " + nextOperands[0] + ", " + nextOperands[1]);
                        i++;  // 跳过下一条
                        changed = true;
                        continue;
                    }
                }

                // 模式10: neg t0, t0; neg t0, t0 -> 删除两条
                if (op == "neg" && operands.size() == 2 && i + 1 < lines.size()) {
                    auto [nextOp, nextOperands] = parseInstruction(lines[i + 1]);
                    if (nextOp == "neg" && nextOperands.size() == 2 &&
                        nextOperands[0] == operands[0] && nextOperands[1] == operands[1] &&
                        operands[0] == operands[1] && nextOperands[0] == nextOperands[1]) {
                        i++;  // 跳过下一条
                        changed = true;
                        continue;
                    }
                }

                newLines.push_back(lines[i]);
            }

            lines = move(newLines);
        }

        // 重新组装代码
        ostringstream result;
        for (const auto& l : lines) {
            result << l << "\n";
        }
        return result.str();
    }

public:
    string generate(Program* prog) {
        out << ".text\n\n";
        for (auto& func : prog->functions) {
            genFunc(func.get());
        }
        return peepholeOptimize(out.str());
    }
};

// ==================== 主函数 ====================
int main(int argc, char* argv[]) {
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-opt") {
            g_optimize = true;
        }
    }

    string source, line;
    while (getline(cin, line)) source += line + "\n";

    try {
        Lexer lexer(source);
        vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        auto ast = parser.parse();

        // 如果启用优化，执行常量折叠等优化
        if (g_optimize) {
            Optimizer optimizer;
            optimizer.optimize(ast.get());
        }

        CodeGenerator codeGen;
        cout << codeGen.generate(ast.get());

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
