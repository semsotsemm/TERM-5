"""Deterministic teaching simulators. Integer model time; no actual jobs execute."""
import argparse
from collections import deque
import csv
from dataclasses import dataclass, field, asdict
import json
from pathlib import Path
import random
from statistics import mean

ROOT = Path(__file__).resolve().parent


@dataclass
class Task:
    id: int
    arrival: int
    burst: int
    priority: int


def load_tasks(path):
    with Path(path).open(encoding="utf-8-sig", newline="") as stream:
        tasks = [Task(**{k: int(v) for k, v in row.items()}) for row in csv.DictReader(stream)]
    if not tasks or len({p.id for p in tasks}) != len(tasks):
        raise ValueError("Tasks must have distinct IDs")
    if any(p.arrival < 0 or p.burst <= 0 for p in tasks):
        raise ValueError("arrival >= 0 and burst > 0 required")
    return sorted(tasks, key=lambda p: (p.arrival, p.id))


def simulate(tasks, algorithm, quantum=20):
    if algorithm not in ("SJF", "RR", "CP") or quantum <= 0:
        raise ValueError("Invalid algorithm or quantum")
    tasks = sorted(tasks, key=lambda p: (p.arrival, p.id))
    remaining = {p.id: p.burst for p in tasks}
    start, finish, timeline = {}, {}, []
    queues = {}
    time = 0
    pending = 0
    active = None
    credit = 0
    switches = 0
    last_running = None

    def enqueue(p, front=False):
        rank = p.priority if algorithm == "CP" else 0
        queue = queues.setdefault(rank, deque())
        queue.appendleft(p) if front else queue.append(p)

    def append_tick(pid):
        if timeline and timeline[-1][2] == pid and timeline[-1][1] == time:
            timeline[-1][1] += 1
        else:
            timeline.append([time, time + 1, pid])

    while len(finish) < len(tasks):
        while pending < len(tasks) and tasks[pending].arrival <= time:
            enqueue(tasks[pending]); pending += 1
        if active and algorithm == "CP":
            highest = max((rank for rank, queue in queues.items() if queue), default=-float("inf"))
            if highest > active.priority:
                # A higher-priority arrival preempts immediately. A new dispatch
                # gets a fresh quantum; the interrupted task heads its peer queue.
                enqueue(active, front=True); active = None
        if active is None:
            ranks = [rank for rank, queue in queues.items() if queue]
            if not ranks:
                append_tick(None); time += 1; last_running = None; continue
            if algorithm == "SJF":
                queue = queues[0]
                active = min(queue, key=lambda p: (p.burst, p.arrival, p.id))
                queue.remove(active)
            else:
                active = queues[max(ranks)].popleft()
            credit = quantum
            start.setdefault(active.id, time)
        if last_running is not None and active.id != last_running:
            switches += 1
        last_running = active.id
        append_tick(active.id)
        remaining[active.id] -= 1
        credit -= 1
        time += 1
        # Arrivals at the boundary are queued before an expired task is requeued.
        while pending < len(tasks) and tasks[pending].arrival <= time:
            enqueue(tasks[pending]); pending += 1
        if remaining[active.id] == 0:
            finish[active.id] = time; active = None
        elif algorithm != "SJF" and credit == 0:
            enqueue(active); active = None

    rows = []
    for p in sorted(tasks, key=lambda p: p.id):
        turnaround = finish[p.id] - p.arrival
        rows.append(dict(id=p.id, arrival=p.arrival, burst=p.burst, priority=p.priority,
                         start=start[p.id], finish=finish[p.id], waiting=turnaround - p.burst,
                         turnaround=turnaround, response=start[p.id] - p.arrival))
    duration = time - min(p.arrival for p in tasks)
    summary = dict(algorithm=algorithm, quantum=0 if algorithm == "SJF" else quantum,
                   avg_waiting=mean(p["waiting"] for p in rows),
                   avg_execution=mean(p.burst for p in tasks),
                   avg_turnaround=mean(p["turnaround"] for p in rows),
                   avg_response=mean(p["response"] for p in rows),
                   max_response=max(p["response"] for p in rows),
                   throughput=len(tasks) / duration, makespan=duration, switches=switches)
    assert all(r["waiting"] >= 0 and r["turnaround"] == r["waiting"] + r["burst"] for r in rows)
    assert sum(end - begin for begin, end, pid in timeline if pid is not None) == sum(p.burst for p in tasks)
    return dict(summary=summary, tasks=rows, timeline=timeline)


