"""Run from repository root: python3 tests/c_verify.py.

Compare C output with independent, tick-by-tick Python reference schedulers.
Uses Python standard library and gcc; creates its compiled library in /tmp.
"""
import ctypes as ct
from collections import deque
from pathlib import Path
import random
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

class Process(ct.Structure):
    _fields_ = [(name, ct.c_int) for name in (
        'pid', 'arrival_time', 'burst_time', 'priority', 'remaining_time',
        'completion_time', 'turnaround_time', 'waiting_time', 'response_time')]

class Entry(ct.Structure):
    _fields_ = [(name, ct.c_int) for name in ('pid', 'start', 'end')]

def reference(rows, mode, quantum=2):
    remaining = [r[2] for r in rows]
    timeline = []
    ready = deque()
    current = None
    used = 0
    time = 0
    while any(remaining):
        if mode == 'rr':
            for i, row in enumerate(rows):
                if row[1] == time:
                    ready.append(i)
            # Arrivals at this tick enter before requeueing the old process.
            if current is not None and (remaining[current] == 0 or used == quantum):
                if remaining[current] > 0:
                    ready.append(current)
                current = None
            if current is None and ready:
                current = ready.popleft()
                used = 0
        else:
            candidates = [i for i, r in enumerate(rows)
                          if r[1] <= time and remaining[i] > 0]
            if mode == 'pre' or current is None or remaining[current] == 0:
                current = min(candidates, key=lambda i: (rows[i][3], rows[i][1], i),
                              default=None)
        if current is None:
            timeline.append(-1)
        else:
            timeline.append(rows[current][0])
            remaining[current] -= 1
            used += 1
        time += 1
    result = []
    for time, pid in enumerate(timeline):
        if result and result[-1][0] == pid:
            result[-1] = (pid, result[-1][1], time + 1)
        else:
            result.append((pid, time, time + 1))
    return result

def main():
    cases = []
    for path in sorted((ROOT / 'tests').glob('c_*.txt')):
        rows = [tuple(map(int, s.split())) for s in path.read_text().splitlines()
                if s.strip() and not s.lstrip().startswith('#')]
        cases.append(rows)
    rng = random.Random(20261028)
    for _ in range(200):
        n = rng.randint(1, 12)
        pids = rng.sample(range(10, 999), n)
        cases.append([(pid, rng.randint(0, 25), rng.randint(1, 12), rng.randint(0, 4))
                      for pid in pids])
    cases += [
        [(1000 - i, 0, 2, i % 3) for i in range(100)],
        [(10, 0, 4096, 0)],
        [(10, 4095, 1, 0)],
        [(10, 0, 2048, 0), (20, 0, 2048, 0)],
    ]
    with tempfile.TemporaryDirectory(prefix='cpu-c-check-') as temp:
        library_path = Path(temp) / 'scheduler.so'
        sources = sorted(str(p) for p in (ROOT / 'src').glob('*.c') if p.name != 'main.c')
        subprocess.run(['gcc', '-shared', '-fPIC', '-std=c11', '-Wall', '-Wextra',
                        '-Werror', '-I' + str(ROOT / 'include'), *sources,
                        '-o', str(library_path)], check=True)
        lib = ct.CDLL(str(library_path))
        signature = [ct.POINTER(Process), ct.c_int, ct.POINTER(Entry),
                     ct.POINTER(ct.c_int), ct.c_int]
        lib.schedule_priority.argtypes = signature
        lib.schedule_priority.restype = None
        lib.schedule_rr.argtypes = signature
        lib.schedule_rr.restype = None
        checks = 0
        for case_number, rows in enumerate(cases):
            for mode, arg in [('nonpre', 0), ('pre', 1), ('rr', 1), ('rr', 2), ('rr', 4), ('rr', 4096)]:
                processes = (Process * len(rows))(*(Process(*r) for r in rows))
                gantt = (Entry * 4096)()
                length = ct.c_int()
                expected = reference(rows, mode, arg)
                fn = lib.schedule_rr if mode == 'rr' else lib.schedule_priority
                for repeat in range(2):
                    fn(processes, len(rows), gantt, ct.byref(length), arg)
                    assert 0 < length.value <= 4096
                    actual = [(g.pid, g.start, g.end) for g in gantt[:length.value]]
                    assert actual == expected, (case_number, mode, arg, repeat, actual, expected)
                    assert all(p.remaining_time == 0 for p in processes)
                    assert [(p.pid, p.arrival_time, p.burst_time, p.priority)
                            for p in processes] == rows
                    checks += 1
        # Invalid quantum must return an empty Gantt, not hang.
        for quantum in [0, -1]:
            processes = (Process * 1)(Process(1, 0, 2, 0))
            length = ct.c_int(99)
            lib.schedule_rr(processes, 1, gantt, ct.byref(length), quantum)
            assert length.value == 0
        print(f'PASS: {len(cases)} inputs, {checks} algorithm runs; invalid quantum checked.')
        print('Checked Gantt, tie rules, idle, quantum boundaries, 100 processes,')
        print('4096 time limit, input order preservation, and repeat calls.')

if __name__ == '__main__':
    main()
