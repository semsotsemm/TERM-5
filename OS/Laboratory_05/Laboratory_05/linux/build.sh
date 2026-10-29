#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
mkdir -p bin
for name in Lab-05a Lab-05x Lab-05b Lab-05c Lab-05SJF Lab-05RR Lab-05CP Lab-05MQ; do
    extra=()
    case "$name" in Lab-05SJF|Lab-05RR|Lab-05CP|Lab-05MQ) extra=(common/Scheduler.cpp);; esac
    g++ -std=c++17 -O2 -Wall -Wextra -pthread "$name/$name.cpp" "${extra[@]}" -o "bin/$name" -lrt
done
cp tasks.csv bin/tasks.csv
printf 'Build complete: %s/bin
' "$PWD"
