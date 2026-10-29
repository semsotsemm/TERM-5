#include "../common/Lab05.hpp"

// Вывод параметров текущего процесса и потока.
int main() {
    try {
        std::cout << std::unitbuf;
        return info();
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << std::endl;
        return 1;
    }
}