def save_csv(path, rows):
    with Path(path).open("w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader(); writer.writerows(rows)


def print_result(result):
    s = result["summary"]
    print(f"ALGORITHM={s['algorithm']} QUANTUM={s['quantum']} SWITCH_COST=0 HIGHER_NUMBER_HIGHER_PRIORITY")
    print("ID ARR CPU PRI START FINISH WAIT TURN RESPONSE")
    for r in result["tasks"]:
        print(" ".join(f"{r[k]:>5}" for k in ("id", "arrival", "burst", "priority", "start", "finish", "waiting", "turnaround", "response")))
    print(" ".join(f"{k}={v:.6f}" if isinstance(v, float) else f"{k}={v}" for k, v in s.items()))
    print("GANTT: " + " ".join(f"[{a},{b}):P{p}" if p is not None else f"[{a},{b}):IDLE" for a,b,p in result["timeline"]))


@dataclass
class MQTask:
    id: int
    arrival: int
    burst: int
    kind: str
    cpu_burst: int
    io_delay: int
    remaining: int = 0
    queue: int = 0
    credit: int = 20
    phase: int = 0
    unblock: int = -1
    finish: int = -1
    first: int = -1
    io_total: int = 0
    waiting: list = field(default_factory=lambda: [0, 0, 0])
    service: list = field(default_factory=lambda: [0, 0, 0])
    visited: list = field(default_factory=lambda: [False, False, False])


def simulate_mq(seed=525, count=100):
    rng = random.Random(seed)
    tasks = []
    for i in range(1, count + 1):
        io = rng.random() < 0.5
        burst = rng.randint(80, 250) if io else rng.randint(120, 600)
        p = MQTask(i, rng.randint(0, 200), burst, "IO" if io else "CPU",
                   rng.choice([rng.randint(2, 8), rng.randint(2, 8), rng.randint(24, 35)]) if io else burst,
                   rng.randint(10, 40) if io else 0)
        p.remaining = p.burst; p.phase = p.cpu_burst
        tasks.append(p)
    inputs = [dict(id=p.id, arrival=p.arrival, burst=p.burst, kind=p.kind,
                   cpu_burst=p.cpu_burst, io_delay=p.io_delay) for p in tasks]
    queues = [deque(), deque(), deque()]
    quanta = [20, 60, 120]
    calendar = [0,0,0,0,0,1,1,1,2,2]  # 20-ms slots: 100/60/40 ms per 200-ms frame.
    time = 0
    done = 0
    switches = migrations = idle = 0
    previous = None
    events = []
    timeline = []

    def enqueue(p, queue, reason, reset=True):
        nonlocal migrations
        old = p.queue
        if old != queue:
            migrations += 1
            events.append(dict(time=time, id=p.id, source=f"Q{old}", target=f"Q{queue}", reason=reason))
        p.queue = queue
        if reset: p.credit = quanta[queue]
        p.visited[queue] = True
        queues[queue].append(p)

    while done < count:
        for p in tasks:
            if p.arrival == time:
                enqueue(p, 0, "arrival")
            elif p.unblock == time:
                p.unblock = -1
                p.phase = min(p.cpu_burst, p.remaining)
                enqueue(p, max(0, p.queue - 1), "io-return")
        requested = calendar[(time // 20) % len(calendar)]
        selected = requested if queues[requested] else next((q for q in range(3) if queues[q]), None)
        if selected is None:
            idle += 1; previous = None; time += 1; continue
        p = queues[selected][0]
        if previous is not None and previous != p.id: switches += 1
        previous = p.id
        if p.first < 0: p.first = time
        for queue in queues:
            for other in queue:
                if other is not p: other.waiting[other.queue] += 1
        p.remaining -= 1; p.phase -= 1; p.credit -= 1; p.service[selected] += 1
        if timeline and timeline[-1][2:] == [p.id, selected] and timeline[-1][1] == time:
            timeline[-1][1] += 1
        else:
            timeline.append([time, time + 1, p.id, selected])
        time += 1
        if p.remaining == 0:
            queues[selected].popleft(); p.finish = time; done += 1
        elif p.kind == "IO" and p.phase == 0:
            queues[selected].popleft(); p.unblock = time + p.io_delay; p.io_total += p.io_delay
        elif p.credit == 0:
            queues[selected].popleft()
            target = selected + 1 if selected < 2 else rng.randrange(3)
            enqueue(p, target, "quantum")

    rows = []
    for p in tasks:
        row = dict(id=p.id, kind=p.kind, arrival=p.arrival, execution=p.burst, finish=p.finish,
                   waiting=sum(p.waiting), io=p.io_total, turnaround=p.finish-p.arrival,
                   response=p.first-p.arrival, final_queue=f"Q{p.queue}")
        for q in range(3): row.update({f"Q{q}_visited":int(p.visited[q]), f"Q{q}_wait":p.waiting[q], f"Q{q}_cpu":p.service[q]})
        assert row["turnaround"] == row["waiting"] + row["execution"] + row["io"]
        rows.append(row)
    cpu_total = sum(p.burst for p in tasks)
    assert cpu_total + idle == time
    queue_rows = []
    for q in range(3):
        visited = [p for p in tasks if p.visited[q]]
        queue_rows.append(dict(queue=f"Q{q}", visited=len(visited), completed=sum(p.queue==q for p in tasks),
                               service=sum(p.service[q] for p in tasks),
                               share_percent=100*sum(p.service[q] for p in tasks)/cpu_total,
                               avg_queue_wait=mean(p.waiting[q] for p in visited) if visited else 0,
                               avg_full_turnaround_visited=mean(p.finish-p.arrival for p in visited) if visited else 0))
    summary = dict(seed=seed, count=count, avg_waiting=mean(r["waiting"] for r in rows),
                   avg_turnaround=mean(r["turnaround"] for r in rows),
                   avg_response=mean(r["response"] for r in rows), makespan=time,
                   cpu_service=cpu_total, idle=idle, utilization_percent=100*cpu_total/time,
                   switches=switches, migrations=migrations)
    return dict(summary=summary, queues=queue_rows, tasks=rows, inputs=inputs,
                events=events, timeline=timeline)


def main(forced=None):
    parser = argparse.ArgumentParser(description=__doc__)
    if forced is None: parser.add_argument("algorithm", choices=["SJF","RR","CP","MQ","compare"])
    parser.add_argument("--tasks", type=Path, default=ROOT/"tasks.csv")
    parser.add_argument("--quantum", type=int, default=20)
    parser.add_argument("--seed", type=int, default=525)
    parser.add_argument("--details", action="store_true", help="Print all 100 process rows for each MQ queue")
    parser.add_argument("--out", type=Path, default=ROOT/"results")
    args = parser.parse_args()
    algorithm = forced or args.algorithm
    args.out.mkdir(parents=True, exist_ok=True)
    if algorithm == "MQ":
        result = simulate_mq(args.seed)
        prefix = f"MQ-seed{args.seed}"
        (args.out/f"{prefix}.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
        save_csv(args.out/f"{prefix}-all.csv",result["tasks"])
        save_csv(args.out/f"{prefix}-queues.csv",result["queues"])
        save_csv(args.out/f"{prefix}-input.csv",result["inputs"])
        save_csv(args.out/f"{prefix}-migrations.csv",result["events"])
        print("MQ: Q0=20 Q1=60 Q2=120; calendar 100/60/40 ms; empty quotas donated; switch cost=0")
        print(json.dumps(result["summary"],indent=2))
        for q,stats in enumerate(result["queues"]):
            save_csv(args.out/f"{prefix}-Q{q}.csv",[dict(id=r["id"],arrival=r["arrival"],execution=r["execution"],
                visited=r[f"Q{q}_visited"],queue_cpu=r[f"Q{q}_cpu"],queue_wait=r[f"Q{q}_wait"],
                full_turnaround=r["turnaround"],final_queue=r["final_queue"]) for r in result["tasks"]])
            print(json.dumps(stats))
            print(f"Q{q}: all 100 process rows are in {prefix}-Q{q}.csv")
            if args.details:
                print(f"Q{q} PID ARRIVAL CPU_TOTAL CPU_QUEUE WAIT_QUEUE TURNAROUND VISITED")
                for r in result["tasks"]:
                    print(r['id'],r['arrival'],r['execution'],r[f'Q{q}_cpu'],r[f'Q{q}_wait'],r['turnaround'],r[f'Q{q}_visited'])
        print("First 8 of 100 process records:")
        for row in result["tasks"][:8]: print(json.dumps(row))
    elif algorithm == "compare":
        tasks = load_tasks(args.tasks)
        results = [simulate(tasks,"SJF")]
        results += [simulate(tasks,alg,q) for alg in ("RR","CP") for q in (5,10,20,40,80)]
        (args.out/"comparison.json").write_text(json.dumps(results,indent=2),encoding="utf-8")
        save_csv(args.out/"comparison.csv",[r["summary"] for r in results])
        print("ALG   Q   AVG_WAIT AVG_EXEC AVG_TURN AVG_RESP MAX_RESP SWITCHES THROUGHPUT")
        for r in results:
            s=r["summary"]
            print(f"{s['algorithm']:3} {s['quantum']:3} {s['avg_waiting']:9.2f} {s['avg_execution']:8.2f} {s['avg_turnaround']:8.2f} "
                  f"{s['avg_response']:8.2f} {s['max_response']:8} {s['switches']:8} {s['throughput']:.6f}")
        print("Selection rule: minimum average waiting, then minimum average response, then fewer switches.")
        for alg in ("RR","CP"):
            best=min((r["summary"] for r in results if r["summary"]["algorithm"]==alg),
                     key=lambda s:(s["avg_waiting"],s["avg_response"],s["switches"],s["quantum"]))
            print(f"BEST_{alg}_QUANTUM={best['quantum']}")
    else:
        result = simulate(load_tasks(args.tasks),algorithm,args.quantum)
        prefix=f"{algorithm}-q{args.quantum}" if algorithm!="SJF" else "SJF"
        (args.out/f"{prefix}.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
        save_csv(args.out/f"{prefix}.csv",result["tasks"])
        print_result(result)
    print(f"Results: {args.out.resolve()}")


if __name__ == "__main__":
    main()
