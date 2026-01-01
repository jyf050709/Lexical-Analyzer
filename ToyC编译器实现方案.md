# ToyC 编译器完整实现方案

## 一、项目概述

### 1.1 目标
实现一个 ToyC 语言编译器，能够将 ToyC 源代码编译为 RISC-V32 汇编代码。

### 1.2 输入输出
- **输入**: ToyC 源代码（标准输入）
- **输出**: RISC-V32 汇编代码（标准输出）
- **可选参数**: `-opt` 启用优化

### 1.3 编译器架构
采用经典的三段式架构：

```
源代码 → [前端] → AST → [中端] → IR → [后端] → RISC-V32 汇编
```

---

## 二、整体代码结构

```
compiler/
├── main.cpp           # 主程序入口
├── lexer.h/cpp        # 词法分析器
├── parser.h/cpp       # 语法分析器 + AST构建
├── ast.h              # AST节点定义
├── symbol.h/cpp       # 符号表管理
├── semantic.h/cpp     # 语义分析
├── ir.h/cpp           # 中间表示
├── codegen.h/cpp      # RISC-V32代码生成
├── regalloc.h/cpp     # 寄存器分配
└── optimizer.h/cpp    # 优化器（可选）
```

---

## 三、前端实现

### 3.1 词法分析器 (Lexer)

#### 3.1.1 Token 定义

```cpp
enum class TokenType {
    // 关键字
    INT, VOID, IF, ELSE, WHILE, BREAK, CONTINUE, RETURN,

    // 标识符和常量
    IDENT,      // 标识符
    INT_CONST,  // 整数常量

    // 运算符
    PLUS, MINUS, STAR, SLASH, MOD,     // + - * / %
    LT, GT, LE, GE, EQ, NE,            // < > <= >= == !=
    AND, OR, NOT,                       // && || !
    ASSIGN,                             // =

    // 分隔符
    LPAREN, RPAREN,    // ( )
    LBRACE, RBRACE,    // { }
    SEMICOLON,         // ;
    COMMA,             // ,

    // 特殊
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};
```

#### 3.1.2 词法分析改进
当前 lexer.cpp 已基本完善，需要的修改：
- 返回结构化的 Token 对象
- 添加列号支持（便于错误定位）

### 3.2 语法分析器 (Parser) + AST 构建

#### 3.2.1 AST 节点定义

```cpp
// 基础节点类型
enum class ASTNodeType {
    // 程序结构
    PROGRAM,
    FUNC_DEF,
    PARAM,

    // 语句
    BLOCK,
    VAR_DECL,
    ASSIGN_STMT,
    IF_STMT,
    WHILE_STMT,
    BREAK_STMT,
    CONTINUE_STMT,
    RETURN_STMT,
    EXPR_STMT,
    EMPTY_STMT,

    // 表达式
    BINARY_EXPR,
    UNARY_EXPR,
    CALL_EXPR,
    IDENT_EXPR,
    NUMBER_EXPR
};

// AST基类
class ASTNode {
public:
    ASTNodeType type;
    int line;
    virtual ~ASTNode() = default;
};

// 程序节点
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<FuncDefNode>> functions;
};

// 函数定义节点
class FuncDefNode : public ASTNode {
public:
    bool isVoid;              // 返回类型：true=void, false=int
    std::string name;         // 函数名
    std::vector<std::unique_ptr<ParamNode>> params;  // 参数列表
    std::unique_ptr<BlockNode> body;                  // 函数体
};

// 参数节点
class ParamNode : public ASTNode {
public:
    std::string name;
};

// 语句块节点
class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<StmtNode>> statements;
};

// 语句基类
class StmtNode : public ASTNode {};

// 变量声明节点
class VarDeclNode : public StmtNode {
public:
    std::string name;
    std::unique_ptr<ExprNode> init;  // 初始化表达式
};

// 赋值语句节点
class AssignStmtNode : public StmtNode {
public:
    std::string name;
    std::unique_ptr<ExprNode> value;
};

// if语句节点
class IfStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> thenStmt;
    std::unique_ptr<StmtNode> elseStmt;  // 可为nullptr
};

// while语句节点
class WhileStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> body;
};

// break语句节点
class BreakStmtNode : public StmtNode {};

// continue语句节点
class ContinueStmtNode : public StmtNode {};

// return语句节点
class ReturnStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> value;  // 可为nullptr（void函数）
};

// 表达式语句节点
class ExprStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expr;
};

// 表达式基类
class ExprNode : public ASTNode {};

// 二元表达式节点
class BinaryExprNode : public ExprNode {
public:
    std::string op;  // 运算符: + - * / % < > <= >= == != && ||
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
};

// 一元表达式节点
class UnaryExprNode : public ExprNode {
public:
    std::string op;  // 运算符: + - !
    std::unique_ptr<ExprNode> operand;
};

// 函数调用表达式节点
class CallExprNode : public ExprNode {
public:
    std::string funcName;
    std::vector<std::unique_ptr<ExprNode>> args;
};

// 标识符表达式节点
class IdentExprNode : public ExprNode {
public:
    std::string name;
};

// 数字常量表达式节点
class NumberExprNode : public ExprNode {
public:
    int value;
};
```

