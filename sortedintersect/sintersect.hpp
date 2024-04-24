
#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cassert>

#include <iostream>


template<typename S, typename T> // "S" is a scalar type; "T" is the type of data associated with each interval
class IITree2 {
	struct StackCell {
		size_t x; // node
		int k, w; // k: level; w: 0 if left child hasn't been processed
		StackCell() {};
		StackCell(int k_, size_t x_, int w_) : x(x_), k(k_), w(w_) {};
	};
	struct Interval {
		S st, en, max;
		T data;
		Interval(const S &s, const S &e, const T &d) : st(s), en(e), max(e), data(d) {};
	};
	struct IntervalLess {
		bool operator()(const Interval &a, const Interval &b) const { return a.st < b.st; }
	};
	std::vector<Interval> a;
	int max_level;
	int index_core(std::vector<Interval> &a) {
		size_t i, last_i; // last_i points to the rightmost node in the tree
		S last; // last is the max value at node last_i
		int k;
		if (a.size() == 0) return -1;
		for (i = 0; i < a.size(); i += 2) last_i = i, last = a[i].max = a[i].en; // leaves (i.e. at level 0)
		for (k = 1; 1LL<<k <= a.size(); ++k) { // process internal nodes in the bottom-up order
			size_t x = 1LL<<(k-1), i0 = (x<<1) - 1, step = x<<2; // i0 is the first node
			for (i = i0; i < a.size(); i += step) { // traverse all nodes at level k
				S el = a[i - x].max;                          // max value of the left child
				S er = i + x < a.size()? a[i + x].max : last; // of the right child
				S e = a[i].en;
				e = e > el? e : el;
				e = e > er? e : er;
				a[i].max = e; // set the max value for node i
			}
			last_i = last_i>>k&1? last_i - x : last_i + x; // last_i now points to the parent of the original last_i
			if (last_i < a.size() && a[last_i].max > last) // update last accordingly
				last = a[last_i].max;
		}
		return k - 1;
	}
public:
	void add(const S &s, const S &e, const T &d) { a.push_back(Interval(s, e, d)); }
	void index(void) {
		std::sort(a.begin(), a.end(), IntervalLess());
		max_level = index_core(a);
	}
	bool overlap(const S &st, const S &en, std::vector<size_t> &out) const {
		int t = 0;
		StackCell stack[64];
		out.clear();
		if (max_level < 0) return false;
		stack[t++] = StackCell(max_level, (1LL<<max_level) - 1, 0); // push the root; this is a top down traversal
		while (t) { // the following guarantees that numbers in out[] are always sorted
			StackCell z = stack[--t];
			if (z.k <= 3) { // we are in a small subtree; traverse every node in this subtree
				size_t i, i0 = z.x >> z.k << z.k, i1 = i0 + (1LL<<(z.k+1)) - 1;
				if (i1 >= a.size()) i1 = a.size();
				for (i = i0; i < i1 && a[i].st < en; ++i)
					if (st < a[i].en) // if overlap, append to out[]
						out.push_back(i);
			} else if (z.w == 0) { // if left child not processed
				size_t y = z.x - (1LL<<(z.k-1)); // the left child of z.x; NB: y may be out of range (i.e. y>=a.size())
				stack[t++] = StackCell(z.k, z.x, 1); // re-add node z.x, but mark the left child having been processed
				if (y >= a.size() || a[y].max > st) // push the left child if y is out of range or may overlap with the query
					stack[t++] = StackCell(z.k - 1, y, 0);
			} else if (z.x < a.size() && a[z.x].st < en) { // need to push the right child
				if (st < a[z.x].en) out.push_back(z.x); // test if z.x overlaps the query; if yes, append to out[]
				stack[t++] = StackCell(z.k - 1, z.x + (1LL<<(z.k-1)), 0); // push the right child
			}
		}
		return out.size() > 0? true : false;
	}
	size_t size(void) const { return a.size(); }
	const S &start(size_t i) const { return a[i].st; }
	const S &end(size_t i) const { return a[i].en; }
	const T &data(size_t i) const { return a[i].data; }
};



bool is_overlapping(int x1, int x2, int y1, int y2) noexcept {
    return std::max(x1, y1) <= std::min(x2, y2);
}

template<typename S>
struct siInterval {
    S end;
    S covered_end;
    size_t covered_idx_left, covered_idx_right;
};

