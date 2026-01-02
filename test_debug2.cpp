// 模拟优化后的AST
// 原始：int sum = 0; int i = 0; while (i < 4) { sum = sum + i; i = i + 1; } return sum;
// 循环展开后：int sum = 0; int i = 4; sum = sum + 0; sum = sum + 1; sum = sum + 2; sum = sum + 3; return sum;
// 常量折叠后：int sum = 0; int i = 4; sum = 0; sum = 1; sum = 3; sum = 6; return sum;
// 这时候sum = 0的初始化和后续赋值中，sum在右边没有被使用
// 但是return sum中sum被使用
// 问题可能是：常量传播把return sum变成了return 6
// 如果是这样，sum就变成死变量了
#include <iostream>
using namespace std;
int main() {
    // 如果常量传播正确工作，sum = 6会被传播到return语句
    // return sum -> return 6
    // 这样sum就真的变成死变量了！
    cout << "常量传播可能把return sum变成return 6" << endl;
    cout << "这样sum就没有任何使用者了，被正确地删除" << endl;
    cout << "但是这会导致int sum = 0这个声明也被删除" << endl;
    cout << "然后sum = 6这个赋值语句还在，引用了未定义的sum" << endl;
    return 0;
}
