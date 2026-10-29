#include "../common/Lab05.hpp"

// Запуск двух потоков и фиксация прогресса при первом завершении.
int main(int argc, char** argv) {
    try {
        std::cout << std::unitbuf;
        if (argc < 4 || argc > 6) throw std::invalid_argument("Usage: Lab-05c mask nice1 nice2 [iterations [sleep_ms]]");
        U64 n = argc > 4 ? number(argv[4]) : 1000000;
        int ms = argc > 5 ? signed_number(argv[5]) : 200;
        if (!n || n > 100000000 || ms < 0 || ms > 1000) throw std::invalid_argument("Invalid iterations/sleep_ms");
        return threads(number(argv[1]), 0, signed_number(argv[2]), signed_number(argv[3]), n, ms);
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << std::endl;
        return 1;
    }
}