template<typename S>
struct siReverse {
    S start, end;
    size_t i;
};

template<typename It, typename T>
It _branchfree_search(It begin, It end, const T& x) {
    typedef typename std::iterator_traits<It>::difference_type diff_t;
    diff_t n = std::distance(begin, end);
    if (n == 0) {
        return end;
    }
    It base = begin;
    while (n > 1) {
        const diff_t half = n / 2;
        base = (*(base + half) < x) ? base + half : base;
        n -= half;
    }
    return (*(base) < x) ? base + 1 : base;
}


template<typename S, typename T>  // S for scalar for start, end. T for data type
class SIntersect {
    public:
    std::vector<S> starts;
    std::vector<siInterval<S>> ends;
    std::vector<T> data;
    S last_r_start, last_q_start, current_r_end, prev_r_start;
    int distance_threshold;
    int COUNTER;
    size_t idx, prev_idx_left;

    IITree2<S, T> tree;

    SIntersect() {
        distance_threshold = 50000;
        COUNTER = 0;
        last_r_start = std::numeric_limits<S>::min();
        last_q_start = 0; //std::numeric_limits<S>::min();
        current_r_end = 0; //std::numeric_limits<S>::min();
        idx = 0;

    };
    ~SIntersect() = default;

    void clear() {
        distance_threshold = 50000;
        COUNTER = 0;
        last_r_start = std::numeric_limits<S>::min();
        last_q_start = 0; //std::numeric_limits<S>::min();
        current_r_end = 0; //std::numeric_limits<S>::min();
        idx = 0;
        starts.clear();
        ends.clear();
        data.clear();
    }

    void reserve(size_t n) {
        starts.reserve(n);
        ends.reserve(n);
        data.reserve(n);
    }

    size_t size() {
        return starts.size();
    }

    void add(S start, S end, const T& value) {
        if (end < start) {
            throw std::logic_error("End is less than start");
        }
        if (last_r_start > start) {
            throw std::logic_error("Reference interval is not in sorted order");
        }
        tree.add(start, end, value);

        size_t size = starts.size();
        starts.push_back(start);
        size_t covered_idx_left;
        if (current_r_end >= end) {  // previous interval contains this interval
            covered_idx_left = prev_idx_left;
            while (covered_idx_left > 0) {  // find minimum containment index
                if (ends[covered_idx_left - 1].end >= end) {
                    --covered_idx_left;
                }
                break;
            }
        } else {
            current_r_end = end;
            covered_idx_left = size;
            prev_idx_left = size;
        }
        ends.emplace_back() = {end, current_r_end, covered_idx_left, size};
//        std::cout << start << " " << end << "   {end=" << end << " cre=" << current_r_end << " cil=" << covered_idx_left << std::endl;
        last_r_start = start;
        data.push_back(value);
    }

    void index() {
        tree.index();
        if (starts.size() < 2) {
            return;
        }
        std::vector<siReverse<S>> rev;
        rev.resize(starts.size());
        for (size_t i=0; i < starts.size(); ++i) {
            rev[i].start = starts[i];
            rev[i].end = ends[i].end;
            rev[i].i = i;
        }
        std::sort(rev.begin(), rev.end(), [](const siReverse<S>& a, const siReverse<S>& b) { return a.end < b.end; });

        // find max containment
        size_t max_idx_right = starts.size() - 1;
        for (int i = rev.size() - 1; i >= 0; --i) {
            size_t idx = rev[i].i;
            S currentEnd = rev[i].end;
            while (max_idx_right > idx) {
                if (starts[max_idx_right] <= currentEnd) {
                    break;
                }
                --max_idx_right;
            }
            ends[idx].covered_idx_right = max_idx_right;
        }
        for (size_t j=0; j < starts.size(); ++j) {
            std::cout << starts[j] << " " << ends[j].end << "   {cre=" << ends[j].covered_end <<
            " cil=" << ends[j].covered_idx_left << " cir=" << ends[j].covered_idx_right  << std::endl;
        }
    }

