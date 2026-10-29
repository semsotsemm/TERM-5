#pragma once
#ifndef _WIN32
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#include <atomic>
#include <bitset>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <exception>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <new>
#include <stdexcept>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using U64 = std::uint64_t;
using Steady = std::chrono::steady_clock;
static std::mutex output_mutex;

static void check(bool ok, const char* operation) {
    if (!ok) {
#ifdef _WIN32
        throw std::runtime_error(std::string(operation) + ": Win32=" + std::to_string(GetLastError()));
#else
        throw std::runtime_error(std::string(operation) + ": " + std::strerror(errno));
#endif
    }
}
static U64 number(const char* text) {
    std::string value(text);
    if (value.empty() || value[0] == '-') throw std::invalid_argument("Expected unsigned integer");
    std::size_t pos = 0;
    U64 n = std::stoull(value, &pos, 0);
    if (pos != value.size()) throw std::invalid_argument("Invalid integer");
    return n;
}
static void write_line(const std::string& line) {
#ifdef _WIN32
    std::cout << line << std::endl;
#else
    const std::string text = line + "\n";
    ssize_t written;
    do { written = write(STDOUT_FILENO, text.data(), text.size()); } while (written < 0 && errno == EINTR);
    check(written == static_cast<ssize_t>(text.size()), "console write");
#endif
}
static int signed_number(const char* text) {
    std::size_t pos = 0;
    const std::string value(text);
    int n = std::stoi(value, &pos, 0);
    if (pos != value.size()) throw std::invalid_argument("Invalid signed integer");
    return n;
}
static U64 process_id() {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return static_cast<U64>(getpid());
#endif
}
static U64 thread_id() {
#ifdef _WIN32
    return GetCurrentThreadId();
#else
    return static_cast<U64>(syscall(SYS_gettid));
#endif
}
static int cpu_number() {
#ifdef _WIN32
    return static_cast<int>(GetCurrentProcessorNumber());
#else
    int n = sched_getcpu(); check(n >= 0, "sched_getcpu"); return n;
#endif
}
static U64 allowed_mask() {
#ifdef _WIN32
    DWORD_PTR pm = 0, sm = 0;
    check(GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm) != 0, "GetProcessAffinityMask");
    return static_cast<U64>(pm);
#else
    cpu_set_t cpus; CPU_ZERO(&cpus);
    check(sched_getaffinity(0, sizeof(cpus), &cpus) == 0, "sched_getaffinity");
    U64 mask = 0;
    for (int i = 0; i < CPU_SETSIZE; ++i) {
        if (CPU_ISSET(i, &cpus)) {
            if (i >= 64) throw std::runtime_error("This lab supports CPU IDs 0..63; use an <=64-vCPU VM");
            mask |= U64(1) << i;
        }
    }
    return mask;