#### 3.2.2 语法分析器改进
基于现有 parser.cpp，需要修改为构建 AST：

```cpp
class Parser {
private:
    std::vector<Token> tokens;
    size_t current = 0;

    // 辅助方法
    Token& peek();
    Token& previous();
    bool check(TokenType type);
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);
    void advance();

    // 解析方法（返回AST节点）
    std::unique_ptr<ProgramNode> parseProgram();
    std::unique_ptr<FuncDefNode> parseFuncDef();
    std::unique_ptr<ParamNode> parseParam();
    std::unique_ptr<BlockNode> parseBlock();
    std::unique_ptr<StmtNode> parseStmt();
    std::unique_ptr<ExprNode> parseExpr();
    std::unique_ptr<ExprNode> parseLOrExpr();
    std::unique_ptr<ExprNode> parseLAndExpr();
    std::unique_ptr<ExprNode> parseRelExpr();
    std::unique_ptr<ExprNode> parseAddExpr();
    std::unique_ptr<ExprNode> parseMulExpr();
    std::unique_ptr<ExprNode> parseUnaryExpr();
    std::unique_ptr<ExprNode> parsePrimaryExpr();

public:
    Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ProgramNode> parse();
};
```

### 3.3 符号表

```cpp
// 符号类型
enum class SymbolKind {
    VARIABLE,
    FUNCTION,
    PARAMETER
};

// 符号信息
struct Symbol {
    std::string name;
    SymbolKind kind;
    bool isVoid;           // 函数返回类型
    int paramCount;        // 函数参数数量
    int stackOffset;       // 局部变量/参数在栈上的偏移
    int scopeLevel;        // 作用域层级
};

// 符号表
class SymbolTable {
private:
    std::vector<std::map<std::string, Symbol>> scopes;
    int currentLevel = 0;
    int currentStackOffset = 0;

public:
    void enterScope();
    void exitScope();

    bool define(const std::string& name, SymbolKind kind, bool isVoid = false, int paramCount = 0);
    Symbol* lookup(const std::string& name);
    Symbol* lookupCurrentScope(const std::string& name);

    int allocateStack(int size = 4);  // 分配栈空间，返回偏移
    int getCurrentStackOffset();
};
```

### 3.4 语义分析

```cpp
class SemanticAnalyzer {
private:
    SymbolTable symbolTable;
    std::string currentFunction;
    bool inLoop = false;
    std::vector<std::string> errors;

    void analyze(ProgramNode* node);
    void analyze(FuncDefNode* node);
    void analyze(StmtNode* node);
    void analyzeExpr(ExprNode* node);

    void error(int line, const std::string& message);

public:
    bool analyze(ProgramNode* ast);
    const std::vector<std::string>& getErrors();
};
```

语义检查项：
1. 变量使用前必须声明
2. 函数调用前必须定义
3. break/continue 只能在循环中使用
4. return 语句检查（int函数必须返回值，void函数不能返回值）
5. main 函数必须存在且返回 int
6. 变量/函数名不能重复定义（同一作用域）

