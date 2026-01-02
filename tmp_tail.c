int fact(int n, int acc){
    if(n<=1) return acc;
    return fact(n-1, n*acc);
}
int main(){
    return fact(5,1);
}
