int nine_params(int a, int b, int c, int d, int e, int f, int g, int h, int i) {
    if (a <= 0) {
        return b + c + d + e + f + g + h + i;
    }
    return nine_params(a - 1, c, d, e, f, g, h, i, b + 1);
}

int ten_params(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    if (a <= 0) {
        return b + c + d + e + f + g + h + i + j;
    }
    return ten_params(a - 1, c, d, e, f, g, h, i, j, b + 1);
}

int main() {
    int r1 = nine_params(3, 1, 2, 3, 4, 5, 6, 7, 8);
    int r2 = ten_params(2, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    return r1 + r2;
}
