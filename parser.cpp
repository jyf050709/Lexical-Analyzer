#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

// Token结构，包含行号信息
struct Token {
    int index;
    string type;
    string value;
    int line;
};

// 词法分析器
class Lexer {
private:
    string source;
    int position;
    int length;
    vector<Token> tokens;
    int tokenIndex;
    int currentLine;

    map<string, string> keywords;

    void initKeywords() {
        keywords["int"] = "'int'";
        keywords["void"] = "'void'";
        keywords["if"] = "'if'";
        keywords["else"] = "'else'";
        keywords["while"] = "'while'";
        keywords["break"] = "'break'";
        keywords["continue"] = "'continue'";
        keywords["return"] = "'return'";
    }

    char currentChar() {
        if (position >= length) return '\0';
        return source[position];
    }

    char peekChar(int offset = 1) {
        int pos = position + offset;
        if (pos >= length) return '\0';
        return source[pos];
    }

    void advance() {
        position++;
    }

    void skipWhitespace() {
        while (position < length && isspace(currentChar())) {
            if (currentChar() == '\n') {
                currentLine++;
            }
            advance();
        }
    }

    void skipComment() {
        if (currentChar() == '/' && peekChar() == '/') {
            advance();
            advance();
            while (position < length && currentChar() != '\n') {
                advance();
            }
        } else if (currentChar() == '/' && peekChar() == '*') {
            advance();
            advance();
            while (position < length - 1) {
                if (currentChar() == '\n') {
                    currentLine++;
                }
                if (currentChar() == '*' && peekChar() == '/') {
                    advance();
                    advance();
                    break;
                }
                advance();
            }
        }
    }

    string readIdentifier() {
        string result;
        while (position < length && (isalnum(currentChar()) || currentChar() == '_')) {
            result += currentChar();
            advance();
        }
        return result;
    }

    string readNumber() {
        string result;
        while (position < length && isdigit(currentChar())) {
            result += currentChar();
            advance();
        }
        return result;
    }

    void addToken(string type, string value) {
        Token token;
        token.index = tokenIndex++;
        token.type = type;
        token.value = value;
        token.line = currentLine;
        tokens.push_back(token);
    }

public:
    Lexer(const string& src) : source(src), position(0), length(src.length()), tokenIndex(0), currentLine(1) {
        initKeywords();
    }

    vector<Token> tokenize() {
        while (position < length) {
            skipWhitespace();
            if (position >= length) break;

            char ch = currentChar();

            if (ch == '/' && (peekChar() == '/' || peekChar() == '*')) {
                skipComment();
                continue;
            }

            if (isalpha(ch) || ch == '_') {
                string id = readIdentifier();
                if (keywords.find(id) != keywords.end()) {
                    addToken(keywords[id], id);
                } else {
                    addToken("Ident", id);
                }
                continue;
            }

            if (isdigit(ch)) {
                string num = readNumber();
                addToken("IntConst", num);
                continue;
            }

            if (ch == '+') {
                addToken("'+'", "+");
                advance();
                continue;
            }

            if (ch == '-') {
                addToken("'-'", "-");
                advance();
                continue;
            }

            if (ch == '*') {
                addToken("'*'", "*");
                advance();
                continue;
            }

            if (ch == '/') {
                addToken("'/'", "/");
                advance();
                continue;
            }

            if (ch == '%') {
                addToken("'%'", "%");
                advance();
                continue;
            }

            if (ch == '<') {
                advance();
                if (currentChar() == '=') {
                    addToken("'<='", "<=");
                    advance();
                } else {
                    addToken("'<'", "<");
                }
                continue;
            }

            if (ch == '>') {
                advance();
                if (currentChar() == '=') {
                    addToken("'>='", ">=");
                    advance();
                } else {
                    addToken("'>'", ">");
                }
                continue;
            }

            if (ch == '=') {
                advance();
                if (currentChar() == '=') {
                    addToken("'=='", "==");
                    advance();
                } else {
                    addToken("'='", "=");
                }
                continue;
            }

            if (ch == '!') {
                advance();
                if (currentChar() == '=') {
                    addToken("'!='", "!=");
                    advance();
                } else {
                    addToken("'!'", "!");
                }
                continue;
            }

            if (ch == '&' && peekChar() == '&') {
                addToken("'&&'", "&&");
                advance();
                advance();
                continue;
            }

            if (ch == '|' && peekChar() == '|') {
                addToken("'||'", "||");
                advance();
                advance();
                continue;
            }

            if (ch == '(') {
                addToken("'('", "(");
                advance();
                continue;
            }

            if (ch == ')') {
                addToken("')'", ")");
                advance();
                continue;
            }

            if (ch == '{') {
                addToken("'{'", "{");
                advance();
                continue;
            }

            if (ch == '}') {
                addToken("'}'", "}");
                advance();
                continue;
            }

            if (ch == ';') {
                addToken("';'", ";");
                advance();
                continue;
            }

            if (ch == ',') {
                addToken("','", ",");
                advance();
                continue;
            }

            advance();
        }

        return tokens;
    }
};

