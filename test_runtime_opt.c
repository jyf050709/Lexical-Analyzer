// 测试运行时优化方案
// 1. 叶函数优化测试
int leafFunc(int a, int b) {
    int x = a + b;
    int y = a - b;
    int z = x * y;
    return z;
}

// 2. 非叶函数测试
int nonLeafFunc(int n) {
    if (n <= 1) {
        return n;
    }
    return leafFunc(n, n - 1);
}

// 3. 局部变量寄存器化测试（多次使用的变量）
int loopTest(int n) {
    int sum = 0;
    int i = 0;
    while (i < n) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}

// 4. 复杂表达式测试
int exprTest(int a, int b, int c) {
    int result = (a + b) * (c - a) + b * c;
    return result;
}

int main() {
    int r1 = leafFunc(10, 5);
    int r2 = nonLeafFunc(10);
    int r3 = loopTest(100);
    int r4 = exprTest(3, 4, 5);
    return r1 + r2 + r3 + r4;
}
