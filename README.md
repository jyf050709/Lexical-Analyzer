# ToyC 词法分析器

这是一个手写的 ToyC 语言词法分析器，用于识别 ToyC 源代码中的所有词法单元（tokens）。

## 功能特性

- 识别所有 ToyC 关键字：int, void, if, else, while, break, continue, return
- 识别运算符：+, -, *, /, %, <, >, <=, >=, ==, !=, &&, ||, !, =
- 识别分隔符：(, ), {, }, ;, ,
- 识别标识符（符合 C 语言规范）
- 识别整数常量
- 正确处理单行注释 (//) 和多行注释 (/* */)
- 忽略空白字符

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
g++ -o compiler lexer.cpp -std=c++11
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

每行一个 token，格式为：

```
<序号>:<类型>:"<内容>"
```

例如：
```
0:'int':"int"
1:Ident:"main"
2:'(':"("
```

## 实现说明

本词法分析器完全手写实现，未使用任何自动生成工具（如 Flex/Lex）。采用状态机方法逐字符扫描源代码，根据字符特征识别不同类型的 token。