---

## 四、中端实现 - 中间表示 (IR)

### 4.1 IR 设计（三地址码）

```cpp
enum class IROpType {
    // 算术运算
    ADD, SUB, MUL, DIV, MOD,
    NEG,                        // 取负

    // 关系运算
    LT, GT, LE, GE, EQ, NE,

    // 逻辑运算
    AND, OR, NOT,

    // 数据移动
    ASSIGN,                     // t1 = t2
    LOAD_IMM,                   // t1 = imm

    // 控制流
    LABEL,                      // label:
    JUMP,                       // goto label
    JUMP_IF_ZERO,              // if t1 == 0 goto label
    JUMP_IF_NOT_ZERO,          // if t1 != 0 goto label

    // 函数相关
    FUNC_BEGIN,                 // 函数开始
    FUNC_END,                   // 函数结束
    PARAM,                      // 函数参数（调用时）
    CALL,                       // 函数调用
    RETURN,                     // 返回

    // 内存操作
    LOAD,                       // t1 = [addr]
    STORE                       // [addr] = t1
};

struct IRInstruction {
    IROpType op;
    std::string result;     // 结果操作数
    std::string arg1;       // 第一个操作数
    std::string arg2;       // 第二个操作数
    std::string label;      // 标签名（用于跳转）
    int imm;                // 立即数
};

class IRGenerator {
private:
    std::vector<IRInstruction> instructions;
    SymbolTable* symbolTable;
    int tempCount = 0;
    int labelCount = 0;

    std::string newTemp();
    std::string newLabel();

    void emit(IROpType op, const std::string& result = "",
              const std::string& arg1 = "", const std::string& arg2 = "");

    void generate(ProgramNode* node);
    void generate(FuncDefNode* node);
    void generate(StmtNode* node);
    std::string generate(ExprNode* node);  // 返回存储结果的临时变量

public:
    IRGenerator(SymbolTable* st);
    std::vector<IRInstruction>& generate(ProgramNode* ast);
};
```

### 4.2 IR 生成示例

对于代码：
```c
int main() {
    int x = 5;
    int y = x + 3;
    return y;
}
```

生成的 IR：
```
FUNC_BEGIN main
    LOAD_IMM t0, 5
    STORE [sp-4], t0       // x = 5
    LOAD [sp-4], t1        // load x
    LOAD_IMM t2, 3
    ADD t3, t1, t2
    STORE [sp-8], t3       // y = x + 3
    LOAD [sp-8], t4        // load y
    RETURN t4
FUNC_END main
```

### 4.3 短路求值实现

对于 `a && b`：
```
    ; 计算 a
    JUMP_IF_ZERO t_a, label_false
    ; 计算 b
    JUMP_IF_ZERO t_b, label_false
    LOAD_IMM t_result, 1
    JUMP label_end
label_false:
    LOAD_IMM t_result, 0
label_end:
```

对于 `a || b`：
```
    ; 计算 a
    JUMP_IF_NOT_ZERO t_a, label_true
    ; 计算 b
    JUMP_IF_NOT_ZERO t_b, label_true
    LOAD_IMM t_result, 0
    JUMP label_end
label_true:
    LOAD_IMM t_result, 1
label_end:
```

---

## 五、后端实现 - RISC-V32 代码生成

### 5.1 RISC-V32 寄存器约定

```
寄存器    ABI名称    用途                      调用约定
------------------------------------------------------------
x0       zero      硬编码为0                  -
x1       ra        返回地址                   调用者保存
x2       sp        栈指针                     被调用者保存
x3       gp        全局指针                   -
x4       tp        线程指针                   -
x5-x7    t0-t2     临时寄存器                 调用者保存
x8       s0/fp     保存寄存器/帧指针          被调用者保存
x9       s1        保存寄存器                 被调用者保存
x10-x11  a0-a1     函数参数/返回值            调用者保存
x12-x17  a2-a7     函数参数                   调用者保存
x18-x27  s2-s11    保存寄存器                 被调用者保存
x28-x31  t3-t6     临时寄存器                 调用者保存
```

