#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
mkdir -p bin
for part in a x b c; do
    g++ -std=c++17 -O2 -Wall -Wextra -pthread "Lab-05${part}.cpp" -o "bin/Lab-05${part}" -lrt
done
printf 'Build complete. Executables are in native/bin.\n'
