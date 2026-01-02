int exprTest(int a,int b,int c){
    int result = (a + b) * (c - a) + b * c;
    return result;
}
int main(){
    return exprTest(3,4,5);
}