### 5.2 栈帧布局

```
高地址
    +-----------------+
    | 参数 n          | <- 参数8及以上通过栈传递
    | ...             |
    +-----------------+
    | 返回地址 (ra)   | <- 旧sp
    +-----------------+
    | 旧帧指针 (s0)   |
    +-----------------+
    | 保存的寄存器    |
    | (s1-s11等)      |
    +-----------------+
    | 局部变量        |
    +-----------------+
    | 临时空间        | <- sp
    +-----------------+
低地址
```

### 5.3 代码生成器

```cpp
class CodeGenerator {
private:
    std::vector<IRInstruction>& ir;
    std::ostringstream output;

    // 寄存器分配器
    RegisterAllocator regAlloc;

    // 当前函数信息
    std::string currentFunc;
    int stackSize = 0;

    // 辅助方法
    void emit(const std::string& instr);
    void emitLabel(const std::string& label);
    void emitComment(const std::string& comment);

    // 函数序言/尾声
    void emitPrologue(const std::string& funcName, int localSize);
    void emitEpilogue();

    // 指令生成
    void generateInstruction(const IRInstruction& instr);

    // 加载/存储辅助
    std::string loadOperand(const std::string& operand, const std::string& tempReg);
    void storeResult(const std::string& result, const std::string& reg);

public:
    CodeGenerator(std::vector<IRInstruction>& ir);
    std::string generate();
};
```

### 5.4 函数调用约定实现

#### 函数调用方（Caller）：
```cpp
void CodeGenerator::emitCall(const std::string& funcName,
                             const std::vector<std::string>& args) {
    // 1. 保存调用者保存寄存器（如果需要）
    // 2. 传递参数
    for (int i = 0; i < args.size() && i < 8; i++) {
        emit("mv a" + std::to_string(i) + ", " + args[i]);
    }
    // 超过8个参数使用栈传递
    for (int i = args.size() - 1; i >= 8; i--) {
        emit("addi sp, sp, -4");
        emit("sw " + args[i] + ", 0(sp)");
    }

    // 3. 调用函数
    emit("call " + funcName);

    // 4. 恢复栈（如果有栈上参数）
    if (args.size() > 8) {
        emit("addi sp, sp, " + std::to_string((args.size() - 8) * 4));
    }

    // 5. 返回值在 a0
}
```

#### 被调用方（Callee）：
```cpp
void CodeGenerator::emitPrologue(const std::string& funcName, int localSize) {
    emitLabel(funcName);
    emitLabel("prologue_" + funcName);

    // 计算栈帧大小（16字节对齐）
    int frameSize = ((localSize + 8 + 15) / 16) * 16;  // +8 for ra and s0

    // 分配栈空间
    emit("addi sp, sp, -" + std::to_string(frameSize));

    // 保存返回地址
    emit("sw ra, " + std::to_string(frameSize - 4) + "(sp)");

    // 保存帧指针
    emit("sw s0, " + std::to_string(frameSize - 8) + "(sp)");

    // 设置新帧指针
    emit("addi s0, sp, " + std::to_string(frameSize));
}

void CodeGenerator::emitEpilogue(int frameSize) {
    // 恢复返回地址
    emit("lw ra, " + std::to_string(frameSize - 4) + "(sp)");

    // 恢复帧指针
    emit("lw s0, " + std::to_string(frameSize - 8) + "(sp)");

    // 恢复栈指针
    emit("addi sp, sp, " + std::to_string(frameSize));

    // 返回
    emit("ret");
}
```

### 5.5 常用指令模板

