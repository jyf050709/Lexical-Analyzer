#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <map>

using namespace std;

struct Token {
    int index;
    string type;
    string value;
};

class Lexer {
private:
    string source;
    int position;
    int length;
    vector<Token> tokens;
    int tokenIndex;

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
        tokens.push_back(token);
    }

public:
    Lexer(const string& src) : source(src), position(0), length(src.length()), tokenIndex(0) {
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

    void printTokens() {
        for (const auto& token : tokens) {
            cout << token.index << ":" << token.type << ":\"" << token.value << "\"" << endl;
        }
    }
};

int main() {
    string line;
    string source;

    while (getline(cin, line)) {
        source += line + "\n";
    }

    Lexer lexer(source);
    lexer.tokenize();
    lexer.printTokens();

    return 0;
}