// 语法分析器
class Parser {
private:
    vector<Token> tokens;
    int current;
    set<int> errorLines;  // 存储错误行号，使用set自动排序和去重
    bool hasError;

    // 当前token
    Token& currentToken() {
        if (current >= tokens.size()) {
            static Token eof = {-1, "EOF", "", tokens.empty() ? 1 : tokens.back().line};
            return eof;
        }
        return tokens[current];
    }

    // 查看下一个token
    Token& peekToken(int offset = 1) {
        int pos = current + offset;
        if (pos >= tokens.size()) {
            static Token eof = {-1, "EOF", "", tokens.empty() ? 1 : tokens.back().line};
            return eof;
        }
        return tokens[pos];
    }

    // 检查当前token类型
    bool check(const string& type) {
        return currentToken().type == type;
    }

    // 前进到下一个token
    void advance() {
        if (current < tokens.size()) {
            current++;
        }
    }

    // 匹配并消耗token
    bool match(const string& type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    // 期望某个token，如果不匹配则记录错误
    bool expect(const string& type) {
        if (check(type)) {
            advance();
            return true;
        }
        // 记录错误
        recordError();
        return false;
    }

    // 记录错误行号
    void recordError() {
        hasError = true;
        errorLines.insert(currentToken().line);
    }

    // 同步：跳过token直到遇到下一个可能的函数定义开始或文件结束
    void synchronizeToNextFunc() {
        // 跳过当前行剩余的token
        while (!check("';'") && !check("'}'") &&
               !check("'int'") && !check("'void'") &&
               currentToken().type != "EOF") {
            advance();
        }
        // 如果遇到分号，跳过它
        if (check("';'")) {
            advance();
        }
        // 如果遇到右大括号，跳过到下一个token
        if (check("'}'")) {
            advance();
        }
    }

    // 同步：跳过token直到遇到同步点
    void synchronize() {
        advance();
        while (currentToken().type != "EOF") {
            if (check("';'") || check("'}'") || check("'{'") ||
                check("'int'") || check("'void'") || check("'if'") ||
                check("'while'") || check("'return'")) {
                return;
            }
            advance();
        }
    }

    // CompUnit → FuncDef+
    bool parseCompUnit() {
        bool hasFunc = false;

        while (currentToken().type != "EOF") {
            // 尝试解析函数定义
            if (check("'int'") || check("'void'")) {
                hasFunc = true;
                if (!parseFuncDef()) {
                    // 解析失败，同步到下一个函数定义
                    synchronizeToNextFunc();
                }
            } else {
                // 不是函数定义的开始
                recordError();
                synchronizeToNextFunc();
            }
        }

        return hasFunc;
    }

    // FuncDef → ("int" | "void") ID "(" (Param ("," Param)*)? ")" Block
    bool parseFuncDef() {
        bool success = true;
        int startLine = currentToken().line; // 记录函数定义开始的行号

        // 返回类型
        if (!match("'int'") && !match("'void'")) {
            recordError();
            return false;
        }

        // 函数名
        if (!check("Ident")) {
            // 缺少函数名，使用开始行号
            errorLines.insert(startLine);
            hasError = true;
            return false;
        }
        advance(); // 消耗Ident

        // 检查是否是函数定义（有左括号）
        if (!check("'('")) {
            // 这不是函数定义，可能是全局变量声明
            errorLines.insert(startLine); // 使用开始行号
            hasError = true;
            success = false;
            // 跳过到下一个函数定义
            return false;
        }

        // 左括号
        if (!expect("'('")) {
            success = false;
        }

        // 参数列表
        if (check("'int'")) {
            parseParam();
            while (match("','")) {
                parseParam();
            }
        }

        // 右括号 - 即使缺少也继续解析
        if (!check("')'")) {
            recordError();
            success = false;
            // 尝试跳过直到找到 { 或其他同步点
            while (!check("'{'") && !check("'int'") && !check("'void'") && currentToken().type != "EOF") {
                advance();
            }
        } else {
            advance();
        }

        // 函数体 - 继续解析即使前面有错误
        if (!parseBlock()) {
            success = false;
        }

        return success;
    }

    // Param → "int" ID
    bool parseParam() {
        if (!expect("'int'")) {
            return false;
        }
        return expect("Ident");
    }

    // Block → "{" Stmt* "}"
    bool parseBlock() {
        bool success = true;
        bool foundFuncDef = false;

        if (!expect("'{'")) {
            success = false;
            // 尝试恢复
            while (!check("'{'") && !check("';'") && currentToken().type != "EOF") {
                advance();
            }
            if (check("'{'")) advance();
            else return false;
        }

        while (!check("'}'") && currentToken().type != "EOF") {
            // 检查是否遇到函数定义（在Block内不应该出现）
            if ((check("'int'") || check("'void'")) && peekToken().type == "Ident" && peekToken(2).type == "'('") {
                // 在Block内遇到函数定义，这是错误的
                recordError();
                success = false;
                foundFuncDef = true;
                // 不消耗这些token，让上层处理
                break;
            }

            // 记录当前位置，防止死循环
            int oldPos = current;
            parseStmt();

            // 如果parseStmt没有前进，强制前进以避免死循环
            if (current == oldPos) {
                recordError();
                advance();
            }
        }

        if (!foundFuncDef) {
            if (!expect("'}'")) {
                success = false;
            }
        }

        return success;
    }

    // Stmt → Block | ";" | Expr ";" | ID "=" Expr ";" | "int" ID "=" Expr ";"
    //      | "if" "(" Expr ")" Stmt ("else" Stmt)? | "while" "(" Expr ")" Stmt
    //      | "break" ";" | "continue" ";" | "return" Expr ";"
    bool parseStmt() {
        // Block
        if (check("'{'")) {
            return parseBlock();
        }

        // 空语句
        if (match("';'")) {
            return true;
        }

        // break;
        if (match("'break'")) {
            return expect("';'");
        }

        // continue;
        if (match("'continue'")) {
            return expect("';'");
        }

        // return Expr;
        if (match("'return'")) {
            parseExpr();
            return expect("';'");
        }

        // if
        if (match("'if'")) {
            bool success = true;
            if (!expect("'('")) {
                success = false;
                // 尝试跳过找到表达式或语句
            }
            parseExpr();
            if (!expect("')'")) success = false;
            parseStmt();
            if (match("'else'")) {
                parseStmt();
            }
            return success;
        }

        // while
        if (match("'while'")) {
            bool success = true;
            if (!expect("'('")) success = false;
            parseExpr();
            if (!expect("')'")) success = false;
            parseStmt();
            return success;
        }

        // 变量声明: int ID = Expr;
        if (match("'int'")) {
            bool success = true;
            if (!expect("Ident")) success = false;
            if (!expect("'='")) success = false;
            parseExpr();
            if (!expect("';'")) success = false;
            return success;
        }

        // 赋值或表达式语句: ID = Expr; 或 Expr;
        if (check("Ident")) {
            // 需要区分赋值和函数调用
            if (peekToken().type == "'='") {
                advance(); // 消耗Ident
                advance(); // 消耗=
                parseExpr();
                return expect("';'");
            } else {
                // 表达式语句
                parseExpr();
                return expect("';'");
            }
        }

        // 其他表达式语句或错误
        // 检查是否是表达式的开始
        if (check("IntConst") || check("'('") || check("'+'") ||
            check("'-'") || check("'!'")) {
            parseExpr();
            return expect("';'");
        }

        // 无法识别的语句，记录错误并跳过到下一个同步点
        recordError();
        // 跳过到分号或语句块结束
        while (!check("';'") && !check("'}'") && !check("'{'") &&
               currentToken().type != "EOF") {
            advance();
        }
        if (check("';'")) advance(); // 消耗分号
        return false;
    }

    // Expr → LOrExpr
    bool parseExpr() {
        return parseLOrExpr();
    }

    // LOrExpr → LAndExpr ("||" LAndExpr)*
    bool parseLOrExpr() {
        if (!parseLAndExpr()) return false;
        while (match("'||'")) {
            parseLAndExpr();
        }
        return true;
    }

    // LAndExpr → RelExpr ("&&" RelExpr)*
    bool parseLAndExpr() {
        if (!parseRelExpr()) return false;
        while (match("'&&'")) {
            parseRelExpr();
        }
        return true;
    }

    // RelExpr → AddExpr (("<" | ">" | "<=" | ">=" | "==" | "!=") AddExpr)*
    bool parseRelExpr() {
        if (!parseAddExpr()) return false;
        while (check("'<'") || check("'>'") || check("'<='") ||
               check("'>='") || check("'=='") || check("'!='")) {
            advance();
            parseAddExpr();
        }
        return true;
    }

    // AddExpr → MulExpr (("+" | "-") MulExpr)*
    bool parseAddExpr() {
        if (!parseMulExpr()) return false;
        while (check("'+'") || check("'-'")) {
            advance();
            parseMulExpr();
        }
        return true;
    }

    // MulExpr → UnaryExpr (("*" | "/" | "%") UnaryExpr)*
    bool parseMulExpr() {
        if (!parseUnaryExpr()) return false;
        while (check("'*'") || check("'/'") || check("'%'")) {
            advance();
            parseUnaryExpr();
        }
        return true;
    }

    // UnaryExpr → PrimaryExpr | ("+" | "-" | "!") UnaryExpr
    bool parseUnaryExpr() {
        if (check("'+'") || check("'-'") || check("'!'")) {
            advance();
            return parseUnaryExpr();
        }
        return parsePrimaryExpr();
    }

    // PrimaryExpr → ID | NUMBER | "(" Expr ")" | ID "(" (Expr ("," Expr)*)? ")"
    bool parsePrimaryExpr() {
        // NUMBER
        if (match("IntConst")) {
            return true;
        }

        // 括号表达式
        if (match("'('")) {
            parseExpr();
            return expect("')'");
        }

        // ID 或函数调用
        if (match("Ident")) {
            // 函数调用
            if (match("'('")) {
                bool success = true;
                // 参数列表
                if (!check("')'") && !check("','")) {
                    if (!parseExpr()) success = false;
                    while (match("','")) {
                        if (!parseExpr()) success = false;
                    }
                } else if (check("','")) {
                    // 缺少第一个参数
                    recordError();
                    success = false;
                    advance(); // 跳过逗号
                    if (!check("')'")) {
                        parseExpr();
                        while (match("','")) {
                            parseExpr();
                        }
                    }
                }
                if (!expect("')'")) success = false;
                return success;
            }
            return true;
        }

        // 错误
        recordError();
        // 尝试跳过到下一个有效token
        if (!check("';'") && !check("')'") && !check("','") && !check("'}'")) {
            advance();
        }
        return false;
    }

public:
    Parser(const vector<Token>& toks) : tokens(toks), current(0), hasError(false) {}

    void parse() {
        parseCompUnit();
    }

    void printResult() {
        if (!hasError) {
            cout << "accept" << endl;
        } else {
            cout << "reject" << endl;
            for (int line : errorLines) {
                cout << line << endl;
            }
        }
    }
};

int main() {
    string line;
    string source;

    // 读取输入
    while (getline(cin, line)) {
        source += line + "\n";
    }

    // 词法分析
    Lexer lexer(source);
    vector<Token> tokens = lexer.tokenize();

    // 语法分析
    Parser parser(tokens);
    parser.parse();
    parser.printResult();

    return 0;
}
