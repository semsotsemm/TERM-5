sudo apt update
sudo apt install -y build-essential python3 unzip
mkdir -p "$HOME/OS-Lab05"
unzip "$HOME/Downloads/Lab05-kit.zip" -d "$HOME/OS-Lab05"
cd "$HOME/OS-Lab05"
bash native/build-linux.sh


clear
cat /etc/os-release
uname -r
lscpu | grep -E 'Model name|CPU\(s\)|Thread|Core|Socket'
python3 -c 'import os; print("Allowed CPU IDs:", sorted(os.sched_getaffinity(0)))'


clear
./native/bin/Lab-05a
all_mask=$(python3 -c 'import os; print(sum(1 << c for c in os.sched_getaffinity(0)))')
one_mask=$(python3 -c 'import os; print(1 << min(os.sched_getaffinity(0)))')
printf 'AllMask=%s OneMask=%s\n' "$all_mask" "$one_mask"


clear
./native/bin/Lab-05x 1000000 200


sudo -v


sudo -v
sudo ./native/bin/Lab-05b "$all_mask" 0 0 1000000 200 > linux-b1.log 2>&1 &
launcher_pid=$!
sleep 1
head -n 5 linux-b1.log
lab_pid=$(sed -n 's/^PARENT_PID=\([0-9]*\).*/\1/p' linux-b1.log | head -n 1)
printf 'Lab PID=%s\n' "$lab_pid"


p1=$(sed -n 's/^CHILD1 PID=\([0-9]*\).*/\1/p' linux-b1.log | head -n 1)
p2=$(sed -n 's/^CHILD2 PID=\([0-9]*\).*/\1/p' linux-b1.log | head -n 1)
ps -p "$lab_pid,$p1,$p2" -o pid,ppid,cls,rtprio,pri,ni,psr,stat,comm
grep -E '^(Name|Pid|Tgid|Cpus_allowed|Cpus_allowed_list):' "/proc/$p1/status" "/proc/$p2/status"
awk '{print FILENAME, "priority=" $18, "nice=" $19}' "/proc/$p1/stat" "/proc/$p2/stat"


while ! grep -q '^FIRST_FINISH' linux-b1.log; do if grep -q '^ERROR:' linux-b1.log; then tail -n 10 linux-b1.log; break; fi; sleep 1; done
head -n 1 linux-b1.log
grep -A 1 '^FIRST_FINISH' linux-b1.log


wait "$launcher_pid"
echo "ExitCode=$?"
grep '^FINISHED' linux-b1.log


sudo -v
sudo ./native/bin/Lab-05b "$all_mask" 19 -20 1000000 200 > linux-b2.log 2>&1 &
launcher_pid=$!
sleep 1
head -n 5 linux-b2.log
lab_pid=$(sed -n 's/^PARENT_PID=\([0-9]*\).*/\1/p' linux-b2.log | head -n 1)
printf 'Lab PID=%s\n' "$lab_pid"


p1=$(sed -n 's/^CHILD1 PID=\([0-9]*\).*/\1/p' linux-b2.log | head -n 1)
p2=$(sed -n 's/^CHILD2 PID=\([0-9]*\).*/\1/p' linux-b2.log | head -n 1)
ps -p "$lab_pid,$p1,$p2" -o pid,ppid,cls,rtprio,pri,ni,psr,stat,comm
grep -E '^(Name|Pid|Tgid|Cpus_allowed|Cpus_allowed_list):' "/proc/$p1/status" "/proc/$p2/status"
awk '{print FILENAME, "priority=" $18, "nice=" $19}' "/proc/$p1/stat" "/proc/$p2/stat"


while ! grep -q '^FIRST_FINISH' linux-b2.log; do if grep -q '^ERROR:' linux-b2.log; then tail -n 10 linux-b2.log; break; fi; sleep 1; done
head -n 1 linux-b2.log
grep -A 1 '^FIRST_FINISH' linux-b2.log


wait "$launcher_pid"
echo "ExitCode=$?"
grep '^FINISHED' linux-b2.log


sudo -v
sudo ./native/bin/Lab-05b "$one_mask" 19 -20 1000000 200 > linux-b3.log 2>&1 &
launcher_pid=$!
sleep 1
head -n 5 linux-b3.log
lab_pid=$(sed -n 's/^PARENT_PID=\([0-9]*\).*/\1/p' linux-b3.log | head -n 1)
printf 'Lab PID=%s\n' "$lab_pid"


p1=$(sed -n 's/^CHILD1 PID=\([0-9]*\).*/\1/p' linux-b3.log | head -n 1)
p2=$(sed -n 's/^CHILD2 PID=\([0-9]*\).*/\1/p' linux-b3.log | head -n 1)
ps -p "$lab_pid,$p1,$p2" -o pid,ppid,cls,rtprio,pri,ni,psr,stat,comm
grep -E '^(Name|Pid|Tgid|Cpus_allowed|Cpus_allowed_list):' "/proc/$p1/status" "/proc/$p2/status"
awk '{print FILENAME, "priority=" $18, "nice=" $19}' "/proc/$p1/stat" "/proc/$p2/stat"


while ! grep -q '^FIRST_FINISH' linux-b3.log; do if grep -q '^ERROR:' linux-b3.log; then tail -n 10 linux-b3.log; break; fi; sleep 1; done
head -n 1 linux-b3.log
grep -A 1 '^FIRST_FINISH' linux-b3.log