#endif
}
static void apply_mask(U64 mask) {
    const U64 allowed = allowed_mask();
    if (!mask || (mask & allowed) != mask) throw std::invalid_argument("Mask is empty or contains unavailable CPUs");
#ifdef _WIN32
    check(SetProcessAffinityMask(GetCurrentProcess(), static_cast<DWORD_PTR>(mask)) != 0, "SetProcessAffinityMask");
#else
    cpu_set_t cpus; CPU_ZERO(&cpus);
    for (int i = 0; i < 64; ++i) if ((mask >> i) & 1) CPU_SET(i, &cpus);
    check(sched_setaffinity(0, sizeof(cpus), &cpus) == 0, "sched_setaffinity");
#endif
}
#ifdef _WIN32
static const char* class_name(DWORD pc) {
    switch (pc) {
    case IDLE_PRIORITY_CLASS: return "Idle";
    case BELOW_NORMAL_PRIORITY_CLASS: return "BelowNormal";
    case NORMAL_PRIORITY_CLASS: return "Normal";
    case ABOVE_NORMAL_PRIORITY_CLASS: return "AboveNormal";
    case HIGH_PRIORITY_CLASS: return "High";
    case REALTIME_PRIORITY_CLASS: return "Realtime";
    default: throw std::runtime_error("Unknown process priority class");
    }
}
static int base_priority(DWORD pc, int tp) {
    int base = pc == IDLE_PRIORITY_CLASS ? 4 : pc == BELOW_NORMAL_PRIORITY_CLASS ? 6 :
               pc == NORMAL_PRIORITY_CLASS ? 8 : pc == ABOVE_NORMAL_PRIORITY_CLASS ? 10 :
               pc == HIGH_PRIORITY_CLASS ? 13 : 24;
    if (tp == THREAD_PRIORITY_IDLE) return pc == REALTIME_PRIORITY_CLASS ? 16 : 1;
    if (tp == THREAD_PRIORITY_TIME_CRITICAL) return pc == REALTIME_PRIORITY_CLASS ? 31 : 15;
    return base + tp;
}
static void apply_class(int pc) {
    if (pc != NORMAL_PRIORITY_CLASS && pc != BELOW_NORMAL_PRIORITY_CLASS && pc != HIGH_PRIORITY_CLASS)
        throw std::invalid_argument("Use process class 32, 16384 or 128; real-time is not used");
    check(SetPriorityClass(GetCurrentProcess(), static_cast<DWORD>(pc)) != 0, "SetPriorityClass");
}
static void thread_priority(int value) {
    if (value < -2 || value > 2) throw std::invalid_argument("Use relative thread priority -2..2");
    check(SetThreadPriority(GetCurrentThread(), value) != 0, "SetThreadPriority");
}
#else
static int current_nice() {
    errno = 0; int value = getpriority(PRIO_PROCESS, 0);
    check(errno == 0, "getpriority"); return value;
}
static void thread_priority(int value) {
    if (value < -20 || value > 19) throw std::invalid_argument("nice must be -20..19");
    check(setpriority(PRIO_PROCESS, 0, value) == 0, "setpriority (negative nice needs sudo/CAP_SYS_NICE)");
    check(current_nice() == value, "verify nice");
}
#endif
static void print_identity(U64 iteration = 0) {
    std::lock_guard<std::mutex> guard(output_mutex);
    std::ostringstream line;
    line << "ITER=" << iteration << " PID=" << process_id() << " TID=" << thread_id();
#ifdef _WIN32
    DWORD pc = GetPriorityClass(GetCurrentProcess()); check(pc != 0, "GetPriorityClass");
    int tp = GetThreadPriority(GetCurrentThread()); check(tp != THREAD_PRIORITY_ERROR_RETURN, "GetThreadPriority");
    line << " CLASS=" << class_name(pc) << " CLASS_CODE=" << pc
              << " THREAD_REL=" << tp << " BASE=" << base_priority(pc, tp);
#else
    line << " NICE=" << current_nice();
#endif
    line << " CPU=" << cpu_number();
    write_line(line.str());
}
static int info() {
    print_identity();
    U64 mask = allowed_mask();
    unsigned count = 0; for (U64 copy = mask; copy; copy >>= 1) count += unsigned(copy & 1);
    std::cout << "PROCESS_MASK_BIN=" << std::bitset<64>(mask) << "\nMASK_DEC=" << mask
              << "\nCPU_COUNT=" << count << "\nONE_CPU_MASK_DEC=" << (mask & (~mask + 1)) << std::endl;
#ifdef _WIN32
    DWORD_PTR pm = 0, sm = 0;
    check(GetProcessAffinityMask(GetCurrentProcess(), &pm, &sm) != 0, "GetProcessAffinityMask");
    std::cout << "SYSTEM_MASK_BIN=" << std::bitset<64>(static_cast<U64>(sm)) << std::endl;
#else
    int policy = sched_getscheduler(0); check(policy >= 0, "sched_getscheduler");
    sched_param param{}; check(sched_getparam(0, &param) == 0, "sched_getparam");
    std::cout << "POLICY=" << policy << " SCHED_PRIORITY=" << param.sched_priority << std::endl;
#endif
    return 0;
}

// All readers/writers use lock-free atomics, supported by the target x64 builds.
struct alignas(64) Counter {
    std::atomic<U64> value{0};
    char padding[64 - sizeof(std::atomic<U64>)]{};
};
struct Shared {
    Counter progress[2];
    std::atomic<int> winner{-1}, ready{0}, failed{0};
    std::atomic<U64> snapshot[2]{{0},{0}};
    std::atomic<U64> pid[2]{{0},{0}}, tid[2]{{0},{0}};
};
static_assert(std::atomic<U64>::is_always_lock_free, "Use x64 lock-free atomics");
struct Mapping {
    Shared* data = nullptr;
    std::string name;
    bool owner = false;
#ifdef _WIN32
    HANDLE handle = nullptr;
#else
    int fd = -1;
#endif
    Mapping(bool create, const std::string& key) : name(key), owner(create) {
#ifdef _WIN32
        if (create) handle = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(Shared), name.c_str());
        else handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
        check(handle != nullptr, "file mapping");
        data = static_cast<Shared*>(MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Shared)));
        check(data != nullptr, "MapViewOfFile");
