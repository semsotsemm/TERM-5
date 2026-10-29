#include "Scheduler.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace lab05 {
// Небольшой сериализатор: проекты не требуют сторонних библиотек.
struct Json {
    using Array = std::vector<Json>;
    using Object = std::map<std::string, Json>;
    std::variant<std::nullptr_t, bool, long long, double, std::string, Array, Object> value;
    Json() : value(nullptr) {}
    Json(bool v) : value(v) {}
    Json(int v) : value(static_cast<long long>(v)) {}
    Json(long long v) : value(v) {}
    Json(double v) : value(v) {}
    Json(const char* v) : value(std::string(v)) {}
    Json(std::string v) : value(std::move(v)) {}
    Json(Array v) : value(std::move(v)) {}
    Json(Object v) : value(std::move(v)) {}
    static Json object(std::initializer_list<Object::value_type> v) { return Object(v); }
    static std::string quote(const std::string& s) {
        std::ostringstream out; out << '"';
        for (unsigned char c : s) {
            if (c == '"' || c == '\\') out << '\\' << c;
            else if (c == '\n') out << "\\n";
            else if (c == '\r') out << "\\r";
            else if (c == '\t') out << "\\t";
            else if (c < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << int(c) << std::dec;
            else out << c;
        }
        return out.str() + '"';
    }
    void write(std::ostream& out) const {
        if (std::holds_alternative<std::nullptr_t>(value)) out << "null";
        else if (auto b = std::get_if<bool>(&value)) out << (*b ? "true" : "false");
        else if (auto n = std::get_if<long long>(&value)) out << *n;
        else if (auto d = std::get_if<double>(&value)) out << std::setprecision(17) << *d;
        else if (auto s = std::get_if<std::string>(&value)) out << quote(*s);
        else if (auto a = std::get_if<Array>(&value)) {
            out << '['; bool first = true;
            for (const auto& item : *a) {
                if (!first) out << ',';
                first = false; item.write(out);
            }
            out << ']';
        } else {
            out << '{'; bool first = true;
            for (const auto& [key, item] : std::get<Object>(value)) {
                if (!first) out << ',';
                first = false; out << quote(key) << ':'; item.write(out);
            }
            out << '}';
        }
    }
};
inline std::ostream& operator<<(std::ostream& out, const Json& v) { v.write(out); return out; }
inline void save_json(const std::filesystem::path& path, const Json& value) {
    std::ofstream out(path); if (!out) throw std::runtime_error("Cannot create " + path.string());
    out << value << '\n'; if (!out) throw std::runtime_error("Cannot write " + path.string());
}
inline void save_csv(const std::filesystem::path& path, const Json::Array& rows,
                     const std::vector<std::string>& fields) {
    std::ofstream out(path); if (!out) throw std::runtime_error("Cannot create " + path.string());
    for (std::size_t i = 0; i < fields.size(); ++i) out << (i ? "," : "") << fields[i];
    out << '\n';
    for (const auto& row : rows) {
        const auto& obj = std::get<Json::Object>(row.value);
        for (std::size_t i = 0; i < fields.size(); ++i) {
            if (i) out << ',';
            const auto& item = obj.at(fields[i]);
            if (const auto* s = std::get_if<std::string>(&item.value)) out << *s;
            else out << item;
        }
        out << '\n';
    }
    if (!out) throw std::runtime_error("CSV write failed");
}
inline int integer(const std::string& value) {
    std::size_t end = 0; int number = std::stoi(value, &end);
    if (end != value.size()) throw std::invalid_argument("Expected an integer: " + value);
    return number;
}
struct Task { int id, arrival, burst, priority; };
inline std::vector<Task> load_tasks(const std::filesystem::path& path) {
    std::ifstream in(path); if (!in) throw std::runtime_error("Cannot open tasks: " + path.string());
    std::string line; std::getline(in, line); std::vector<Task> tasks; std::set<int> ids;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::istringstream row(line); std::string field; std::vector<int> v;
        while (std::getline(row, field, ',')) v.push_back(integer(field));
        if (v.size() != 4 || v[1] < 0 || v[2] <= 0 || !ids.insert(v[0]).second)
            throw std::invalid_argument("Invalid task row: " + line);
        tasks.push_back({v[0], v[1], v[2], v[3]});
    }
    if (tasks.empty()) throw std::invalid_argument("Task list is empty");
    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return std::tie(a.arrival, a.id) < std::tie(b.arrival, b.id);
    });
    return tasks;
}
struct Stats {
    std::string algorithm; int quantum = 0;
    double waiting = 0, execution = 0, turnaround = 0, response = 0, throughput = 0;
    int max_response = 0, makespan = 0, switches = 0;
    Json json() const {
        return Json::object({{"algorithm", algorithm}, {"quantum", quantum}, {"avg_waiting", waiting},
            {"avg_execution", execution}, {"avg_turnaround", turnaround}, {"avg_response", response},
            {"max_response", max_response}, {"throughput", throughput},
            {"makespan", makespan}, {"switches", switches}});
    }
};
struct Result { Stats summary; Json::Array tasks, timeline; Json json() const {
    return Json::object({{"summary", summary.json()}, {"tasks", tasks}, {"timeline", timeline}});
} };
inline Result simulate(const std::vector<Task>& input, const std::string& algorithm, int quantum = 20) {
    if (input.empty() || quantum <= 0 || (algorithm != "SJF" && algorithm != "RR" && algorithm != "CP"))
        throw std::invalid_argument("Invalid scheduling parameters");
    auto tasks = input;
    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
        return std::tie(a.arrival, a.id) < std::tie(b.arrival, b.id);
    });
    std::map<int, int> remaining, start, finish;
    std::map<int, std::deque<int>> queues;
    for (const auto& p : tasks) remaining[p.id] = p.burst;
    std::vector<std::array<int, 3>> timeline;
    int time = 0, pending = 0, active = -1, credit = 0, previous = -1, switches = 0;
    auto enqueue = [&](int i, bool front = false) {
        auto& q = queues[algorithm == "CP" ? tasks[i].priority : 0];
        if (front) q.push_front(i); else q.push_back(i);
    };
    auto arrivals = [&] {
        while (pending < int(tasks.size()) && tasks[pending].arrival <= time) enqueue(pending++);
    };
    auto highest = [&] {
        for (auto it = queues.rbegin(); it != queues.rend(); ++it) if (!it->second.empty()) return it->first;
        return std::numeric_limits<int>::min();
    };
    auto tick = [&](int pid) {
        if (!timeline.empty() && timeline.back()[2] == pid && timeline.back()[1] == time) ++timeline.back()[1];
        else timeline.push_back({time, time + 1, pid});
    };
    while (finish.size() < tasks.size()) {
        arrivals();
        // Новый более высокий приоритет вытесняет сразу, до следующего такта.
        if (active >= 0 && algorithm == "CP" && highest() > tasks[active].priority) {
            enqueue(active, true); active = -1;
        }
        if (active < 0) {
            int rank = highest();
            if (rank == std::numeric_limits<int>::min()) { tick(-1); ++time; previous = -1; continue; }
            auto& q = queues[rank];
            if (algorithm == "SJF") {
                auto selected = std::min_element(q.begin(), q.end(), [&](int a, int b) {
                    return std::tie(tasks[a].burst, tasks[a].arrival, tasks[a].id) <
                           std::tie(tasks[b].burst, tasks[b].arrival, tasks[b].id);
                });
                active = *selected; q.erase(selected);
            } else { active = q.front(); q.pop_front(); }
            credit = quantum; start.emplace(tasks[active].id, time);
        }
        int pid = tasks[active].id;
        if (previous >= 0 && previous != pid) ++switches;
        previous = pid; tick(pid); --remaining[pid]; --credit; ++time;
        // Поступление на границе кванта предшествует возврату прерванной задачи.
        arrivals();
        if (remaining[pid] == 0) { finish[pid] = time; active = -1; }
        else if (algorithm != "SJF" && credit == 0) { enqueue(active); active = -1; }
    }
    Result result; auto& s = result.summary; s.algorithm = algorithm;
    s.quantum = algorithm == "SJF" ? 0 : quantum; s.makespan = time - tasks.front().arrival; s.switches = switches;
    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) { return a.id < b.id; });
    int service = 0;
    for (const auto& p : tasks) {
        int turnaround = finish[p.id] - p.arrival, waiting = turnaround - p.burst, response = start[p.id] - p.arrival;
        if (waiting < 0) throw std::logic_error("Negative waiting time");
        s.waiting += waiting; s.execution += p.burst; s.turnaround += turnaround; s.response += response;
        s.max_response = std::max(s.max_response, response); service += p.burst;
        result.tasks.push_back(Json::object({{"id", p.id}, {"arrival", p.arrival}, {"burst", p.burst},
            {"priority", p.priority}, {"start", start[p.id]}, {"finish", finish[p.id]},
            {"waiting", waiting}, {"turnaround", turnaround}, {"response", response}}));
    }
    double count = double(tasks.size());
    s.waiting /= count; s.execution /= count; s.turnaround /= count; s.response /= count;
    s.throughput = count / s.makespan;
    int accounted = 0;
    for (const auto& [begin, end, pid] : timeline) {
        result.timeline.push_back(Json::Array{begin, end, pid < 0 ? Json() : Json(pid)});
        if (pid >= 0) accounted += end - begin;
    }
    if (accounted != service) throw std::logic_error("CPU accounting mismatch");
    return result;
}

