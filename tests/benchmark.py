import random
import subprocess
from sortedintersect import IntervalSet
import time
import quicksect
from quicksect import Interval
from ncls import NCLS
import cgranges as cr
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

random.seed(0)

def make_random_bedtools(shuffle):

    with open("chr1.genome", "w") as f:
        f.write(f"chr1\t250000000")

    subprocess.run("bedtools random -g chr1.genome -l 1000 -seed 1 > a.bed", shell=True)
    subprocess.run("bedtools random -g chr1.genome -l 1000 -seed 2 > b.bed", shell=True)

    intervals = []
    with open("a.bed", "r") as b:
        for line in b:
            l = line.split("\t")
            intervals.append((int(l[1]), int(l[2])))
    queries = []
    with open("b.bed", "r") as b:
        for line in b:
            l = line.split("\t")
            queries.append((int(l[1]), int(l[2])))
    if not shuffle:
        queries.sort()
    intervals.sort()
    # subprocess.run("rm a.bed b.bed chr1.genome", shell=True)
    return np.array(intervals), np.array(queries)


def make_random(n, shuffle):
    intervals = []
    n = int(n / 3)
    # sparse
    i = 0
    for i in range(i, n, 1_000):
        intervals.append((i, i + random.randint(50, 100)))

    # nested
    for i in range(i, n + i, 1_000):
        intervals.append((i, i + random.randint(25, 25_000)))

    # dense
    for i in range(i, n + i, 1_000):
        intervals.append((i, i + 1000))

    queries = list(zip(range(n), range(n)))
    if shuffle:
        random.shuffle(queries)
    else:
        queries.sort()
    intervals = np.array(sorted(intervals))
    queries = np.array(queries)
    return intervals, queries


def load_intervals(shuffle):
    dirname = os.path.dirname(__file__)
    queries = []
    intervals = []
    with open(dirname + "/chr1_ucsc_genes.bed", "r") as f:
        for line in f:
            l = line.split("\t")
            intervals.append( (int(l[1]), int(l[2])) )
    with open(dirname + "/chr1_reads.bed", "r") as f:
        for line in f:
            l = line.split("\t")
            queries.append( (int(l[1]), int(l[2])) )
    if shuffle:
        random.shuffle(queries)
    else:
        queries.sort()
    intervals.sort()
    return np.array(intervals), np.array(queries)


def run_tools(intervals, queries, shuffled):

    res = []

    # sortedintersect
    itv = IntervalSet(False)
    itv.add_from_iter(intervals)

    # quicksect
    tree = quicksect.IntervalTree()
    for s, e in intervals:
        tree.add(s, e)

    # cgranges
    cg = cr.cgranges()
    for s, e in intervals:
        cg.add("1", s, e, 0)
    cg.index()

    # ncls
    starts = pd.Series(intervals[:, 0])
    ends = pd.Series(intervals[:, 1])
    treencls = NCLS(starts, ends, starts)

    t0 = time.time()
    v = 0
    for start, end in queries:
        a = itv.search_interval(start, end)
        if a:
            v += len(a)
    res.append({'library': 'sortedintersect', 'time (s)': time.time() - t0, 'intersections': v})

    t0 = time.time()
    v = 0
    for start, end in queries:
        a = tree.find(Interval(start, end))
        if a:
            v += len(a)
    res.append({'library': 'quicksect', 'time (s)': time.time() - t0, 'intersections': v})

    t0 = time.time()
    v = 0
    for start, end in queries:
        a = list(cg.overlap("1", start, end))
        if a:
            v += len(a)
    res.append({'library': 'cgranges', 'time (s)': time.time() - t0, 'intersections': v})

    t0 = time.time()
    v = 0
    for start, end in queries:
        a = list(treencls.find_overlap(start - 1, end + 1))
        if a:
            v += len(a)
    res.append({'library': 'ncls', 'time (s)': time.time() - t0, 'intersections': v})


    return pd.DataFrame.from_records(res)



dfs = []
for shuffle in (True, False):
    print("Shuffled data", shuffle)
    print("random1")
    intervals, queries = make_random(100_000, shuffle)
    res = run_tools(intervals, queries, shuffle)
    res["test"] = ["random1"] * len(res)
    res["shuffle"] = [shuffle] * len(res)
    dfs.append(res)
    print("random2")
    intervals, queries = make_random_bedtools(shuffle)
    df2 = run_tools(intervals, queries, shuffle)
    df2["test"] = ["random2"] * len(df2)
    df2["shuffle"] = [shuffle] * len(df2)
    dfs.append(df2)
    print("reads+genes")
    intervals, queries = load_intervals(shuffle)
    df2 = run_tools(intervals, queries, shuffle)
    df2["test"] = ["reads+genes"] * len(df2)
    df2["shuffle"] = [shuffle] * len(df2)
    dfs.append(df2)
    print()


df = pd.concat(dfs)
print(df.to_markdown(index=False))

sns.set_palette("Set2")
sns.catplot(kind='bar', data=df, x="shuffle", y="time (s)", hue="library", col="test", sharey=False)
plt.show()
# plt.savefig('benchmark.png')
