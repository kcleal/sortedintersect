#cython: language_level=3, boundscheck=False, wraparound=False
#distutils: language=c++
from libcpp.vector cimport vector
from libc.stdint cimport uint64_t


__all__ = ["IntervalSet"]


cdef struct Interval:
    int low
    int high


cdef extern from "sisect.h":
    cdef cppclass SortedIntersecter:
        BasicIntervalTree()
        void add(int, int, int)
        Interval* searchPoint(int, int)



cdef inline bint is_overlapping(int x1, int x2, int y1, int y2) nogil:
    return max(x1, y1) <= min(x2, y2)


cdef class ISet:
    """A sorted interval set for searching with sorted query intervals / points"""
    cdef vector[int] starts
    cdef vector[int] ends
    cdef int last_r_start, last_r_end
    cdef int last_q_start, last_q_end
    cdef int current_r_start, current_r_end
    cdef int index
    cdef bint add_data
    cdef bint started_ref
    cdef object data
    def __init__(self, with_data):
        if with_data:
            self.add_data = True
            self.data = []
        else:
            self.add_data = False
            self.data = None
        self.last_r_start = -2_147_483_648
        self.last_q_start = -2_147_483_648
        self.current_r_start = 0
        self.current_r_end = 0
        self.index = 0

    cpdef add(self, int start, int end, value=None):
        assert end >= start
        if start < self.last_r_start:
            raise ValueError(f'Interval {start}-{end} is not in sorted order, last interval seen was {self.last_r_start}')
        self.starts.push_back(start)
        self.ends.push_back(end)
        self.last_r_start = start
        if self.add_data:
            self.data.append(value)

    cpdef add_from_iter(self, iterable):
        cdef int start, end
        for item in iterable:
            start = item[0]
            end = item[1]
            assert end >= start
            if start < self.last_r_start:
                raise ValueError(f'Interval {start}-{end} is not in sorted order, last interval seen was {self.last_r_start}')
            self.starts.push_back(start)
            self.ends.push_back(end)
            self.last_r_start = start
            if self.add_data:
                self.data.append(item[2])

    cpdef search_point(self, int pos):

        cdef uint64_t i = self.index

        if self.starts.size() == 0 or i >= self.starts.size():
            return False

        if pos < self.last_q_start:
            raise ValueError(f'Position {pos} is not in sorted order, last query interval seen was {self.last_q_start}')

        cdef bint passed = False
        self.current_r_start = self.starts[i]
        self.current_r_end = self.ends[i]
        self.last_q_start = pos
        if pos > self.current_r_end:
            i += 1
            while i < self.starts.size():
                self.current_r_start = self.starts[i]
                self.current_r_end = self.ends[i]
                if pos < self.current_r_start:
                    break
                elif self.current_r_start <= pos <= self.current_r_end:
                    passed = True
                    break
                i += 1
            self.index = i

        elif self.current_r_start <= pos <= self.current_r_end:
            passed = True

        if passed:
            if self.add_data:
                return self.current_r_start, self.current_r_end, self.data[self.index]
            return self.current_r_start, self.current_r_end
        return False


    cpdef search_interval(self, int start, int end):
        assert end >= start
        cdef uint64_t i = self.index
        if self.starts.size() == 0 or i >= self.starts.size():
            return False

        if start < self.last_q_start:
            raise ValueError(f'Search interval {start}-{end} is not in sorted order, last query interval seen was {self.last_q_start}')

        cdef bint passed = False
        self.last_q_start = start
        self.current_r_start = self.starts[i]
        self.current_r_end = self.ends[i]
        if start > self.current_r_end:
            i = self.index + 1
            while i < self.starts.size():
                self.current_r_start = self.starts[i]
                self.current_r_end = self.ends[i]
                if is_overlapping(start, end, self.current_r_start, self.current_r_end):
                    passed = True
                    break
                elif self.current_r_start > start:
                    break
                i += 1
            self.index = i

        elif is_overlapping(start, end, self.current_r_start, self.current_r_end):
            passed = True

        if passed:
            if self.add_data:
                return self.current_r_start, self.current_r_end, self.data[self.index]
            return self.current_r_start, self.current_r_end
        return False


cpdef ISet IntervalSet(with_data):
    """Returns a python friendly IntervalSet for searching with sorted query intervals / points"""
    return ISet(with_data)