    size_t _line_scan(S pos) {
//        COUNTER += 1;
        std::cout << " line scan\n";
        if (pos < last_q_start) {
            std::cout << " pos < last_q_start\n";
            if (ends[idx].covered_idx_left == idx) {
                return 0;
            }
            while (idx > 0 && ends[idx].covered_end >= pos) {
                idx = ends[idx].covered_idx_left;
            }
        } else {
            while (idx < ends.size() && pos > ends[idx].end) {
                ++idx;
            }
            std::cout << " linear search to idx=" << idx << std::endl;
        }
        std::cout << " cri=" << ends[idx].covered_idx_right << " so min comparisons = " << ends[idx].covered_idx_right - idx << std::endl;
        return ends[idx].covered_idx_right - idx;
    }

    size_t _binary_search(S pos) {
        auto lower = last_q_start < pos
//                     ? std::lower_bound(starts.begin() + idx, starts.end(), pos)
//                     : std::lower_bound(starts.begin(), starts.begin() + idx, pos);
                     ? _branchfree_search(starts.begin() + idx, starts.end(), pos)
                     : _branchfree_search(starts.begin(), starts.begin() + idx, pos);
        idx = std::distance(starts.begin(), lower);
        if (idx >= ends.size()) {
            --idx;
        }
        std::cout << pos << " binary search to idx=" << idx << std::endl;
        idx = ends[idx].covered_idx_left;
        size_t min_comparisons = ends[idx].covered_idx_right - idx;
        std::cout << pos << " binary search to idx=" << idx << " cir=" << ends[idx].covered_idx_right <<
        " min comparissons = " << min_comparisons << std::endl;
        while (idx > 0 && ends[idx].covered_end >= pos) {
            --idx;
        }
        if (ends[idx].covered_end < pos) {
            ++idx;
        }
        std::cout << " final idx " << idx << "\n";
        return min_comparisons;
    }

    size_t _set_reference_idx(S pos) {
        size_t min_comparisons;
        return _binary_search(pos);
        if (std::abs(pos - last_q_start) > distance_threshold) {

//            std::cout << " bin\n";
//            return -1;
            min_comparisons = _binary_search(pos);
        } else {
//            COUNTER += 1;
//            std::cout << idx << " lin\n";
            min_comparisons = _line_scan(pos);
        }
//        return _line_scan(pos);
        return min_comparisons;
    }

    void search_overlap(S start, S end, std::vector<size_t>& found) {
        size_t size = ends.size();
        if (size == 0) {
            return;
        }
        if (!found.empty()) {
            found.clear();
        }
//        size_t min_comparisons = _line_scan(start); //_set_reference_idx(start);
        size_t min_comparisons = _set_reference_idx(start);
//        if (min_comparisons > 295) {
//            std::cout << idx << " s " << start << " " << end << " " << min_comparisons << std::endl;
//        }

        if (min_comparisons > 50) {
            tree.overlap(start, end, found);
            if (!found.empty()) {
                idx = found.front();
            }
            return;
        }
        last_q_start = start;
        for (size_t i=idx; i < size; ++i) {

            if (is_overlapping(start, end, starts[i], ends[i].end)) {
                found.push_back(i);
            } else if (start < starts[i] || start > ends[i].covered_end) {
                break;
            }
        }
    }












    bool search_overlap(S start, S end) {
        size_t size = ends.size();
        if (size == 0) {
            return false;
        }
        _set_reference_idx(start);
        last_q_start = starts[idx];
        for (size_t i=idx; i < size; ++i) {
            if (is_overlapping(start, end, starts[i], ends[i].end)) {
                return true;
            } else if (start < starts[i] || start > ends[i].covered_end) {
                return false;
            }
        }
        return false;
    }

    void search_point(S pos, std::vector<size_t>& found) {
        size_t size = ends.size();
        if (size == 0) {
            return;
        }
        if (!found.empty()) {
            found.clear();
        }
        _set_reference_idx(pos);
        last_q_start = pos;
        for (size_t i=idx; i < size; ++i) {
            if (starts[i] <= pos && pos <= ends[i].end) {
                found.push_back(i);
            } else if (pos < starts[i] || pos > ends[i].covered_end) {
                break;
            }
        }
    }

    bool search_point(S pos) {
        size_t size = ends.size();
        if (size == 0) {
            return false;
        }
        _set_reference_idx(pos);
        last_q_start = pos;
        for (size_t i=idx; i < size; ++i) {
            if (starts[i] <= pos && pos <= ends[i].end) {
                return true;
            } else if (pos < starts[i] || pos > ends[i].covered_end) {
                return false;
            }
        }
        return false;
    }

};