```cpp
// 算术运算
void emitAdd(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("add " + rd + ", " + rs1 + ", " + rs2);
}

void emitAddi(const std::string& rd, const std::string& rs1, int imm) {
    emit("addi " + rd + ", " + rs1 + ", " + std::to_string(imm));
}

void emitSub(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("sub " + rd + ", " + rs1 + ", " + rs2);
}

void emitMul(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("mul " + rd + ", " + rs1 + ", " + rs2);
}

void emitDiv(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("div " + rd + ", " + rs1 + ", " + rs2);
}

void emitRem(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("rem " + rd + ", " + rs1 + ", " + rs2);
}

// 加载立即数
void emitLi(const std::string& rd, int imm) {
    emit("li " + rd + ", " + std::to_string(imm));
}

// 比较运算
void emitSlt(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("slt " + rd + ", " + rs1 + ", " + rs2);  // rd = (rs1 < rs2) ? 1 : 0
}

// 分支跳转
void emitBeqz(const std::string& rs, const std::string& label) {
    emit("beqz " + rs + ", " + label);
}

void emitBnez(const std::string& rs, const std::string& label) {
    emit("bnez " + rs + ", " + label);
}

void emitJ(const std::string& label) {
    emit("j " + label);
}

// 内存操作
void emitLw(const std::string& rd, int offset, const std::string& rs) {
    emit("lw " + rd + ", " + std::to_string(offset) + "(" + rs + ")");
}

void emitSw(const std::string& rs, int offset, const std::string& rd) {
    emit("sw " + rs + ", " + std::to_string(offset) + "(" + rd + ")");
}
```

### 5.6 关系运算实现

```cpp
// a < b
void emitLessThan(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("slt " + rd + ", " + rs1 + ", " + rs2);
}

// a > b  等价于 b < a
void emitGreaterThan(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("slt " + rd + ", " + rs2 + ", " + rs1);
}

// a <= b 等价于 !(a > b)
void emitLessEqual(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("slt " + rd + ", " + rs2 + ", " + rs1);  // rd = (rs2 < rs1)
    emit("xori " + rd + ", " + rd + ", 1");        // rd = !rd
}

// a >= b 等价于 !(a < b)
void emitGreaterEqual(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("slt " + rd + ", " + rs1 + ", " + rs2);  // rd = (rs1 < rs2)
    emit("xori " + rd + ", " + rd + ", 1");        // rd = !rd
}

// a == b
void emitEqual(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("sub " + rd + ", " + rs1 + ", " + rs2);  // rd = rs1 - rs2
    emit("seqz " + rd + ", " + rd);               // rd = (rd == 0) ? 1 : 0
}

// a != b
void emitNotEqual(const std::string& rd, const std::string& rs1, const std::string& rs2) {
    emit("sub " + rd + ", " + rs1 + ", " + rs2);  // rd = rs1 - rs2
    emit("snez " + rd + ", " + rd);               // rd = (rd != 0) ? 1 : 0
}
```

---

## 六、寄存器分配

### 6.1 线性扫描算法（推荐）

