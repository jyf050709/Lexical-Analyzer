# ToyC 语法分析器

这是一个手写的 ToyC 语言语法分析器，用于检查 ToyC 源代码的语法正确性。

## 功能特性

### 词法分析
- 识别所有 ToyC 关键字：int, void, if, else, while, break, continue, return
- 识别运算符：+, -, *, /, %, <, >, <=, >=, ==, !=, &&, ||, !, =
- 识别分隔符：(, ), {, }, ;, ,
- 识别标识符（符合 C 语言规范）
- 识别整数常量
- 正确处理单行注释 (//) 和多行注释 (/* */)
- 忽略空白字符
- 跟踪行号信息

### 语法分析
- 使用递归下降分析法实现
- 支持完整的 ToyC 文法
- 错误恢复机制，可以检测并报告多个语法错误
- 自动去重和排序错误行号

## 构建方法

### 使用 CMake (推荐)

```bash
cmake -B build
cmake --build build
```

### 使用 Makefile

```bash
make
```

### 直接编译

```bash
g++ -o compiler parser.cpp -std=c++11
```

## 使用方法

### 从标准输入读取

```bash
echo "int a = 1;" | ./compiler
```

### 从文件读取

```bash
./compiler < test.c
```

### 输出到文件

```bash
./compiler < test.c > output.tokens
```

## 输出格式

### 语法正确时
输出一行：
```
accept
```

### 语法错误时
第一行输出：
```
reject
```
后续行列出所有错误的行号（按升序排列，自动去重）：
```
reject
1
6
10
```

## 示例

### 正确的 ToyC 程序
```c
int main() {
    int result = 0;
    return result;
}
```
输出：
```
accept
```

### 有语法错误的程序
```c
int f(int x, int y {  // 缺少 )
    return x;
}
```
输出：
```
reject
1
```

## 实现说明

### 词法分析器 (Lexer)
- 完全手写实现，未使用任何自动生成工具（如 Flex/Lex）
- 采用状态机方法逐字符扫描源代码
- 根据字符特征识别不同类型的 token
- 在扫描过程中跟踪当前行号

### 语法分析器 (Parser)
- 完全手写实现，未使用任何自动生成工具（如 Yacc/Bison）
- 采用递归下降分析法
- 每个文法产生式对应一个解析函数
- 通过 LL(1) 预测分析选择正确的产生式
- 实现了左递归消除（将左递归改写为迭代）

### 错误恢复机制
- 遇到语法错误时记录错误行号
- 使用恐慌模式 (Panic Mode) 进行错误恢复
- 跳过token直到找到合适的同步点
- 同步点包括：分号、大括号、关键字等
- 能够在一次扫描中检测多个语法错误
- 自动去重和排序错误行号

## 支持的 ToyC 文法

```
CompUnit → FuncDef+
FuncDef → ("int" | "void") ID "(" (Param ("," Param)*)? ")" Block
Param → "int" ID
Block → "{" Stmt* "}"
Stmt → Block | ";" | Expr ";" | ID "=" Expr ";"
     | "int" ID "=" Expr ";"
     | "if" "(" Expr ")" Stmt ("else" Stmt)?
     | "while" "(" Expr ")" Stmt
     | "break" ";" | "continue" ";" | "return" Expr ";"
Expr → LOrExpr
LOrExpr → LAndExpr ("||" LAndExpr)*
LAndExpr → RelExpr ("&&" RelExpr)*
RelExpr → AddExpr (("<" | ">" | "<=" | ">=" | "==" | "!=") AddExpr)*
AddExpr → MulExpr (("+" | "-") MulExpr)*
MulExpr → UnaryExpr (("*" | "/" | "%") UnaryExpr)*
UnaryExpr → PrimaryExpr | ("+" | "-" | "!") UnaryExpr
PrimaryExpr → ID | NUMBER | "(" Expr ")" | ID "(" (Expr ("," Expr)*)? ")"
```

## 文件结构

- `parser.cpp` - 语法分析器主文件（包含词法分析器和语法分析器）
- `lexer.cpp` - 独立的词法分析器（用于词法分析作业）
- `CMakeLists.txt` - CMake 构建配置
- `Makefile` - Make 构建配置
- `test*.c` - 测试用例文件
- `README.md` - 本文档
