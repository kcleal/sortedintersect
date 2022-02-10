================
sorted-intersect
================

sortedintersect can be used for searching a set of reference intervals for intersecting points or intervals.


A common task in bioinformatics is to check if an interval or point overlaps a set of reference intervals.
An interval tree is often used for this purpose although if both the
reference intervals and query intervals are sorted ahead of time, then a simpler plane-sweep algorithm can be used.
This situation arises when processing a sorted alignment or vcf file and checking against a sorted reference interval set, for example.


Installation
------------

To build from source::

    git clone https://github.com/kcleal/sortedintersect
    cd sortedintersect
    pip install .

Overview
--------

Common usage is to check if a point overlaps a reference set:

.. code-block:: python

    from sortedintersect import IntervalSet

    # intervals without data
    itv = IntervalSet(False)
    itv.add(0, 10)
    itv.search_point(1)
    # >>> True

    # intervals with python object as data
    itv = IntervalSet(True)
    itv.add(0, 10, 'interval1')
    itv.add(20, 30, {'a': 1})
    itv.search_point(1)
    # >>> 'interval1'
    itv.search_point(20)
    # >>> {'a': 1}

Note, both reference and query intervals must be added and queried in sorted order otherwise a ValueError will be raised:

.. code-block:: python

    # intervals without data
    itv = IntervalSet(False)
    itv.add(10, 11)
    itv.add(0, 1)
    # >>> ValueError
    itv.search_point(10)  # True
    itv.search_point(9)
    # >>> ValueError


Intervals can also be queried:

.. code-block:: python

    # intervals without data
    itv = IntervalSet(False)
    itv.add(10, 11)
    itv.add(50, 60)
    itv.search_interval(9, 10)   # True
    itv.search_interval(20, 30)  # False
    itv.search_interval(50, 51)  # True

Benchmarks
----------

sortedintersect was compared to popular python implementations based on interval trees, see tests folder.
A major advantage of comparison tools is that queries can be performed in non-sorted order,
which is not the case for sortedintersect:

.. list-table::
   :widths: 25 25 25 25 25
   :header-rows: 1

   * - N intersections
     - sortedintersect (s)
     - quicksect (s)
     - ncls (s)
     - ailist (s)
   * - 100 k
     - 0.006469
     - 0.033503
     - 0.123314
     - 0.053892
   * - 1 million
     - 0.064078
     - 0.334860
     - 1.230206
     - 0.570837
   * - 10 million
     - 0.630569
     - 3.962883
     - 12.819762
     - 5.696887
   * - 100 million
     - 6.407564
     - 40.743349
     - 127.128570
     - 56.149942

.. image:: https://github.com/kcleal/sortedintersect/blob/master/tests/benchmark.png


Limitations
-----------

- Both reference and queries must be assessed in sorted order
- Only the first overlapping interval is currently returned
