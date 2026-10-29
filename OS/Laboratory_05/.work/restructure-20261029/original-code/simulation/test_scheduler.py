import unittest
from scheduler import Task, load_tasks, simulate, simulate_mq, ROOT


class SchedulerTests(unittest.TestCase):
    def test_sjf_reference_order(self):
        r = simulate(load_tasks(ROOT/'tasks.csv'),'SJF')
        self.assertEqual([p for _,_,p in r['timeline']], [1,4,2,8,6,3,9,7,10,5])
        self.assertEqual(r['summary']['makespan'],625)

    def test_rr_boundary_arrivals(self):
        r=simulate([Task(1,0,4,0),Task(2,2,1,0)],'RR',2)
        self.assertEqual(r['timeline'],[[0,2,1],[2,3,2],[3,5,1]])

    def test_priority_immediate_preemption(self):
        r=simulate([Task(1,0,4,1),Task(2,1,1,5)],'CP',20)
        self.assertEqual(r['timeline'],[[0,1,1],[1,2,2],[2,5,1]])

    def test_sjf_does_not_preempt(self):
        r=simulate([Task(1,0,4,1),Task(2,1,1,5)],'SJF')
        self.assertEqual(r['timeline'],[[0,4,1],[4,5,2]])

    def test_idle_time(self):
        r=simulate([Task(1,5,3,1)],'RR',2)
        self.assertEqual(r['tasks'][0]['waiting'],0)
        self.assertEqual(r['summary']['makespan'],3)

    def test_all_quanta_conserve_work_and_statistics(self):
        for alg in ('SJF','RR','CP'):
            for q in (1,5,10,20,40,80,1000):
                r=simulate(load_tasks(ROOT/'tasks.csv'),alg,q)
                self.assertEqual(r['summary']['makespan'],625)
                self.assertEqual(r['summary']['avg_execution'],62.5)
                self.assertEqual(r['summary']['avg_turnaround'],r['summary']['avg_waiting']+62.5)

    def test_mq_reproducibility_and_accounting(self):
        r=simulate_mq(525)
        self.assertEqual(r,simulate_mq(525))
        self.assertEqual(r['summary']['makespan'],28957)
        self.assertEqual(r['summary']['avg_waiting'],17911.13)
        self.assertEqual(r['summary']['migrations'],397)
        self.assertEqual(len(r['tasks']),100)
        self.assertEqual(sum(q['completed'] for q in r['queues']),100)
        self.assertEqual(sum(q['service'] for q in r['queues']),r['summary']['cpu_service'])
        self.assertTrue(all(p['finish']>p['arrival'] for p in r['tasks']))
        self.assertTrue(any(p['io']>0 for p in r['tasks']))
        self.assertTrue(any(e['reason']=='io-return' for e in r['events']))
        q2=[e for e in r['events'] if e['source']=='Q2' and e['reason']=='quantum']
        self.assertTrue({'Q0','Q1'} <= {e['target'] for e in q2})


if __name__=='__main__': unittest.main()
