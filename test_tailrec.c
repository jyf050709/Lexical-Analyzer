// 测试尾递归优化

// 1. 简单尾递归：阶乘（累加器模式）
int factorial_tail(int n, int acc) {
    if (n <= 1) {
        return acc;
    }
    return factorial_tail(n - 1, n * acc);
}

int factorial(int n) {
    return factorial_tail(n, 1);
}

// 2. 斐波那契（尾递归版本）
int fib_tail(int n, int a, int b) {
    if (n == 0) {
        return a;
    }
    if (n == 1) {
        return b;
    }
    return fib_tail(n - 1, b, a + b);
}

int fib(int n) {
    return fib_tail(n, 0, 1);
}

// 3. 求和（1到n）
int sum_tail(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    return sum_tail(n - 1, acc + n);
}

int sum(int n) {
    return sum_tail(n, 0);
}

// 4. GCD（欧几里得算法 - 天然的尾递归）
int gcd(int a, int b) {
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

// 主函数
int main() {
    int result = 0;

    // 测试阶乘
    result = result + factorial(5);  // 120

    // 测试斐波那契
    result = result + fib(10);  // 55

    // 测试求和
    result = result + sum(10);  // 55

    // 测试GCD
    result = result + gcd(48, 18);  // 6

    return result;  // 应该返回 236
}
