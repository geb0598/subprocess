#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

int main(int argc, char** argv) {
    if (argc > 1) {
        string arg = argv[1];
        exit(stoi(arg));
    } else {
        exit(EXIT_FAILURE);
    }
}