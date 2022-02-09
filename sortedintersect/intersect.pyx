#cython: language_level=3, boundscheck=False, wraparound=False
#distutils: language=c++
from libcpp.vector cimport vector
from libc.stdint cimport uint64_t


__all__ = ["IntervalSet"]


cdef inline bint is_overlapping(int x1, int x2, int y1, int y2) nogil:
    return max(x1, y1) <= min(x2, y2)


cdef class ISet:
    cdef vector[int] starts
    cdef vector[int] ends
    cdef int last_r_start, last_r_end
    cdef int last_q_start, last_q_end
    cdef int current_r_start, current_r_end
    cdef int index
    cdef bint add_data
    cdef bint started_ref, started_query
    cdef object data
    def __init__(self, with_data):
        if with_data:
            self.add_data = True
            self.data = []
        else:
            self.add_data = False
            self.data = None

        self.last_r_start = 0
        self.last_r_end = 0
        self.last_q_start = 0
        self.last_q_end = 0
        self.current_r_start = 0
        self.current_r_end = 0
        self.index = 0
        self.started_ref = False
        self.started_query = False

    cpdef add(self, int start, int end, value=None):

        assert end > start
        if self.started_ref and start < self.last_r_start or end < self.last_r_end:
            raise ValueError(f'Interval {start}-{end} is not in sorted order, last interval seen was {self.last_r_start}-{self.last_r_end}')

        if not self.started_ref:
            self.last_q_start = start
            self.last_q_end = end
            self.started_ref = True

        self.starts.push_back(start)
        self.ends.push_back(end)

        self.last_r_start = start
        self.last_r_end = end

        if self.add_data:
            self.data.append(value)

    cpdef add_from_iter(self, iterable):

        cdef int start, end
        for item in iterable:
            start = item[0]
            end = item[1]

            assert end > start
            if self.started_ref and start < self.last_r_start or end < self.last_r_end:
                raise ValueError(f'Interval {start}-{end} is not in sorted order, last interval seen was {self.last_r_start}-{self.last_r_end}')

            if not self.started_ref:
                self.last_q_start = start
                self.last_q_end = end
                self.started_ref = True

            self.starts.push_back(start)
            self.ends.push_back(end)

            self.last_r_start = start
            self.last_r_end = end

            if self.add_data:
                self.data.append(item[2])

    cpdef search_point(self, int pos):

        if not self.started_ref:
            if self.add_data:
                return False
            return False

        cdef uint64_t i = self.index

        if not self.started_query:
            self.current_r_start = self.starts[i]
            self.current_r_end = self.ends[i]

        if self.started_query and pos < self.last_q_start:
            raise ValueError(f'Position {pos} is not in sorted order, last query interval seen was {self.last_q_start}-{self.last_q_end}')

        cdef bint passed = False

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

        self.last_q_start = pos
        self.last_q_end = pos
        self.started_query = True

        if self.add_data:
            if passed:
                return self.data[self.index]
            return False
        else:
            if passed:
                return True
            return False

    cpdef search_interval(self, int start, int end):

        assert end > start
        if not self.started_ref:
            if self.add_data:
                return False
            return False

        if not self.started_query:
            self.current_r_start = self.starts[self.index]
            self.current_r_end = self.ends[self.index]

        if self.started_query and start < self.last_q_start:
            raise ValueError(f'Interval {start}-{end} is not in sorted order, last query interval seen was {self.last_q_start}-{self.last_q_end}')

        cdef bint passed = False
        cdef uint64_t i

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

        self.last_q_start = start
        self.last_q_end = end
        self.started_query = True

        if self.add_data:
            if passed:
                return self.data[self.index]
            return False
        else:
            if passed:
                return True
            return False


cpdef ISet IntervalSet(with_data):
    """Returns an IntervalSet for searching with sorted query intervals / points"""
    return ISet(with_data)