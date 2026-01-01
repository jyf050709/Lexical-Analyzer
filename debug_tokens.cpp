#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "parser.cpp"

int main() {
    string line;
    string source;

    while (getline(cin, line)) {
        source += line + "\n";
    }

    Lexer lexer(source);
    vector<Token> tokens = lexer.tokenize();

    // 打印最后几个token的行号
    int start = max(0, (int)tokens.size() - 5);
    for (int i = start; i < tokens.size(); i++) {
        cout << "Token[" << i << "]: " << tokens[i].type << " at line " << tokens[i].line << endl;
    }

    return 0;
}
