#include <iostream>
#include <string>
#include <vector>
#include <memory>
using namespace std;

// 简化版测试
struct Stmt { virtual ~Stmt() = default; virtual string toString() = 0; };
struct VarDeclStmt : Stmt { string name; int val; VarDeclStmt(string n, int v) : name(n), val(v) {} string toString() override { return "int " + name + " = " + to_string(val) + ";"; } };
struct AssignStmt : Stmt { string name; string expr; AssignStmt(string n, string e) : name(n), expr(e) {} string toString() override { return name + " = " + expr + ";"; } };
struct WhileStmt : Stmt { string toString() override { return "while(...)"; } };
struct ReturnStmt : Stmt { string name; ReturnStmt(string n) : name(n) {} string toString() override { return "return " + name + ";"; } };

int main() {
    vector<unique_ptr<Stmt>> stmts;
    stmts.push_back(make_unique<VarDeclStmt>("sum", 0));
    stmts.push_back(make_unique<VarDeclStmt>("i", 0));
    stmts.push_back(make_unique<WhileStmt>());
    stmts.push_back(make_unique<ReturnStmt>("sum"));
    
    cout << "Before unroll:" << endl;
    for (size_t i = 0; i < stmts.size(); i++) {
        cout << "  [" << i << "] " << stmts[i]->toString() << endl;
    }
    
    // 模拟循环展开
    size_t initStmtIdx = 1;  // int i = 0
    size_t whileIdx = 2;     // while
    
    // 准备展开后的语句
    vector<unique_ptr<Stmt>> unrolled;
    unrolled.push_back(make_unique<VarDeclStmt>("i", 4));  // final value
    unrolled.push_back(make_unique<AssignStmt>("sum", "sum + 0"));
    unrolled.push_back(make_unique<AssignStmt>("sum", "sum + 1"));
    unrolled.push_back(make_unique<AssignStmt>("sum", "sum + 2"));
    unrolled.push_back(make_unique<AssignStmt>("sum", "sum + 3"));
    
    // 删除
    stmts.erase(stmts.begin() + initStmtIdx, stmts.begin() + whileIdx + 1);
    
    cout << "After erase:" << endl;
    for (size_t i = 0; i < stmts.size(); i++) {
        cout << "  [" << i << "] " << stmts[i]->toString() << endl;
    }
    
    // 插入
    for (size_t i = 0; i < unrolled.size(); i++) {
        stmts.insert(stmts.begin() + initStmtIdx + i, move(unrolled[i]));
    }
    
    cout << "After insert:" << endl;
    for (size_t i = 0; i < stmts.size(); i++) {
        cout << "  [" << i << "] " << stmts[i]->toString() << endl;
    }
    
    return 0;
}
