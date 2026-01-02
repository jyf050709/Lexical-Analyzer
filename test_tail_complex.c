int tail_sum(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    return tail_sum(n - 1, acc + n);
}

int tail_swap(int a, int b, int count) {
    if (count <= 0) {
        return a * 10 + b;
    }
    return tail_swap(b, a, count - 1);
}

int tail_mult(int n, int acc) {
    if (n <= 1) {
        return acc;
    }
    return tail_mult(n - 1, acc * n);
}

int tail_fib(int n, int a, int b) {
    if (n <= 0) {
        return a;
    }
    return tail_fib(n - 1, b, a + b);
}

int main() {
    int r1 = tail_sum(10, 0);
    int r2 = tail_swap(3, 7, 5);
    int r3 = tail_mult(5, 1);
    int r4 = tail_fib(10, 0, 1);
    return r1 + r2 + r3 + r4;
}
