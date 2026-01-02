int swap_tail(int a, int b, int count) {
    if (count <= 0) {
        return a + b;
    }
    return swap_tail(b, a, count - 1);
}

int main() {
    return swap_tail(3, 5, 4);
}
