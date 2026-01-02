#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <cctype>

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
            }
            if (binary->op == "-") {
                // x - 0 = x
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 0)
                    return left;
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
                // 强度削减: x * 2^n = x << n
                if (right->kind == ExprKind::NUMBER) {
                    int val = getConstValue(right.get());
                    if (isPowerOfTwo(val)) {
                        // 用加法代替乘以2: x * 2 = x + x
                        if (val == 2) {
                            return make_unique<BinaryExpr>("+", cloneExpr(left.get()), cloneExpr(left.get()));
                        }
                        // x * 4 = (x + x) + (x + x)
                        if (val == 4) {
                            auto x2 = make_unique<BinaryExpr>("+", cloneExpr(left.get()), cloneExpr(left.get()));
                            return make_unique<BinaryExpr>("+", cloneExpr(x2.get()), move(x2));
                        }
                    }
                }
            }
            if (binary->op == "/") {
                // x / 1 = x
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 1)
                    return left;
                // x / x = 1 (assuming x != 0)
                if (left->kind == ExprKind::IDENT && right->kind == ExprKind::IDENT &&
                    static_cast<IdentExpr*>(left.get())->name == static_cast<IdentExpr*>(right.get())->name)
                    return make_unique<NumberExpr>(1);
            }
            if (binary->op == "%") {
                // x % 1 = 0
                if (right->kind == ExprKind::NUMBER && getConstValue(right.get()) == 1)
                    return make_unique<NumberExpr>(0);
            }
            // 短路求值优化
            if (binary->op == "&&") {
                if (left->kind == ExprKind::NUMBER) {
                    if (getConstValue(left.get()) == 0) return make_unique<NumberExpr>(0);
                    else return right;
                }
            }
            if (binary->op == "||") {
                if (left->kind == ExprKind::NUMBER) {
                    if (getConstValue(left.get()) != 0) return make_unique<NumberExpr>(1);
                    else return right;
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
                    optimizeStmtList(static_cast<BlockStmt*>(ifStmt->thenStmt.get())->stmts);
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
                whileStmt->cond = foldExpr(whileStmt->cond.get());

                // while(0) 消除
                if (whileStmt->cond->kind == ExprKind::NUMBER &&
                    getConstValue(whileStmt->cond.get()) == 0) {
                    it = stmts.erase(it);
                    changed = true;
                    continue;
                }

                // 循环内的优化（保守：清除循环体可能修改的变量）
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

                // 组合循环体
                for (auto& s : outerBody->stmts) {
                    static_cast<BlockStmt*>(loopBody.get())->stmts.insert(
                        static_cast<BlockStmt*>(loopBody.get())->stmts.begin(),
                        move(s));
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
            if (sig.length() > 10 && !hasCallExpr(newExpr.get())) {
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
                // 分支内的 CSE 需要独立处理
                map<string, string> savedCse = cseMap;
                if (i->thenStmt->kind == StmtKind::BLOCK) {
                    cseStmtList(static_cast<BlockStmt*>(i->thenStmt.get())->stmts);
                }
                cseMap = savedCse;
                if (i->elseStmt && i->elseStmt->kind == StmtKind::BLOCK) {
                    cseStmtList(static_cast<BlockStmt*>(i->elseStmt.get())->stmts);
                }
                cseMap = savedCse;
                break;
            }
            case StmtKind::WHILE: {
                auto* w = static_cast<WhileStmt*>(stmt.get());
                // 循环内清空 CSE（保守策略）
                cseMap.clear();
                if (w->body->kind == StmtKind::BLOCK) {
                    cseStmtList(static_cast<BlockStmt*>(w->body.get())->stmts);
                }
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

        // 收集所有使用的变量
        set<string> usedVars;
        for (auto& stmt : stmts) {
            collectUsedVarsInStmt(stmt.get(), usedVars);
        }

        // 删除未使用变量的声明（如果初始化没有副作用）
        for (auto it = stmts.begin(); it != stmts.end(); ) {
            if ((*it)->kind == StmtKind::VARDECL) {
                auto* v = static_cast<VarDeclStmt*>(it->get());
                if (usedVars.find(v->name) == usedVars.end() &&
                    !hasCallExpr(v->init.get())) {
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

public:
    void optimize(Program* prog) {
        // 第一阶段：多轮基础优化
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

        // 第三阶段：公共子表达式消除
        for (auto& func : prog->functions) {
            cseMap.clear();
            cseTempCount = 0;
            cseStmtList(func->body->stmts);
        }

        // 第四阶段：尾递归优化
        for (auto& func : prog->functions) {
            optimizeTailRecursion(func.get());
        }

        // 第五阶段：死变量消除
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
        // 查找全局变量
        throw runtime_error("Undefined variable: " + name);
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
