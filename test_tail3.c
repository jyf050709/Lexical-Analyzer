int complex_helper(int x) {
    int result = 0;
    int i = 0;
    while (i < x) {
        result = result + i;
        i = i + 1;
    }
    return result;
}

int tail_with_call(int n, int acc) {
    if (n <= 0) {
        return acc;
    }
    return tail_with_call(n - 1, acc + complex_helper(n));
}

int main() {
    return tail_with_call(3, 0);
}
