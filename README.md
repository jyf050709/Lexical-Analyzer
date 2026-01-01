# ToyC 编译器

一个完整的 ToyC 语言编译器，能够将 ToyC 源代码编译为 RISC-V32 汇编代码。

## 功能特性

### 词法分析
- 识别所有 ToyC 关键字：`int`, `void`, `if`, `else`, `while`, `break`, `continue`, `return`
- 识别运算符：`+`, `-`, `*`, `/`, `%`, `<`, `>`, `<=`, `>=`, `==`, `!=`, `&&`, `||`, `!`, `=`
- 识别分隔符：`(`, `)`, `{`, `}`, `;`, `,`
- 识别标识符和整数常量
- 正确处理单行注释 (`//`) 和多行注释 (`/* */`)

### 语法分析
- 递归下降解析器
- 构建抽象语法树 (AST)
- 支持完整的 ToyC 文法

### 代码生成
- 生成 RISC-V32 汇编代码
- 支持函数调用约定
- 栈式存储管理
- 短路求值 (`&&`, `||`)

## 支持的语法

- 函数定义和调用（支持递归）
- 变量声明和赋值
- 算术运算：`+` `-` `*` `/` `%`
- 关系运算：`<` `>` `<=` `>=` `==` `!=`
- 逻辑运算：`&&` `||` `!`（短路求值）
- 一元运算：`+` `-` `!`
- if-else 条件语句
- while 循环
- break / continue
- return 语句

## 构建方法

### 使用 Make

```bash
make
```

### 直接编译

```bash
g++ -std=c++17 -o compiler compiler.cpp
```

## 使用方法

### 从标准输入读取

```bash
echo "int main() { return 42; }" | ./compiler
```

### 从文件读取

```bash
./compiler < test.c
```

### 输出到文件

```bash
./compiler < test.c > output.s
```

## 示例

### 输入
```c
int main() {
    return 1;
}
```

### 输出
```asm
.text

.globl main
main:
prologue_main:
    addi sp, sp, -144
    sw ra, 140(sp)
    sw s0, 136(sp)
    addi s0, sp, 144
    li t0, 1
    mv a0, t0
    lw ra, 140(sp)
    lw s0, 136(sp)
    addi sp, sp, 144
    ret
```

### 更多示例

#### 函数调用
```c
int add(int a, int b) {
    return a + b;
}
int main() {
    return add(3, 5);
}
```

#### 循环
```c
int main() {
    int sum = 0;
    int i = 1;
    while (i <= 10) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}
```

#### 递归
```c
int fib(int n) {
    if (n <= 1) { return n; }
    return fib(n - 1) + fib(n - 2);
}
int main() {
    return fib(10);
}
```

## ToyC 文法

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

- `compiler.cpp` - 完整的 ToyC 编译器实现
- `ToyC编译器实现方案.md` - 详细的实现方案文档
- `作业要求` - 作业要求说明
- `Makefile` - 构建配置
- `README.md` - 本文档
