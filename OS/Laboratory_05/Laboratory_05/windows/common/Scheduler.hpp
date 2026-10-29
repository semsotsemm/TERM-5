#pragma once
#include <string>

namespace lab05 {
    // Общая точка запуска моделей SJF, RR, CP и MQ.
    int run_scheduler(int argc, char** argv, const std::string& algorithm);
}