// MT19937 с последовательностью integer-seed/random/randrange прежней модели.
// Это сохраняет исходные задачи и переходы MQ при seed=525.
class Random {
    std::array<std::uint32_t, 624> state{}; std::size_t index = 624;
    std::uint32_t next() {
        if (index == 624) {
            for (std::size_t i = 0; i < 624; ++i) {
                auto y = (state[i] & 0x80000000u) | (state[(i + 1) % 624] & 0x7fffffffu);
                state[i] = state[(i + 397) % 624] ^ (y >> 1) ^ ((y & 1) ? 0x9908b0dfu : 0u);
            }
            index = 0;
        }
        auto y = state[index++]; y ^= y >> 11; y ^= (y << 7) & 0x9d2c5680u;
        y ^= (y << 15) & 0xefc60000u; return y ^ (y >> 18);
    }
public:
    explicit Random(std::uint64_t seed) {
        state[0] = 19650218u;
        for (std::size_t i = 1; i < 624; ++i) state[i] = 1812433253u * (state[i-1] ^ (state[i-1] >> 30)) + std::uint32_t(i);
        std::vector<std::uint32_t> key{std::uint32_t(seed)};
        if (seed >> 32) key.push_back(std::uint32_t(seed >> 32));
        std::size_t i = 1, j = 0;
        for (std::size_t k = std::max<std::size_t>(624, key.size()); k; --k) {
            state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1664525u)) + key[j] + std::uint32_t(j);
            if (++i == 624) { state[0] = state[623]; i = 1; }
            if (++j == key.size()) j = 0;
        }
        for (int k = 623; k; --k) {
            state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1566083941u)) - std::uint32_t(i);
            if (++i == 624) { state[0] = state[623]; i = 1; }
        }
        state[0] = 0x80000000u;
    }
    double real() { auto a = next() >> 5, b = next() >> 6; return (double(a) * 67108864.0 + b) / 9007199254740992.0; }
    int below(int limit) {
        unsigned bits = 0; for (unsigned n = unsigned(limit); n; n >>= 1) ++bits;
        std::uint32_t value; do { value = next() >> (32 - bits); } while (value >= unsigned(limit));
        return int(value);
    }
    int between(int a, int b) { return a + below(b - a + 1); }
};
struct MQTask {
    int id, arrival, burst; std::string kind; int cpu_burst, io_delay;
    int remaining = 0, queue = 0, credit = 20, phase = 0, unblock = -1, finish = -1, first = -1, io_total = 0;
    std::array<int, 3> waiting{}, service{}; std::array<bool, 3> visited{};
};
inline Json simulate_mq(int seed = 525, int count = 100) {
    if (seed < 0 || count <= 0) throw std::invalid_argument("Invalid seed/count");
    Random rng(static_cast<std::uint64_t>(seed)); std::vector<MQTask> tasks;
    Json::Array inputs, events, rows, queue_rows, timeline;
    for (int i = 1; i <= count; ++i) {
        bool io = rng.real() < 0.5; int burst = io ? rng.between(80, 250) : rng.between(120, 600);
        int arrival = rng.between(0, 200), cpu = burst;
        if (io) {
            int choices[] = {rng.between(2, 8), rng.between(2, 8), rng.between(24, 35)};
            cpu = choices[rng.below(3)];
        }
        int delay = io ? rng.between(10, 40) : 0;
        MQTask p{i, arrival, burst, io ? "IO" : "CPU", cpu, delay}; p.remaining = burst; p.phase = cpu;
        tasks.push_back(p);
        inputs.push_back(Json::object({{"id", i}, {"arrival", arrival}, {"burst", burst},
            {"kind", p.kind}, {"cpu_burst", cpu}, {"io_delay", delay}}));
    }
    std::array<std::deque<int>, 3> queues;
    const int quanta[] = {20, 60, 120}, calendar[] = {0,0,0,0,0,1,1,1,2,2};
    int time = 0, done = 0, switches = 0, migrations = 0, idle = 0, previous = -1;
    std::vector<std::array<int, 4>> segments;
    auto enqueue = [&](int i, int queue, const std::string& reason) {
        auto& p = tasks[i];
        if (p.queue != queue) {
            ++migrations;
            events.push_back(Json::object({{"time", time}, {"id", p.id}, {"source", "Q" + std::to_string(p.queue)},
                {"target", "Q" + std::to_string(queue)}, {"reason", reason}}));
        }
        p.queue = queue; p.credit = quanta[queue]; p.visited[queue] = true; queues[queue].push_back(i);
    };
    while (done < count) {
        for (int i = 0; i < count; ++i) {
            auto& p = tasks[i];
            if (p.arrival == time) enqueue(i, 0, "arrival");
            else if (p.unblock == time) {
                p.unblock = -1; p.phase = std::min(p.cpu_burst, p.remaining);
                enqueue(i, std::max(0, p.queue - 1), "io-return");
            }
        }
        int selected = calendar[(time / 20) % 10];
        if (queues[selected].empty()) {
            selected = -1;
            for (int q = 0; q < 3; ++q) if (!queues[q].empty()) { selected = q; break; }
        }
        if (selected < 0) { ++idle; previous = -1; ++time; continue; }
        int task_index = queues[selected].front(); auto& p = tasks[task_index];
        if (previous >= 0 && previous != p.id) ++switches;
        previous = p.id; if (p.first < 0) p.first = time;
        for (const auto& q : queues) for (int i : q) if (i != task_index) ++tasks[i].waiting[tasks[i].queue];
        --p.remaining; --p.phase; --p.credit; ++p.service[selected];
        if (!segments.empty() && segments.back()[2] == p.id && segments.back()[3] == selected && segments.back()[1] == time)
            ++segments.back()[1];
        else segments.push_back({time, time + 1, p.id, selected});
        ++time;
        if (p.remaining == 0) { queues[selected].pop_front(); p.finish = time; ++done; }
        else if (p.kind == "IO" && p.phase == 0) {
            queues[selected].pop_front(); p.unblock = time + p.io_delay; p.io_total += p.io_delay;
        } else if (p.credit == 0) {
            queues[selected].pop_front(); enqueue(task_index, selected < 2 ? selected + 1 : rng.below(3), "quantum");
        }
    }
    double waiting = 0, turnaround = 0, response = 0; int service = 0;
    for (const auto& p : tasks) {
        int wait = std::accumulate(p.waiting.begin(), p.waiting.end(), 0), turn = p.finish - p.arrival;
        if (turn != wait + p.burst + p.io_total) throw std::logic_error("MQ accounting mismatch");
        Json::Object row = std::get<Json::Object>(Json::object({{"id", p.id}, {"kind", p.kind}, {"arrival", p.arrival},
            {"execution", p.burst}, {"finish", p.finish}, {"waiting", wait}, {"io", p.io_total},
            {"turnaround", turn}, {"response", p.first - p.arrival}, {"final_queue", "Q" + std::to_string(p.queue)}}).value);
        for (int q = 0; q < 3; ++q) {
            std::string key = "Q" + std::to_string(q);
            row[key + "_visited"] = int(p.visited[q]); row[key + "_wait"] = p.waiting[q]; row[key + "_cpu"] = p.service[q];
        }
        rows.push_back(row); waiting += wait; turnaround += turn; response += p.first - p.arrival; service += p.burst;
    }
    for (int q = 0; q < 3; ++q) {
        int visited = 0, completed = 0, cpu = 0; double wait = 0, turn = 0;
        for (const auto& p : tasks) {
            if (p.visited[q]) { ++visited; wait += p.waiting[q]; turn += p.finish - p.arrival; }
            completed += p.queue == q; cpu += p.service[q];
        }
        queue_rows.push_back(Json::object({{"queue", "Q" + std::to_string(q)}, {"visited", visited},
            {"completed", completed}, {"service", cpu}, {"share_percent", 100.0 * cpu / service},
            {"avg_queue_wait", visited ? wait / visited : 0.0}, {"avg_full_turnaround_visited", visited ? turn / visited : 0.0}}));
    }
    for (const auto& [a,b,p,q] : segments) timeline.push_back(Json::Array{a,b,p,q});
    if (service + idle != time) throw std::logic_error("MQ service + idle mismatch");
    return Json::object({{"summary", Json::object({{"seed", seed}, {"count", count},
        {"avg_waiting", waiting / count}, {"avg_turnaround", turnaround / count}, {"avg_response", response / count},
        {"makespan", time}, {"cpu_service", service}, {"idle", idle}, {"utilization_percent", 100.0 * service / time},
        {"switches", switches}, {"migrations", migrations}})}, {"queues", queue_rows}, {"tasks", rows},
        {"inputs", inputs}, {"events", events}, {"timeline", timeline}});
}