wait "$launcher_pid"
echo "ExitCode=$?"
grep '^FINISHED' linux-b3.log


sudo -v
sudo ./native/bin/Lab-05c "$all_mask" 0 0 1000000 200 > linux-c1.log 2>&1 &
launcher_pid=$!
sleep 1
head -n 5 linux-c1.log
lab_pid=$(sed -n 's/^PARENT_PID=\([0-9]*\).*/\1/p' linux-c1.log | head -n 1)
printf 'Lab PID=%s\n' "$lab_pid"


ps -L -p "$lab_pid" -o pid,tid,cls,rtprio,pri,ni,psr,stat,comm
for task in /proc/"$lab_pid"/task/*; do grep -E '^(Name|Pid|Tgid|Cpus_allowed|Cpus_allowed_list):' "$task/status"; done
for task in /proc/"$lab_pid"/task/*; do awk '{print FILENAME, "priority=" $18, "nice=" $19}' "$task/stat"; done


while ! grep -q '^FIRST_FINISH' linux-c1.log; do if grep -q '^ERROR:' linux-c1.log; then tail -n 10 linux-c1.log; break; fi; sleep 1; done
head -n 1 linux-c1.log
grep -A 1 '^FIRST_FINISH' linux-c1.log


wait "$launcher_pid"
echo "ExitCode=$?"
grep '^FINISHED' linux-c1.log


sudo -v
sudo ./native/bin/Lab-05c "$all_mask" 19 -20 1000000 200 > linux-c2.log 2>&1 &
launcher_pid=$!
sleep 1
head -n 5 linux-c2.log
lab_pid=$(sed -n 's/^PARENT_PID=\([0-9]*\).*/\1/p' linux-c2.log | head -n 1)
printf 'Lab PID=%s\n' "$lab_pid"


ps -L -p "$lab_pid" -o pid,tid,cls,rtprio,pri,ni,psr,stat,comm
for task in /proc/"$lab_pid"/task/*; do grep -E '^(Name|Pid|Tgid|Cpus_allowed|Cpus_allowed_list):' "$task/status"; done
for task in /proc/"$lab_pid"/task/*; do awk '{print FILENAME, "priority=" $18, "nice=" $19}' "$task/stat"; done


while ! grep -q '^FIRST_FINISH' linux-c2.log; do if grep -q '^ERROR:' linux-c2.log; then tail -n 10 linux-c2.log; break; fi; sleep 1; done
head -n 1 linux-c2.log
grep -A 1 '^FIRST_FINISH' linux-c2.log


wait "$launcher_pid"
echo "ExitCode=$?"
grep '^FINISHED' linux-c2.log


sudo -v
sudo ./native/bin/Lab-05c "$one_mask" 19 -20 1000000 200 > linux-c3.log 2>&1 &
launcher_pid=$!
sleep 1
head -n 5 linux-c3.log
lab_pid=$(sed -n 's/^PARENT_PID=\([0-9]*\).*/\1/p' linux-c3.log | head -n 1)
printf 'Lab PID=%s\n' "$lab_pid"


ps -L -p "$lab_pid" -o pid,tid,cls,rtprio,pri,ni,psr,stat,comm
for task in /proc/"$lab_pid"/task/*; do grep -E '^(Name|Pid|Tgid|Cpus_allowed|Cpus_allowed_list):' "$task/status"; done
for task in /proc/"$lab_pid"/task/*; do awk '{print FILENAME, "priority=" $18, "nice=" $19}' "$task/stat"; done


while ! grep -q '^FIRST_FINISH' linux-c3.log; do if grep -q '^ERROR:' linux-c3.log; then tail -n 10 linux-c3.log; break; fi; sleep 1; done
head -n 1 linux-c3.log
grep -A 1 '^FIRST_FINISH' linux-c3.log


wait "$launcher_pid"
echo "ExitCode=$?"
grep '^FINISHED' linux-c3.log


cd "$HOME/OS-Lab05"
python3 -m unittest discover -s simulation -v


clear
python3 simulation/Lab-05SJF.py


clear
python3 simulation/Lab-05RR.py --quantum 80


clear
python3 simulation/Lab-05CP.py --quantum 80


clear
python3 simulation/scheduler.py compare


clear
python3 simulation/Lab-05MQ.py --seed 525


clear
python3 -c 'import csv,pathlib; root=pathlib.Path("simulation/results"); [(print("Q"+str(q),"rows=",len(rows)),print("first 3:",rows[:3])) for q in range(3) for rows in [list(csv.DictReader((root/("MQ-seed525-Q"+str(q)+".csv")).open(encoding="utf-8-sig")))]]'


python3 simulation/Lab-05MQ.py --seed 525 --details > simulation/results/MQ-full.txt


clear
python3 -c 'import csv,pathlib; rows=list(csv.DictReader(pathlib.Path("simulation/results/MQ-seed525-migrations.csv").open(encoding="utf-8-sig"))); print("FIRST QUANTUM MOVES:"); [print(r) for r in [r for r in rows if r["reason"]=="quantum"][:5]]; print("IO RETURN PROMOTIONS:"); [print(r) for r in [r for r in rows if r["reason"]=="io-return"][:5]]; print("Q2 RETURNS:"); [print(r) for r in [r for r in rows if r["source"]=="Q2"][:5]]'
