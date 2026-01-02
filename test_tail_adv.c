int tail_gcd(int a, int b) {
    if (b == 0) {
        return a;
    }
    return tail_gcd(b, a % b);
}

int tail_pow(int base, int exp, int acc) {
    if (exp <= 0) {
        return acc;
    }
    if (exp % 2 == 0) {
        return tail_pow(base * base, exp / 2, acc);
    }
    return tail_pow(base, exp - 1, acc * base);
}

int countdown(int n) {
    if (n <= 0) {
        return 0;
    }
    return countdown(n - 1);
}

int deep_tail(int a, int b, int c, int d, int count) {
    if (count <= 0) {
        return a + b + c + d;
    }
    return deep_tail(b, c, d, a + 1, count - 1);
}

int collatz(int n, int steps) {
    if (n <= 1) {
        return steps;
    }
    if (n % 2 == 0) {
        return collatz(n / 2, steps + 1);
    }
    return collatz(n * 3 + 1, steps + 1);
}

int main() {
    int r1 = tail_gcd(48, 18);
    int r2 = tail_gcd(100, 35);
    int r3 = tail_pow(2, 10, 1);
    int r4 = tail_pow(3, 5, 1);
    int r5 = countdown(500);
    int r6 = deep_tail(1, 2, 3, 4, 50);
    int r7 = collatz(27, 0);

    return r1 + r2 + r3 + r4 + r5 + r6 + r7;
}