```cpp
struct LiveInterval {
    std::string varName;
    int start;          // 活跃区间开始位置
    int end;            // 活跃区间结束位置
    std::string reg;    // 分配的寄存器（空表示溢出到栈）
    int spillSlot;      // 溢出槽位
};

class RegisterAllocator {
private:
    // 可用寄存器
    std::vector<std::string> availableRegs = {
        "t0", "t1", "t2", "t3", "t4", "t5", "t6",
        "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"
    };

    std::map<std::string, LiveInterval> intervals;
    std::set<std::string> freeRegs;
    std::vector<LiveInterval*> active;
    int spillSlotCount = 0;

    void computeLiveness(const std::vector<IRInstruction>& ir);
    void expireOldIntervals(int position);
    void spillAtInterval(LiveInterval* interval);

public:
    void allocate(std::vector<IRInstruction>& ir);
    std::string getRegister(const std::string& varName);
    int getSpillSlot(const std::string& varName);
    bool isSpilled(const std::string& varName);
    int getTotalSpillSlots();
};

void RegisterAllocator::allocate(std::vector<IRInstruction>& ir) {
    // 1. 计算活跃区间
    computeLiveness(ir);

    // 2. 按开始位置排序
    std::vector<LiveInterval*> sortedIntervals;
    for (auto& [name, interval] : intervals) {
        sortedIntervals.push_back(&interval);
    }
    std::sort(sortedIntervals.begin(), sortedIntervals.end(),
              [](LiveInterval* a, LiveInterval* b) { return a->start < b->start; });

    // 3. 初始化空闲寄存器
    for (const auto& reg : availableRegs) {
        freeRegs.insert(reg);
    }

    // 4. 线性扫描
    for (auto* interval : sortedIntervals) {
        expireOldIntervals(interval->start);

        if (freeRegs.empty()) {
            // 没有空闲寄存器，需要溢出
            spillAtInterval(interval);
        } else {
            // 分配一个空闲寄存器
            auto it = freeRegs.begin();
            interval->reg = *it;
            freeRegs.erase(it);
            active.push_back(interval);
        }
    }
}

void RegisterAllocator::expireOldIntervals(int position) {
    active.erase(
        std::remove_if(active.begin(), active.end(),
            [this, position](LiveInterval* interval) {
                if (interval->end < position) {
                    freeRegs.insert(interval->reg);
                    return true;
                }
                return false;
            }),
        active.end()
    );
}

void RegisterAllocator::spillAtInterval(LiveInterval* interval) {
    // 找到结束最晚的活跃区间
    auto spill = std::max_element(active.begin(), active.end(),
        [](LiveInterval* a, LiveInterval* b) { return a->end < b->end; });

    if (spill != active.end() && (*spill)->end > interval->end) {
        // 溢出结束最晚的区间
        interval->reg = (*spill)->reg;
        (*spill)->reg = "";
        (*spill)->spillSlot = spillSlotCount++;
        active.erase(spill);
        active.push_back(interval);
    } else {
        // 溢出当前区间
        interval->reg = "";
        interval->spillSlot = spillSlotCount++;
    }
}
```

### 6.2 简单寄存器分配（备选方案）

如果线性扫描太复杂，可以使用简单的临时变量到寄存器的映射：

```cpp
class SimpleRegisterAllocator {
private:
    std::map<std::string, std::string> regMap;
    std::map<std::string, int> spillMap;
    std::queue<std::string> freeRegs;
    int spillOffset = 0;

public:
    SimpleRegisterAllocator() {
        // 使用 t0-t6 作为临时寄存器
        for (int i = 0; i <= 6; i++) {
            freeRegs.push("t" + std::to_string(i));
        }
    }

    std::string allocate(const std::string& temp) {
        if (regMap.count(temp)) {
            return regMap[temp];
        }

        if (!freeRegs.empty()) {
            std::string reg = freeRegs.front();
            freeRegs.pop();
            regMap[temp] = reg;
            return reg;
        }

        // 溢出到栈
        spillOffset -= 4;
        spillMap[temp] = spillOffset;
        return "";  // 表示需要从栈加载
    }

    void release(const std::string& temp) {
        if (regMap.count(temp)) {
            freeRegs.push(regMap[temp]);
            regMap.erase(temp);
        }
    }

    int getSpillOffset(const std::string& temp) {
        return spillMap[temp];
    }
};
```

---

## 七、优化（可选）

### 7.1 常量折叠

在 AST 层面或 IR 生成时进行：

```cpp
int foldConstant(const std::string& op, int left, int right) {
    if (op == "+") return left + right;
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/") return left / right;
    if (op == "%") return left % right;
    if (op == "<") return left < right ? 1 : 0;
    if (op == ">") return left > right ? 1 : 0;
    if (op == "<=") return left <= right ? 1 : 0;
    if (op == ">=") return left >= right ? 1 : 0;
    if (op == "==") return left == right ? 1 : 0;
    if (op == "!=") return left != right ? 1 : 0;
    return 0;
}
```

### 7.2 死代码消除

在 IR 层面：
1. 标记所有使用到的变量
2. 删除结果未被使用的指令（除了有副作用的指令）

### 7.3 代码生成优化

```cpp
// 使用移位代替乘2的幂
if (isPowerOfTwo(imm)) {
    int shift = log2(imm);
    emit("slli " + rd + ", " + rs + ", " + std::to_string(shift));
} else {
    emit("li t0, " + std::to_string(imm));
    emit("mul " + rd + ", " + rs + ", t0");
}
```

---

## 八、完整编译流程

### 8.1 main.cpp

