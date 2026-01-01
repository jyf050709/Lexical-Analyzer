# f07_scope_shadow_plus 测试用例排查记录

## 错误信息
```
f07_scope_shadow_plus  编译器异常  -  0.00
Error: Parse error
```

## ToyC 语言文法（参考）
```
CompUnit → FuncDef+
Stmt → Block | ";" | Expr ";" | ID "=" Expr ";"
     | "int" ID "=" Expr ";"
     | "if" "(" Expr ")" Stmt ("else" Stmt)?
     | "while" "(" Expr ")" Stmt
     | "break" ";" | "continue" ";" | "return" Expr ";"
Block → "{" Stmt* "}"
FuncDef → ("int" | "void") ID "(" (Param ("," Param)*)? ")" Block
Param → "int" ID
Expr → LOrExpr
LOrExpr → LAndExpr | LOrExpr "||" LAndExpr
LAndExpr → RelExpr | LAndExpr "&&" RelExpr
RelExpr → AddExpr | RelExpr ("<"|">"|"<="|">="|"=="|"!=") AddExpr
AddExpr → MulExpr | AddExpr ("+"|"-") MulExpr
MulExpr → UnaryExpr | MulExpr ("*"|"/"|"%") UnaryExpr
UnaryExpr → PrimaryExpr | ("+"|"-"|"!") UnaryExpr
PrimaryExpr → ID | NUMBER | "(" Expr ")" | ID "(" (Expr ("," Expr)*)? ")"
```

**注意**: 文法中无全局变量，但已添加支持

---

## 已应用的修复

1. **链式赋值表达式** - 支持 `y = x = x + 1`
2. **全局变量支持** - 已由其他agent添加

---

## 已确认会导致 Parse error 的语法

| 语法 | 示例 | 错误类型 |
|------|------|----------|
| 逗号表达式 | `(1, 2)` | Parse error |
| 三元运算符 | `a ? b : c` | Parse error |
| 位运算 AND | `1 & 2` | Parse error |
| 位运算 OR | `1 \| 2` | Parse error |
| 位运算 XOR | `1 ^ 2` | Parse error |
| 位运算 NOT | `~1` | Parse error |
| 数组声明 | `int arr[10]` | Parse error |
| 十六进制 | `0x10` | Parse error |
| 指针声明 | `int *p` | Parse error |
| do-while | `do {} while(0)` | Parse error |
| for循环 | `for(...)` | Parse error |
| 后缀自增 | `x++` | Expected expression |
| 左移 | `1 << 2` | Expected expression |
| 右移 | `4 >> 1` | Expected expression |
| sizeof | `sizeof(int)` | Expected expression |
| 类型转换 | `(int)1` | Expected expression |
| 字符字面量 | `'a'` | Undefined variable |
| const | `const int x` | Expected type |

## 已修复并验证通过的功能 ✓

- **全局变量**: `int x = 1; int main() { return x; }` ✓
- **全局变量+局部遮蔽**: `int x = 1; int main() { int x = 2; return x; }` ✓
- **全局变量+加法**: `int x = 1; int main() { x = x + 1; return x; }` ✓

---

## 已确认正常工作的 scope_shadow_plus 模式 ✓

### 所有作用域遮蔽+加法模式均已测试通过：

```c
// 基本遮蔽+加法
int x = 1; { int x = x + 1; }  ✓

// 多层嵌套遮蔽
int x = 1; { int x = 2; { int x = x + x; } }  ✓

// 参数遮蔽+加法
int f(int x) { int x = x + 1; return x; }  ✓

// if中遮蔽+加法
if (x) { int x = 2; return x + 1; }  ✓

// while中遮蔽
while (x) { int x = 0; }  ✓

// 逻辑表达式+遮蔽
int x = 1; { int x = x || 1; }  ✓
int y = x + 1 || x;  ✓

// 链式赋值+遮蔽
y = x = x + 1;  ✓

// 赋值在子表达式
return x + (x = 2);  ✓

// 复杂表达式
int x = (x + 1) * 2;  ✓
```

---

## 结论

**所有与 "scope_shadow_plus" 直接相关的语法模式均已测试通过。**
**全局变量支持已添加并测试通过。**

### 已验证的完整测试列表：

| 测试模式 | 代码示例 | 结果 |
|----------|----------|------|
| 基本作用域遮蔽+加法 | `int x=1; { int x=x+1; }` | ✓ |
| 多层嵌套遮蔽 | `{ int x=2; { int x=x+x; } }` | ✓ |
| 参数遮蔽+加法 | `int f(int x) { int x=x+1; }` | ✓ |
| if中遮蔽+加法 | `if(x) { int x=x+2; return x; }` | ✓ |
| while中遮蔽 | `while(x) { int x=0; }` | ✓ |
| 链式赋值 | `y = x = x + 1;` | ✓ |
| 全局变量+遮蔽 | `int x=1; int main() { int x=2; }` | ✓ |
| 全局变量+加法 | `int x=1; int main() { x=x+1; }` | ✓ |
| 逻辑表达式 | `x \|\| 1`, `x && 1` | ✓ |
| 复杂表达式 | `1 + 2 * 3` | ✓ |
| 表达式语句 | `x + 1;` | ✓ |
| 函数调用语句 | `f(x+1);` | ✓ |
| void return | `void f() { return; }` | ✓ |

### 错误可能来自不支持的语法：

1. **逗号表达式** `(a, b)` - Parse error
2. **位运算符** `&`, `|`, `^` - Parse error
3. **三元运算符** `?:` - Parse error
4. **do-while** 循环 - Parse error
5. **for循环** - Parse error

---

## 测试命令
```bash
echo "代码" | "c:\Users\Administrator\Desktop\词法分析器\compiler.exe" 2>&1
```
