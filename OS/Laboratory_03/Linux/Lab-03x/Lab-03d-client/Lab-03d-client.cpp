#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>

using namespace std;

long long gcd(long long a, long long b) {
    while (b) {
        a %= b;
        swap(a, b);
    }
    return a;
}

int main(int argc, char* argv[]) {
    if (argc < 4) return 1;

    long long M = stoll(argv[1]);
    long long L = stoll(argv[2]);
    long long R = stoll(argv[3]);

    stringstream ss;
    for (long long n = L; n <= R; ++n) {
        if (gcd(n, M) == 1) {
            ss << n << " ";
        }
    }

    string result = ss.str();
    if (!result.empty()) {
        write(STDOUT_FILENO, result.c_str(), result.length());
    }

    return 0;
}