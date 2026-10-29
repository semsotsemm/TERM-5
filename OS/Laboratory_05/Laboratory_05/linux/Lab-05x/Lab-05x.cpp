#include "../common/Lab05.hpp"

// Циклическая нагрузка; используется также дочерними процессами.
int main(int argc, char** argv) {
    try {
        std::cout << std::unitbuf;
        return child_main(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << std::endl;
        return 1;
    }
}
