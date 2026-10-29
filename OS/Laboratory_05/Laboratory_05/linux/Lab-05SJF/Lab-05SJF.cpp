#include "../common/Scheduler.hpp"

// Моделирование SJF: реальные пользовательские задания не запускаются.
// Входные параметры и журнал результатов обрабатываются общей моделью.
int main(int argc, char** argv) {
    return lab05::run_scheduler(argc, argv, "SJF");
}
