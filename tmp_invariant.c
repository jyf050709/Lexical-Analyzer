int g(int x){
    int t = 0;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    t = t + 1;
    return x + t;
}

int foo(int n){
    int bound = g(n);
    int i = 0;
    int sum = 0;
    while(i < bound){
        sum = sum + 1;
        i = i + 1;
    }
    return sum;
}

int main(){
    return foo(100);
}
