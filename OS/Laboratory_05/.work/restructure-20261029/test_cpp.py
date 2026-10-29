from pathlib import Path
import json,subprocess,sys,re,importlib.util,csv
W=Path(__file__).resolve().parent;R=W.parents[1];BIN=R/'Laboratory_05/windows/bin/x64/Release'
spec=importlib.util.spec_from_file_location('scheduler_reference',W/'original-code/simulation/scheduler.py')
ref=importlib.util.module_from_spec(spec);sys.modules[spec.name]=ref;spec.loader.exec_module(ref)
def run(name,args,ok=True):
 p=subprocess.run([str(BIN/(name+'.exe')),*map(str,args)],capture_output=True,text=True,encoding='utf8',timeout=30)
 assert (p.returncode==0)==ok,(name,p.stdout,p.stderr)
 return p.stdout
out=W/'cpp-tests';out.mkdir(exist_ok=True)
run('Lab-05RR',['--compare','--out',out])
assert json.loads((out/'comparison.json').read_text())==json.loads((W/'original-code/simulation/results/comparison.json').read_text())
for seed in [0,1,525,999]:
 run('Lab-05MQ',['--seed',seed,'--summary','--out',out])
 result=json.loads((out/f'MQ-seed{seed}.json').read_text())
 assert result==ref.simulate_mq(seed),seed
 for q in range(3):
  with (out/f'MQ-seed{seed}-Q{q}.csv').open() as f:assert len(list(csv.DictReader(f)))==100
cases=[[(1,0,4,0),(2,2,1,0)],[(1,0,4,1),(2,1,1,5)],[(1,5,3,1)],[(1,0,1,1),(2,10,2,1)],[(1,0,3,1),(2,0,3,1)]]
for index,rows in enumerate(cases):
 taskfile=out/f'tasks-{index}.csv'
 with taskfile.open('w',newline='') as f:
  w=csv.writer(f);w.writerow(['id','arrival','burst','priority']);w.writerows(rows)
 for alg in ['SJF','RR','CP']:
  for quantum in [1,2,20]:
   run('Lab-05'+alg,['--tasks',taskfile,'--quantum',quantum,'--out',out])
   prefix='SJF' if alg=='SJF' else f'{alg}-q{quantum}'
   got=json.loads((out/(prefix+'.json')).read_text())
   expected=ref.simulate([ref.Task(*v) for v in rows],alg,quantum)
   assert got==expected,(index,alg,quantum)
run('Lab-05RR',['--quantum','0'],ok=False)
run('Lab-05MQ',['--seed','-1'],ok=False)
info=run('Lab-05a',[]);mask=int(re.search(r'ONE_CPU_MASK_DEC=(\d+)',info)[1])
assert 'BASE=' in info and 'PROCESS_MASK_BIN=' in info
assert 'ITER=2000' in run('Lab-05x',[2000,0])
threads=run('Lab-05c',[mask,32,0,0,10000,1])
assert 'FIRST_FINISH SLOT=' in threads and threads.count('FINISHED PID=')==2
run('Lab-05c',[0,32,0,0,10,0],ok=False)
report={'windows_projects':8,'reference_comparison_exact':True,'mq_seeds':[0,1,525,999],'mq_full_result_exact':True,'boundary_cases':45,'native_info_workload_threads':True,'invalid_arguments_rejected':True}
(W/'test-results.json').write_text(json.dumps(report,indent=2),encoding='utf8')
print(json.dumps(report,indent=2))
