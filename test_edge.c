int neg_test(int n, int acc) {
    if (n >= 0) {
        return acc;
    }
    return neg_test(n + 1, acc - n);
}

int zero_base(int n) {
    if (n == 0) {
        return 100;
    }
    if (n < 0) {
        return zero_base(n + 1);
    }
    return zero_base(n - 1);
}

int single_param(int n) {
    if (n <= 0) {
        return 0;
    }
    return single_param(n - 1);
}

int six_params(int a, int b, int c, int d, int e, int f) {
    if (a <= 0) {
        return b + c + d + e + f;
    }
    return six_params(a - 1, c, d, e, f, b + 1);
}

int ret_zero(int n) {
    if (n <= 0) {
        return 0;
    }
    return ret_zero(n - 1);
}

int boundary(int n, int acc) {
    if (n == 0) {
        return acc;
    }
    if (n == 1) {
        return acc + 1;
    }
    return boundary(n - 2, acc + n);
}

int main() {
    int r1 = neg_test(-5, 0);
    int r2 = zero_base(10);
    int r3 = zero_base(-10);
    int r4 = single_param(100);
    int r5 = six_params(5, 1, 2, 3, 4, 5);
    int r6 = ret_zero(50);
    int r7 = boundary(10, 0);
    int r8 = boundary(11, 0);

    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}
