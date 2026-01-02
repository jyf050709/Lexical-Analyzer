// 简单尾递归测试

int sum(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    return sum(n - 1, acc + n);
}

int main() {
    return sum(5, 0);  // 应该返回 15
}
