#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    string input;
    string line;
    int lineNum = 1;

    while (getline(cin, line)) {
        input += line + "\n";
        lineNum++;
    }

    cout << "Total lines read: " << lineNum - 1 << endl;
    cout << "Last char: " << (int)(input.empty() ? 0 : input[input.length()-1]) << endl;

    return 0;
}
