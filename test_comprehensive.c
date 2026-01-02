int abs(int x) {
    if (x < 0) {
        return 0 - x;
    }
    return x;
}

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

int fact(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * fact(n - 1);
}

int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

int gcd(int a, int b) {
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int test_operators(int x, int y) {
    int result = 0;
    if (x + y == 15) { result = result + 1; }
    if (x - y == 5) { result = result + 2; }
    if (x * y == 50) { result = result + 4; }
    if (x / y == 2) { result = result + 8; }
    if (x % y == 0) { result = result + 16; }
    if (x > y) { result = result + 32; }
    if (x >= y) { result = result + 64; }
    if (!(x < y)) { result = result + 128; }
    if (x != y) { result = result + 256; }
    return result;
}

int test_logic(int a, int b, int c) {
    if (a && b) {
        if (b || c) {
            return 1;
        }
    }
    if (!a || (b && c)) {
        return 2;
    }
    return 0;
}

int test_loop() {
    int sum = 0;
    int i = 1;
    while (i <= 10) {
        if (i == 5) {
            i = i + 1;
            continue;
        }
        if (i == 8) {
            break;
        }
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}

int test_nested() {
    int x = 1;
    {
        int x = 2;
        {
            int x = 3;
            x = x + 1;
        }
        x = x + 10;
    }
    return x;
}

int main() {
    int result = 0;

    result = result + abs(-5);
    result = result + max(3, 7);
    result = result + min(3, 7);
    result = result + fact(5);
    result = result + fib(7);
    result = result + gcd(48, 18);
    result = result + test_operators(10, 5);
    result = result + test_logic(1, 1, 0);
    result = result + test_loop();
    result = result + test_nested();

    return result;
}