#else
        fd = shm_open(name.c_str(), O_RDWR | (create ? O_CREAT | O_EXCL : 0), 0600);
        check(fd >= 0, "shm_open");
        if (create) check(ftruncate(fd, sizeof(Shared)) == 0, "ftruncate");
        void* ptr = mmap(nullptr, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        check(ptr != MAP_FAILED, "mmap"); data = static_cast<Shared*>(ptr);
#endif
        if (create) new(data) Shared();
    }
    ~Mapping() {
#ifdef _WIN32
        if (data) UnmapViewOfFile(data);
        if (handle) CloseHandle(handle);
#else
        if (data) munmap(data, sizeof(Shared));
        if (fd >= 0) close(fd);
        if (owner) shm_unlink(name.c_str());
#endif
    }
};
static std::string mapping_name() {
#ifdef _WIN32
    return "Local\\OSLab05_" + std::to_string(process_id());
#else
    return "/os_lab05_" + std::to_string(process_id());
#endif
}
static void workload(U64 iterations, int sleep_ms, Shared* shared = nullptr, int slot = 0) {
    if (shared) { shared->pid[slot] = process_id(); shared->tid[slot] = thread_id(); }
    const auto start = Steady::now();
    for (U64 i = 1; i <= iterations; ++i) {
        if (shared) shared->progress[slot].value.store(i, std::memory_order_relaxed);
        if (i % 1000 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            print_identity(i);
        }
    }
    if (shared) {
        int expected = -1;
        if (shared->winner.compare_exchange_strong(expected, slot)) {
            shared->snapshot[0] = shared->progress[0].value.load();
            shared->snapshot[1] = shared->progress[1].value.load();
            shared->ready.store(1);
        }
    }
    const double elapsed = std::chrono::duration<double>(Steady::now() - start).count();
    const std::clock_t raw_clock = std::clock();
    check(raw_clock != std::clock_t(-1), "clock");
    const double ticks = double(raw_clock) / CLOCKS_PER_SEC;
    std::lock_guard<std::mutex> guard(output_mutex);
    std::ostringstream line;
    line << std::fixed << std::setprecision(6) << "FINISHED PID=" << process_id()
              << " TID=" << thread_id() << " ITER=" << iterations
              << " WALL_SECONDS=" << elapsed << " CLOCK_SECONDS=" << ticks;
    write_line(line.str());
}
static void snapshot(Shared* shared) {
    std::ostringstream line;
    line << "FIRST_FINISH SLOT=" << shared->winner.load() + 1
              << " PID1=" << shared->pid[0] << " TID1=" << shared->tid[0]
              << " PID2=" << shared->pid[1] << " TID2=" << shared->tid[1]
              << "\nITER1=" << shared->snapshot[0] << " ITER2=" << shared->snapshot[1]
              << " DELTA=" << (shared->snapshot[0] > shared->snapshot[1] ?
                  shared->snapshot[0] - shared->snapshot[1] : shared->snapshot[1] - shared->snapshot[0])
              << '\n';
    write_line(line.str());
}
static std::string executable_x() {
#ifdef _WIN32
    std::vector<char> path(32768);
    DWORD n = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
    check(n > 0 && n < path.size(), "GetModuleFileName");
    std::string value(path.data(), n);
    return value.substr(0, value.find_last_of("\\/")) + "\\Lab-05x.exe";
#else
    std::vector<char> path(4096);
    ssize_t n = readlink("/proc/self/exe", path.data(), path.size() - 1);
    check(n > 0 && std::size_t(n) < path.size() - 1, "readlink");
    std::string value(path.data(), std::size_t(n));
    return value.substr(0, value.find_last_of('/')) + "/Lab-05x";
#endif
}
static int child_main(int argc, char** argv) {
    U64 n = argc > 1 ? number(argv[1]) : 1000000;
    int ms = argc > 2 ? signed_number(argv[2]) : 200;
    if (!n || n > 100000000 || ms < 0 || ms > 1000) throw std::invalid_argument("Invalid iterations/sleep_ms");
    if (argc == 3 || argc <= 2) { workload(n, ms); return 0; }
    if (argc != 6) throw std::invalid_argument("Lab-05x [iterations [sleep_ms]]; internal args are reserved");
    Mapping shared(false, argv[3]); int slot = signed_number(argv[4]);
    if (slot < 0 || slot > 1) throw std::invalid_argument("Invalid slot");
#ifdef _WIN32
    HANDLE gate = OpenEventA(SYNCHRONIZE, FALSE, argv[5]); check(gate != nullptr, "OpenEvent");
    check(WaitForSingleObject(gate, INFINITE) == WAIT_OBJECT_0, "WaitForSingleObject"); CloseHandle(gate);
#else
    int gate = signed_number(argv[5]); char signal = 0;
    ssize_t got; do { got = read(gate, &signal, 1); } while (got < 0 && errno == EINTR);
    check(got == 1, "start gate"); close(gate);
#endif
    workload(n, ms, shared.data, slot);
#ifdef _WIN32
    std::cout << "Press Enter to close this child console." << std::endl;
    std::cin.get();
#endif
    return 0;
}

