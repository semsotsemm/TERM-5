#!/usr/bin/env bash
set -euo pipefail
cd /work
./bin/Lab-05a > /checks/linux-info.txt
mask=$(sed -n 's/^ONE_CPU_MASK_DEC=//p' /checks/linux-info.txt)
test -n "$mask"
./bin/Lab-05x 2000 0 > /checks/linux-workload.txt
./bin/Lab-05b "$mask" 0 0 10000 1 > /checks/linux-processes.txt
./bin/Lab-05c "$mask" 0 0 10000 1 > /checks/linux-threads.txt
./bin/Lab-05c "$mask" 19 -20 10000 1 > /checks/linux-priorities.txt
./bin/Lab-05SJF --out /checks/linux-results > /checks/linux-sjf.txt
./bin/Lab-05RR --compare --out /checks/linux-results > /checks/linux-compare.txt
./bin/Lab-05CP --quantum 80 --out /checks/linux-results > /checks/linux-cp.txt
./bin/Lab-05MQ --summary --out /checks/linux-results > /checks/linux-mq.txt
grep -q FIRST_FINISH /checks/linux-processes.txt
grep -q FIRST_FINISH /checks/linux-threads.txt
grep -q NICE=-20 /checks/linux-priorities.txt
printf 'PASS: all 8 Linux programs, processes, threads, affinity and negative nice\n'