```cpp
#include <iostream>
#include <string>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "codegen.h"

int main(int argc, char* argv[]) {
    bool optimize = false;

    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-opt") {
            optimize = true;
        }
    }

    // 读取源代码
    std::string source;
    std::string line;
    while (std::getline(std::cin, line)) {
        source += line + "\n";
    }

    // 1. 词法分析
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // 2. 语法分析，构建AST
    Parser parser(tokens);
    auto ast = parser.parse();

    // 3. 语义分析
    SymbolTable symbolTable;
    SemanticAnalyzer semantic(&symbolTable);
    if (!semantic.analyze(ast.get())) {
        // 语义错误
        return 1;
    }

    // 4. 生成IR
    IRGenerator irGen(&symbolTable);
    auto ir = irGen.generate(ast.get());

    // 5. 优化（可选）
    if (optimize) {
        Optimizer optimizer;
        optimizer.optimize(ir);
    }

    // 6. 代码生成
    CodeGenerator codeGen(ir);
    std::string assembly = codeGen.generate();

    // 7. 输出汇编代码
    std::cout << assembly;

    return 0;
}
```

### 8.2 示例输出

输入：
```c
int main() {
    return 1;
}
```

输出：
```asm
.text

.globl main
main:
prologue_main:
label0:
        li t0, 1
        mv a0, t0
        ret
```

---

## 九、测试建议

### 9.1 单元测试

1. **词法分析器测试**: 验证各类 token 识别
2. **语法分析器测试**: 验证 AST 构建正确性
3. **语义分析测试**: 验证错误检测
4. **IR生成测试**: 验证中间代码正确性
5. **代码生成测试**: 验证汇编输出

### 9.2 集成测试用例

```c
// 1. 基本返回
int main() { return 42; }

// 2. 算术运算
int main() { return 1 + 2 * 3; }

// 3. 变量声明和使用
int main() {
    int x = 10;
    int y = x + 5;
    return y;
}

// 4. 条件语句
int main() {
    int x = 5;
    if (x > 0) {
        return 1;
    } else {
        return 0;
    }
}

// 5. 循环语句
int main() {
    int sum = 0;
    int i = 1;
    while (i <= 10) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}

// 6. 函数调用
int add(int a, int b) {
    return a + b;
}
int main() {
    return add(3, 5);
}

// 7. 递归
int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}
int main() {
    return fib(10);
}
```

### 9.3 使用 RISC-V 模拟器测试

```bash
# 编译生成汇编
./compiler < test.c > test.s

# 使用 riscv32 工具链汇编
riscv32-unknown-elf-as test.s -o test.o
riscv32-unknown-elf-ld test.o -o test

# 使用模拟器运行
spike pk test
# 或使用 qemu
qemu-riscv32 test
echo $?  # 查看返回值
```

---

## 十、开发计划

### 阶段一：基础框架
1. 重构词法分析器，添加更好的 Token 结构
2. 修改语法分析器，构建 AST
3. 实现符号表

### 阶段二：语义分析
1. 实现作用域管理
2. 添加类型检查
3. 验证语义约束

### 阶段三：IR 生成
1. 设计 IR 指令集
2. 实现表达式的 IR 生成
3. 实现语句的 IR 生成
4. 处理控制流（if/while/break/continue）

### 阶段四：代码生成
1. 实现简单的寄存器分配
2. 生成函数序言/尾声
3. 实现各类指令的翻译
4. 处理函数调用约定

### 阶段五：测试与调试
1. 编写测试用例
2. 使用模拟器验证
3. 修复问题

### 阶段六：优化（可选）
1. 实现常量折叠
2. 实现死代码消除
3. 改进寄存器分配

---

## 十一、注意事项

1. **32位限制**: 所有运算都是32位有符号整数
2. **栈对齐**: RISC-V 要求栈16字节对齐
3. **返回值**: main 函数返回值会被截断到 0-255
4. **调用约定**: 严格遵循 RISC-V 调用约定
5. **短路求值**: `&&` 和 `||` 必须实现短路求值
6. **作用域**: 内层作用域变量会遮蔽外层同名变量
