int compute(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    return compute(n - 1, acc + n);
}

void do_nothing(int n) {
    if (n <= 0) {
        return;
    }
    do_nothing(n - 1);
}

int wrapper(int x) {
    do_nothing(50);
    return compute(x, 0);
}

int main() {
    int r1 = wrapper(10);
    int r2 = compute(5, 100);
    return r1 + r2;
}
