int helper(int x) {
    return x * 2;
}

int tail_complex(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    return tail_complex(n - 1, acc + helper(n));
}

int main() {
    return tail_complex(5, 0);
}
