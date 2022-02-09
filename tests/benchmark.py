
from sortedintersect import intersect
import time
import quicksect
from quicksect import Interval
from ailist import AIList
from ncls import NCLS
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


def benchmark(n):
    print(f'N = {n}')

    res = []
    intervals = []
    bases = 0

    for i in range(0, n, 1_000):
        intervals.append((i, i+100))
        bases += 100
    intervals = np.array(intervals)

    # sortedintersect
    itv = intersect.IntervalSet(False)
    itv.add_from_iter(intervals)

    t0 = time.time()
    v = 0
    for i in range(n):
        v += int(itv.search_point(i))
    res.append({'library': 'sortedintersect', 'time (s)': time.time() - t0, 'v': v, "n": n})

    # quicksect
    tree = quicksect.IntervalTree()
    for s, e in intervals:
        tree.add(s, e)

    t0 = time.time()
    v = 0
    for i in range(n):
        if tree.find(Interval(i, i)):
            v += 1
    res.append({'library': 'quicksect', 'time (s)': time.time() - t0, 'v': v, "n": n})

    # ncls
    starts = pd.Series(intervals[:, 0])
    ends = pd.Series(intervals[:, 1])
    tree = NCLS(starts, ends, starts)

    t0 = time.time()
    v = 0
    for i in range(n):
        if any(tree.find_overlap(i, i)):
            v += 1
    res.append({'library': 'ncls', 'time (s)': time.time() - t0, 'v': v, "n": n})

    # ailist
    tree = AIList()
    for s, e in intervals:
        tree.add(s, e)

    t0 = time.time()
    v = 0
    for i in range(n):
        if tree.intersect(i, i):
            v += 1
    res.append({'library': 'ailist', 'time (s)': time.time() - t0, 'v': v, "n": n})

    df = pd.DataFrame.from_records(res)
    return df


dfs = []
for n in (100_000, 1_000_000, 10_000_000, 100_000_000):
    dfs.append(benchmark(n))

df = pd.concat(dfs)
print(df)

plt.figure()
for name, grp in df.groupby('library'):
    plt.plot(grp['n'], grp['time (s)'], '-o', label=name)
plt.xscale('log')
plt.yscale('log')
plt.legend()
plt.savefig('benchmark.png')
plt.show()