inline const Json& member(const Json& j, std::string key) { return std::get<Json::Object>(j.value).at(key); }
inline const Json::Array& array(const Json& j) { return std::get<Json::Array>(j.value); }
inline void print_result(const Result& r) {
    std::cout << "ALGORITHM=" << r.summary.algorithm << " QUANTUM=" << r.summary.quantum
              << " SWITCH_COST=0 HIGHER_NUMBER_HIGHER_PRIORITY\nID ARR CPU PRI START FINISH WAIT TURN RESPONSE\n";
    for (const auto& row : r.tasks) {
        for (const auto& key : {"id","arrival","burst","priority","start","finish","waiting","turnaround","response"})
            std::cout << member(row, key) << ' ';
        std::cout << '\n';
    }
    std::cout << r.summary.json() << "\nGANTT: ";
    for (const auto& item : r.timeline) {
        const auto& v = array(item); std::cout << '[' << v[0] << ',' << v[1] << "):";
        if (std::holds_alternative<std::nullptr_t>(v[2].value)) std::cout << "IDLE ";
        else std::cout << 'P' << v[2] << ' ';
    }
    std::cout << '\n';
}
int run_scheduler(int argc, char** argv, const std::string& forced) {
    try {
        std::filesystem::path executable = std::filesystem::absolute(argv[0]);
        auto task_path = executable.parent_path() / "tasks.csv";
        if (!std::filesystem::exists(task_path)) task_path = "tasks.csv";
        std::filesystem::path out = "results"; int quantum = 20, seed = 525; bool details = true, compare = false;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--help") {
                std::cout << forced << " [--tasks tasks.csv] [--quantum 20] [--seed 525] [--out results] [--compare] [--summary|--details]\n";
                return 0;
            }
            if (arg == "--summary") { details = false; continue; }
            if (arg == "--details") { details = true; continue; }
            if (arg == "--compare") { compare = true; continue; }
            if (i + 1 >= argc) throw std::invalid_argument("Missing option value");
            std::string value = argv[++i];
            if (arg == "--tasks") task_path = value;
            else if (arg == "--out") out = value;
            else if (arg == "--quantum") quantum = integer(value);
            else if (arg == "--seed") seed = integer(value);
            else throw std::invalid_argument("Unknown argument: " + arg);
        }
        if (quantum <= 0 || seed < 0) throw std::invalid_argument("quantum > 0 and seed >= 0 required");
        std::filesystem::create_directories(out);
        const std::vector<std::string> stats_fields{"algorithm","quantum","avg_waiting","avg_execution","avg_turnaround",
            "avg_response","max_response","throughput","makespan","switches"};
        if (forced == "MQ") {
            Json result = simulate_mq(seed); std::string prefix = "MQ-seed" + std::to_string(seed);
            const auto& result_object = std::get<Json::Object>(result.value);
            const auto& task_rows = std::get<Json::Array>(result_object.at("tasks").value);
            save_json(out / (prefix + ".json"), result);
            save_csv(out / (prefix + "-input.csv"), array(member(result,"inputs")), {"id","arrival","burst","kind","cpu_burst","io_delay"});
            save_csv(out / (prefix + "-migrations.csv"), array(member(result,"events")), {"time","id","source","target","reason"});
            save_csv(out / (prefix + "-queues.csv"), array(member(result,"queues")), {"queue","visited","completed","service","share_percent","avg_queue_wait","avg_full_turnaround_visited"});
            std::vector<std::string> task_fields{"id","kind","arrival","execution","finish","waiting","io","turnaround","response","final_queue"};
            for (int q = 0; q < 3; ++q) for (const auto& suffix : {"_visited","_wait","_cpu"}) task_fields.push_back("Q" + std::to_string(q) + suffix);
            save_csv(out / (prefix + "-all.csv"), task_rows, task_fields);
            std::cout << "MQ: Q0=20 Q1=60 Q2=120; calendar 100/60/40 ms; empty quotas donated; switch cost=0\n"
                      << member(result,"summary") << '\n';
            for (int q = 0; q < 3; ++q) {
                std::string key = "Q" + std::to_string(q); Json::Array records;
                for (const auto& row : task_rows) records.push_back(Json::object({
                    {"id", member(row,"id")}, {"arrival", member(row,"arrival")}, {"execution", member(row,"execution")},
                    {"visited", member(row,key+"_visited")}, {"queue_cpu", member(row,key+"_cpu")},
                    {"queue_wait", member(row,key+"_wait")}, {"full_turnaround", member(row,"turnaround")},
                    {"final_queue", member(row,"final_queue")}}));
                save_csv(out / (prefix + "-" + key + ".csv"), records,
                    {"id","arrival","execution","visited","queue_cpu","queue_wait","full_turnaround","final_queue"});
                std::cout << array(member(result,"queues"))[q] << '\n';
                if (details) {
                    std::cout << key << " PID ARRIVAL CPU_TOTAL CPU_QUEUE WAIT_QUEUE TURNAROUND VISITED\n";
                    for (const auto& row : records) {
                        for (const auto& field : {"id","arrival","execution","queue_cpu","queue_wait","full_turnaround","visited"}) std::cout << member(row,field) << ' ';
                        std::cout << '\n';
                    }
                }
            }
        } else if (compare) {
            auto tasks = load_tasks(task_path); std::vector<Result> results{simulate(tasks,"SJF")};
            for (const auto& alg : {"RR","CP"}) for (int q : {5,10,20,40,80}) results.push_back(simulate(tasks,alg,q));
            Json::Array full, summaries;
            std::cout << "ALG Q AVG_WAIT AVG_EXEC AVG_TURN AVG_RESP MAX_RESP SWITCHES THROUGHPUT\n";
            for (const auto& r : results) {
                full.push_back(r.json()); summaries.push_back(r.summary.json()); const auto& s = r.summary;
                std::cout << s.algorithm << ' ' << s.quantum << ' ' << s.waiting << ' ' << s.execution << ' '
                    << s.turnaround << ' ' << s.response << ' ' << s.max_response << ' ' << s.switches << ' ' << s.throughput << '\n';
            }
            save_json(out / "comparison.json", full); save_csv(out / "comparison.csv", summaries, stats_fields);
            for (const auto& alg : {"RR","CP"}) {
                const Stats* best = nullptr;
                for (const auto& r : results) if (r.summary.algorithm == alg) {
                    const auto& s = r.summary;
                    if (!best || std::tie(s.waiting,s.response,s.switches,s.quantum) < std::tie(best->waiting,best->response,best->switches,best->quantum)) best = &s;
                }
                std::cout << "BEST_" << alg << "_QUANTUM=" << best->quantum << '\n';
            }
        } else {
            Result result = simulate(load_tasks(task_path),forced,quantum);
            std::string prefix = forced == "SJF" ? forced : forced + "-q" + std::to_string(quantum);
            save_json(out / (prefix + ".json"), result.json());
            save_csv(out / (prefix + ".csv"), result.tasks, {"id","arrival","burst","priority","start","finish","waiting","turnaround","response"});
            print_result(result);
        }
        std::cout << "Results: " << std::filesystem::absolute(out).string() << '\n'; return 0;
    } catch (const std::exception& e) { std::cerr << "ERROR: " << e.what() << '\n'; return 1; }
}
} // namespace lab05
