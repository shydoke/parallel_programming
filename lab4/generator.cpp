#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

int main() {

    int n = 2000;

    ofstream file("B_2000.txt");

    file << n << '\n';

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file << rand() % 10 << ' ';
        }
        file << '\n';
    }

    file.close();

    return 0;
}