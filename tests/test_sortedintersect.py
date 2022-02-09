
import unittest
from sortedintersect.intersect import IntervalSet


class TestConstruct(unittest.TestCase):
    """ Test construction and searching"""
    def test_construct(self):

        itv = IntervalSet(True)
        itv.add(0, 10, '1')
        res = itv.search_point(1)
        assert res == '1'

        itv = IntervalSet(False)
        itv.add(0, 10)
        res = itv.search_point(1)
        assert res

    def test_point_search(self):

        itv = IntervalSet(False)
        itv.add(2, 4)
        itv.add(6, 8)
        s = 0
        for i in range(10):
            s += int(itv.search_point(i))
        assert s == 6

        itv = IntervalSet(False)
        itv.add(12, 14)
        itv.add(16, 18)
        itv.search_point(18)

    def test_ref_intervals_not_sorted(self):
        itv = IntervalSet(False)
        itv.add(2, 4)
        self.assertRaises(ValueError, itv.add, 1, 2)
        itv.add(6, 7)
        self.assertRaises(ValueError, itv.add, 5, 8)
        self.assertRaises(AssertionError, itv.add, 5, 5)

    def test_interval_search(self):
        itv = IntervalSet(True)
        itv.add(2, 4, 'a')
        itv.add(7, 8, 'b')
        assert itv.search_interval(1, 4) == 'a'
        assert itv.search_interval(2, 4) == 'a'
        assert itv.search_interval(2, 9) == 'a'
        assert not itv.search_interval(5, 6)
        assert itv.search_interval(6, 9) == 'b'
        assert not itv.search_interval(9, 10)

    def test_query_interval_not_sorted(self):
        itv = IntervalSet(False)
        itv.add(7, 8)
        self.assertRaises(ValueError, itv.add, 2, 4)
        itv.add(7, 10)
        self.assertRaises(ValueError, itv.add, 7, 9)

    def test_query_points_not_sorted(self):
        itv = IntervalSet(False)
        itv.add(-1, 1)
        itv.add(2, 4)
        itv.search_point(2)
        self.assertRaises(ValueError, itv.search_point, 1)
        itv.search_point(3)
        self.assertRaises(ValueError, itv.search_point, -1)

    def test_add_from_iter(self):
        intervals = []
        for i in range(0, 10_000_000, 1_000):
            intervals.append((i, i + 100, i))
        itv = IntervalSet(True)
        itr = iter(intervals)
        itv.add_from_iter(itr)




def main():
    unittest.main()


if __name__ == "__main__":
    unittest.main()
