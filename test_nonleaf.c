// 非叶函数测试
int add(int a, int b) {
    return a + b;
}

int caller(int x) {
    return add(x, x + 1);
}

int main() {
    int x = caller(5);
    return x;
}
