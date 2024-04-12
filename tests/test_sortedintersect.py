
import unittest
from sortedintersect import IntervalSet


class TestConstruct(unittest.TestCase):
    """ Test construction and searching"""
    def test_empty(self):
        itv = IntervalSet(True)
        res = itv.search_point(1)
        assert not res
        res = itv.search_interval(1, 2)
        assert not res
        itv.add_from_iter([])
        res = itv.search_point(1)
        assert not res

    def test_construct(self):
        itv = IntervalSet(True)
        itv.add(0, 10, '1')
        res = itv.search_point(1)
        assert res[0] == (0, 10, '1')
        itv = IntervalSet(False, bool_only=True)
        itv.add(0, 10)
        itv.add(5, 5)
        res = itv.search_point(1)
        assert res
        assert isinstance(res, bool)

    def test_point_search(self):
        itv = IntervalSet(False)
        itv.add(2, 4)
        itv.add(6, 8)
        s = 0
        for i in range(10):
            if itv.search_point(i):
                s += 1
        assert s == 6
        itv = IntervalSet(False)
        itv.add(12, 14)
        itv.add(16, 18)
        assert itv.search_point(18)[0] == (16, 18)
        itv.add(20, 20)
        assert itv.search_point(20)[0] == (20, 20)

    def test_multi_point_search(self):
        itv = IntervalSet(False)
        itv.add(1, 4)
        itv.add(2, 5)
        itv.add(3, 8)
        itv.add(5, 8)
        itv.add(6, 8)
        assert tuple(itv.search_point(5)) == ((2, 5), (3, 8), (5, 8))

    def test_nested_point_search(self):
        itv = IntervalSet(False)
        itv.add(1, 20)
        itv.add(5, 6)
        itv.add(10, 12)
        itv.add(15, 16)
        assert tuple(itv.search_point(1)) == ((1, 20),)
        assert tuple(itv.search_point(5)) == ((1, 20), (5, 6))
        assert tuple(itv.search_point(12)) == ((1, 20), (10, 12))
        assert tuple(itv.search_point(15)) == ((1, 20), (15, 16))
        itv.add(16, 19)
        assert tuple(itv.search_point(16)) == ((1, 20), (15, 16), (16, 19))

    def test_return_bool(self):
        itv = IntervalSet(False, bool_only=True)
        itv.add(1, 10)
        itv.add(1, 100)
        itv.add(50, 60)
        itv.add(90, 110)
        assert not itv.search_point(120)
        assert itv.search_point(50)
        assert itv.search_point(110)

    def test_ref_intervals_not_sorted(self):
        itv = IntervalSet(False)
        itv.add(2, 4)
        self.assertRaises(ValueError, itv.add, 1, 2)
        itv.add(6, 7)
        self.assertRaises(ValueError, itv.add, 5, 8)

    # Interval based tests

    def test_interval_search(self):
        itv = IntervalSet(True)
        itv.add(2, 4, 'a')
        itv.add(7, 8, 'b')
        assert itv.search_interval(1, 4)[0] == (2, 4, 'a')
        assert itv.search_interval(2, 4)[0] == (2, 4, 'a')
        assert itv.search_interval(2, 9)[0] == (2, 4, 'a')
        assert not itv.search_interval(5, 6)
        assert itv.search_interval(6, 9)[0] == (7, 8, 'b')
        assert not itv.search_interval(9, 10)
        itv.add(20, 30, 'v')
        assert itv.search_interval(10, 21)[0] == (20, 30, 'v')

    def test_multi_interval_search(self):
        itv = IntervalSet(False)
        itv.add(1, 4)
        itv.add(2, 5)
        itv.add(3, 8)
        itv.add(5, 8)
        itv.add(6, 8)
        assert tuple(itv.search_interval(5, 5)) == ((2, 5), (3, 8), (5, 8))

    def test_query_interval_not_sorted(self):
        itv = IntervalSet(False)
        itv.add(7, 8)
        self.assertRaises(ValueError, itv.add, 2, 4)
        itv.add(7, 10)
        self.assertRaises(ValueError, itv.add, 6, 9)

    def test_query_points_not_sorted(self):
        itv = IntervalSet(False)
        itv.add(-1, 1)
        itv.add(2, 4)
        itv.add(9, 10)
        assert tuple(itv.search_point(10)) == ((9, 10),)
        assert tuple(itv.search_point(0)) == ((-1, 1),)

    def test_add_from_iter(self):
        intervals = []
        for i in range(0, 10_000_000, 1_000):
            intervals.append((i, i + 100, i))
        itv = IntervalSet(True)
        itr = iter(intervals)
        itv.add_from_iter(itr)

    def test_point_search_not_sorted(self):
        itv = IntervalSet(False)
        itv.add(2, 7)
        itv.add(6, 8)
        assert itv.search_point(8)[0] == (6, 8)
        assert tuple(itv.search_point(6)) == ((2, 7), (6, 8))
        assert itv.search_point(4)[0] == (2, 7)
        assert itv.search_point(2)[0] == (2, 7)
        assert tuple(itv.search_point(6)) == ((2, 7), (6, 8))


def main():
    unittest.main()


if __name__ == "__main__":
    unittest.main()
