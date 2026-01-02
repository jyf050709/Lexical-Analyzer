int foo(int n){
    int i = 0;
    int sum = 0;
    int bound = n;
    while(i < bound){
        sum = sum + 1;
        i = i + 1;
    }
    return sum;
}
int main(){ return foo(100); }
