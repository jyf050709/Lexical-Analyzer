int not_tail_add(int n) {
    if (n <= 0) {
        return 0;
    }
    return not_tail_add(n - 1) + n;
}

int not_tail_mult(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * not_tail_mult(n - 1);
}

int mixed(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    if (n % 2 == 0) {
        return mixed(n - 1, acc + n);
    }
    return 1 + mixed(n - 1, acc);
}

int tail_only(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    return tail_only(n - 1, acc + n);
}

int main() {
    int r1 = not_tail_add(10);
    int r2 = not_tail_mult(5);
    int r3 = mixed(10, 0);
    int r4 = tail_only(10, 0);
    return r1 + r2 + r3 + r4;
}