static int processes(U64 mask, int first, int second, U64 n, int ms) {
    apply_mask(mask);
    Mapping map(true, mapping_name());
    std::cout << "PARENT_PID=" << process_id() << " MASK=" << mask
              << " P2=" << first << " P3=" << second << " N=" << n << " SLEEP_MS=" << ms << std::endl;
#ifdef _WIN32
    const std::string gate_name = map.name + "_gate";
    HANDLE gate = CreateEventA(nullptr, TRUE, FALSE, gate_name.c_str()); check(gate != nullptr, "CreateEvent");
    PROCESS_INFORMATION children[2]{};
    try {
        int priorities[2] = {first, second};
        for (int i = 0; i < 2; ++i) {
            if (priorities[i] != 32 && priorities[i] != 16384 && priorities[i] != 128)
                throw std::invalid_argument("Use process priorities 32, 16384, 128");
            std::string command = "\"" + executable_x() + "\" " + std::to_string(n) + " " + std::to_string(ms) +
                " " + map.name + " " + std::to_string(i) + " " + gate_name;
            STARTUPINFOA startup{}; startup.cb = sizeof(startup);
            check(CreateProcessA(nullptr, &command[0], nullptr, nullptr, FALSE,
                CREATE_NEW_CONSOLE | CREATE_SUSPENDED | DWORD(priorities[i]), nullptr, nullptr,
                &startup, &children[i]) != 0, "CreateProcess");
            map.data->pid[i] = children[i].dwProcessId;
            check(SetProcessAffinityMask(children[i].hProcess, static_cast<DWORD_PTR>(mask)) != 0, "child affinity");
            check(ResumeThread(children[i].hThread) != DWORD(-1), "ResumeThread");
            std::cout << "CHILD" << i + 1 << " PID=" << children[i].dwProcessId << std::endl;
        }
        check(SetEvent(gate) != 0, "SetEvent");
        while (!map.data->ready.load()) {
            for (auto& child : children) {
                if (WaitForSingleObject(child.hProcess, 0) == WAIT_OBJECT_0)
                    throw std::runtime_error("Child exited before snapshot");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        snapshot(map.data);
        std::cout << "Capture the snapshot. Close child consoles with Enter after FINISHED." << std::endl;
        for (auto& child : children) {
            check(WaitForSingleObject(child.hProcess, INFINITE) == WAIT_OBJECT_0, "wait child");
            DWORD code = 1; check(GetExitCodeProcess(child.hProcess, &code) != 0, "GetExitCodeProcess");
            check(code == 0, "child exit code");
        }
    } catch (...) {
        for (auto& child : children) if (child.hProcess) TerminateProcess(child.hProcess, 1);
        for (auto& child : children) { if (child.hProcess) CloseHandle(child.hProcess); if (child.hThread) CloseHandle(child.hThread); }
        CloseHandle(gate); throw;
    }
    for (auto& child : children) { CloseHandle(child.hProcess); CloseHandle(child.hThread); }
    CloseHandle(gate);
#else
    int gate[2]; check(pipe(gate) == 0, "pipe");
    pid_t children[2] = {-1,-1};
    int priorities[2] = {first, second};
    for (int i = 0; i < 2; ++i) {
        if (priorities[i] < -20 || priorities[i] > 19) throw std::invalid_argument("nice must be -20..19");
        children[i] = fork();
        if (children[i] < 0) {
            for (pid_t pid : children) if (pid > 0) { kill(pid, SIGTERM); waitpid(pid, nullptr, 0); }
            close(gate[0]); close(gate[1]); throw std::runtime_error("fork failed");
        }
        if (children[i] == 0) {
            close(gate[1]);
            errno = 0;
            int value = nice(priorities[i] - current_nice());
            if ((value == -1 && errno) || value != priorities[i]) {
                perror("nice: run with sudo for -20"); map.data->failed.store(1); _exit(1);
            }
            std::string ns = std::to_string(n), mss = std::to_string(ms), slot = std::to_string(i), fd = std::to_string(gate[0]);
            const std::string executable = executable_x();
            execl(executable.c_str(), executable.c_str(), ns.c_str(), mss.c_str(), map.name.c_str(), slot.c_str(), fd.c_str(), nullptr);
            perror("execl"); map.data->failed.store(1); _exit(1);
        }
        map.data->pid[i] = static_cast<U64>(children[i]);
        std::cout << "CHILD" << i + 1 << " PID=" << children[i] << std::endl;
    }
    close(gate[0]);
    const char release[2] = {'1','1'};
    check(write(gate[1], release, 2) == 2, "release children"); close(gate[1]);
    bool failed = false;
    while (!map.data->ready.load() && !map.data->failed.load()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (map.data->ready.load()) snapshot(map.data);
    if (map.data->failed.load()) {
        failed = true; for (pid_t pid : children) kill(pid, SIGTERM);
    }
    for (pid_t pid : children) {
        int status = 0; pid_t result;
        do { result = waitpid(pid, &status, 0); } while (result < 0 && errno == EINTR);
        if (result != pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0) failed = true;
    }
    if (failed) throw std::runtime_error("Child failed; see errors above");
#endif
    return 0;
}
static int threads(U64 mask, int process_class, int first, int second, U64 n, int ms) {
    apply_mask(mask);
#ifdef _WIN32
    apply_class(process_class);
#else
    (void)process_class;
#endif
    Shared shared;
    std::mutex mutex; std::condition_variable cv;
    int ready = 0; bool go = false;
    std::thread workers[2];
    std::cout << "PARENT_PID=" << process_id() << " MASK=" << mask << " PROCESS_CLASS=" << process_class
              << " PRIORITY1=" << first << " PRIORITY2=" << second << " N=" << n << " SLEEP_MS=" << ms << std::endl;
    auto run = [&](int slot, int priority) {
        try { thread_priority(priority); }
        catch (const std::exception& error) {
            { std::lock_guard<std::mutex> output(output_mutex); std::cerr << error.what() << std::endl; }
            shared.failed.store(1);
        }
        { std::unique_lock<std::mutex> lock(mutex); ++ready; cv.notify_all(); cv.wait(lock,[&]{return go;}); }
        if (shared.failed.load()) return;
        try { workload(n, ms, &shared, slot); }
        catch (const std::exception& error) {
            std::lock_guard<std::mutex> output(output_mutex); std::cerr << error.what() << std::endl; shared.failed.store(1);
        }
    };
    try { workers[0] = std::thread(run,0,first); workers[1] = std::thread(run,1,second); }
    catch (...) {
        shared.failed.store(1); { std::lock_guard<std::mutex> lock(mutex); go = true; } cv.notify_all();
        for (auto& worker : workers) if (worker.joinable()) worker.join();
        throw;
    }
    { std::unique_lock<std::mutex> lock(mutex); cv.wait(lock,[&]{return ready == 2;}); go = true; }
    cv.notify_all();
    while (!shared.ready.load() && !shared.failed.load()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (shared.ready.load()) { std::lock_guard<std::mutex> output(output_mutex); snapshot(&shared); }
    for (auto& worker : workers) worker.join();
    if (shared.failed.load()) throw std::runtime_error("Worker failed");
    return 0;
}
int main(int argc, char** argv) {
    try {
        std::cout << std::unitbuf;
        if (LAB_MODE == 'a') return info();
        if (LAB_MODE == 'x') return child_main(argc, argv);
        const bool is_thread = LAB_MODE == 'c';
#ifdef _WIN32
        const int required = is_thread ? 5 : 4;
#else
        const int required = 4;
#endif
        if (argc < required || argc > required + 2)
            throw std::invalid_argument("b: mask priority1 priority2 [iterations [sleep_ms]]; c on Windows adds process class after mask");
        U64 mask = number(argv[1]);
        U64 n = argc > required ? number(argv[required]) : 1000000;
        int ms = argc > required + 1 ? signed_number(argv[required+1]) : 200;
        if (!n || n > 100000000 || ms < 0 || ms > 1000) throw std::invalid_argument("Invalid iterations/sleep_ms");
        if (!is_thread) return processes(mask,signed_number(argv[2]),signed_number(argv[3]),n,ms);
#ifdef _WIN32
        return threads(mask,signed_number(argv[2]),signed_number(argv[3]),signed_number(argv[4]),n,ms);
#else
        return threads(mask,0,signed_number(argv[2]),signed_number(argv[3]),n,ms);
#endif
    } catch (const std::exception& error) { std::cerr << "ERROR: " << error.what() << std::endl; return 1; }
}
