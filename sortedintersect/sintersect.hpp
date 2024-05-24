
#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cassert>
#include <queue>
#include <iostream>

namespace sortedIntersect {


    template <class Scalar, typename Value>
    class Interval {
    public:
        Scalar start;
        Scalar stop;
        Value value;
        Interval(const Scalar& s, const Scalar& e, const Value& v)
        : start(std::min(s, e))
        , stop(std::max(s, e))
        , value(v)
        {}
    };

    template <class Scalar, typename Value>
    Value intervalStart(const Interval<Scalar,Value>& i) {
        return i.start;
    }

    template <class Scalar, typename Value>
    Value intervalStop(const Interval<Scalar, Value>& i) {
        return i.stop;
    }


    template <class Scalar, class Value>
    class IntervalTree {
    public:
        typedef Interval<Scalar, Value> interval;
        typedef std::vector<interval> interval_vector;

        struct IntervalStartCmp {
            bool operator()(const interval& a, const interval& b) {
                return a.start < b.start;
            }
        };

        struct IntervalStopCmp {
            bool operator()(const interval& a, const interval& b) {
                return a.stop < b.stop;
            }
        };

        IntervalTree()
            : left(nullptr)
            , right(nullptr)
            , center(0)
        {}

        ~IntervalTree() = default;

        std::unique_ptr<IntervalTree> clone() const {
            return std::unique_ptr<IntervalTree>(new IntervalTree(*this));
        }

        IntervalTree(const IntervalTree& other)
        :   intervals(other.intervals),
            left(other.left ? other.left->clone() : nullptr),
            right(other.right ? other.right->clone() : nullptr),
            center(other.center)
        {}

        IntervalTree& operator=(IntervalTree&&) = default;
        IntervalTree(IntervalTree&&) = default;

        IntervalTree& operator=(const IntervalTree& other) {
            center = other.center;
            intervals = other.intervals;
            left = other.left ? other.left->clone() : nullptr;
            right = other.right ? other.right->clone() : nullptr;
            return *this;
        }

        IntervalTree(
                interval_vector&& ivals,
                std::size_t depth = 16,
                std::size_t minbucket = 64,
                std::size_t maxbucket = 512,
                Scalar leftextent = 0,
                Scalar rightextent = 0)
          : left(nullptr)
          , right(nullptr)
        {
            --depth;
            const auto minmaxStop = std::minmax_element(ivals.begin(), ivals.end(),
                                                        IntervalStopCmp());
            const auto minmaxStart = std::minmax_element(ivals.begin(), ivals.end(),
                                                         IntervalStartCmp());
            if (!ivals.empty()) {
                center = (minmaxStart.first->start + minmaxStop.second->stop) / 2;
            }
            if (leftextent == 0 && rightextent == 0) {
                std::sort(ivals.begin(), ivals.end(), IntervalStartCmp());
            } else {
                assert(std::is_sorted(ivals.begin(), ivals.end(), IntervalStartCmp()));
            }
            if (depth == 0 || (ivals.size() < minbucket && ivals.size() < maxbucket)) {
                std::sort(ivals.begin(), ivals.end(), IntervalStartCmp());
                intervals = std::move(ivals);
                assert(is_valid().first);
                return;
            } else {
                Scalar leftp = 0;
                Scalar rightp = 0;

                if (leftextent || rightextent) {
                    leftp = leftextent;
                    rightp = rightextent;
                } else {
                    leftp = ivals.front().start;
                    rightp = std::max_element(ivals.begin(), ivals.end(), IntervalStopCmp())->stop;
                }
                interval_vector lefts;
                interval_vector rights;
                for (typename interval_vector::const_iterator i = ivals.begin();
                     i != ivals.end(); ++i) {
                    const interval& interval = *i;
                    if (interval.stop < center) {
                        lefts.push_back(interval);
                    } else if (interval.start > center) {
                        rights.push_back(interval);
                    } else {
                        assert(interval.start <= center);
                        assert(center <= interval.stop);
                        intervals.push_back(interval);
                    }
                }
                if (!lefts.empty()) {
                    left.reset(new IntervalTree(std::move(lefts), depth, minbucket, maxbucket, leftp, center));
                }
                if (!rights.empty()) {
                    right.reset(new IntervalTree(std::move(rights), depth, minbucket, maxbucket, center, rightp));
                }
            }
            assert(is_valid().first);
        }

        // Call f on all intervals near the range [start, stop]:
        template <class UnaryFunction>
        void visit_near(const Scalar& start, const Scalar& stop, UnaryFunction f) const {
            if (!intervals.empty() && ! (stop < intervals.front().start)) {
                for (auto & i : intervals) {
                  f(i);
                }
            }
            if (left && start <= center) {
                left->visit_near(start, stop, f);
            }
            if (right && stop >= center) {
                right->visit_near(start, stop, f);
            }
        }

        // Call f on all intervals crossing pos
        template <class UnaryFunction>
        void visit_overlapping(const Scalar& pos, UnaryFunction f) const {
            visit_overlapping(pos, pos, f);
        }

        // Call f on all intervals overlapping [start, stop]
        template <class UnaryFunction>
        void visit_overlapping(const Scalar& start, const Scalar& stop, UnaryFunction f) const {
            auto filterF = [&](const interval& interval) {
                if (interval.stop >= start && interval.start <= stop) {
                    // Only apply f if overlapping
                    f(interval);
                }
            };
            visit_near(start, stop, filterF);
        }

        interval_vector findOverlapping(const Scalar& start, const Scalar& stop) const {
            interval_vector result;
            visit_overlapping(start, stop,
                              [&](const interval& interval) {
                                result.emplace_back(interval);
                              });
            return result;
        }

        template <class UnaryFunction>
        void visit_all(UnaryFunction f) const {
            if (left) {
                left->visit_all(f);
            }
            std::for_each(intervals.begin(), intervals.end(), f);
            if (right) {
                right->visit_all(f);
            }
        }

        std::pair<Scalar, Scalar> extentBruitForce() const {
            struct Extent {
                std::pair<Scalar, Scalar> x = {std::numeric_limits<Scalar>::max(),
                                                           std::numeric_limits<Scalar>::min() };
                void operator()(const interval & interval) {
                    x.first  = std::min(x.first,  interval.start);
                    x.second = std::max(x.second, interval.stop);
                }
                                                                    };
                                                Extent extent;

            visit_all([&](const interval & interval) { extent(interval); });
            return extent.x;
                                                }

        // Check all constraints.
        // If first is false, second is invalid.
        std::pair<bool, std::pair<Scalar, Scalar>> is_valid() const {
            const auto minmaxStop = std::minmax_element(intervals.begin(), intervals.end(),
                                                        IntervalStopCmp());
            const auto minmaxStart = std::minmax_element(intervals.begin(), intervals.end(),
                                                         IntervalStartCmp());

            std::pair<bool, std::pair<Scalar, Scalar>> result = {true, { std::numeric_limits<Scalar>::max(),
                                                                         std::numeric_limits<Scalar>::min() }};
            if (!intervals.empty()) {
                result.second.first   = std::min(result.second.first,  minmaxStart.first->start);
                result.second.second  = std::min(result.second.second, minmaxStop.second->stop);
            }
            if (left) {
                auto valid = left->is_valid();
                result.first &= valid.first;
                result.second.first   = std::min(result.second.first,  valid.second.first);
                result.second.second  = std::min(result.second.second, valid.second.second);
                if (!result.first) { return result; }
                if (valid.second.second >= center) {
                    result.first = false;
                    return result;
                }
            }
            if (right) {
                auto valid = right->is_valid();
                result.first &= valid.first;
                result.second.first   = std::min(result.second.first,  valid.second.first);
                result.second.second  = std::min(result.second.second, valid.second.second);
                if (!result.first) { return result; }
                if (valid.second.first <= center) {
                    result.first = false;
                    return result;
                }
            }
            if (!std::is_sorted(intervals.begin(), intervals.end(), IntervalStartCmp())) {
                result.first = false;
            }
            return result;
        }
    private:
        interval_vector intervals;
        std::unique_ptr<IntervalTree> left;
        std::unique_ptr<IntervalTree> right;
        Scalar center;
    };


    template<typename S>
    struct siInterval {
        S start, end;
    };

    template<typename S>
    struct siReverse {
        S end;
        size_t idx;
    };


    template<typename S>
    inline bool is_overlapping(const S x1, const S x2, const S y1, const S y2) noexcept {
        return std::max(x1, y1) <= std::min(x2, y2);
    }

    template<typename S>
    inline bool is_overlapping_interval(const siInterval<S>& itv1, const siInterval<S>& itv2) noexcept {
        return std::max(itv1.start, itv2.start) <= std::min(itv1.end, itv2.end);
    }

    template<typename S>
    inline bool is_overlapping_interval(const S x1, const S x2, const siInterval<S>& itv) noexcept {
        return std::max(x1, itv.start) <= std::min(x2, itv.end);
    }

    // S for scalar for start, end. T for data type
    template<typename S, typename T>
    class SIntersect {
        public:
        std::vector<S> starts;
        std::vector<S> ends;
        std::vector<siInterval<S>> intervals;
        std::vector<long int> branch_left;
        std::vector<long int> branch_right;
        std::vector<T> data;
        S distance_threshold;
        size_t idx, n_intervals;
        S last_q_start;
        bool isSorted;
        int COUNTER;
//        typename std::vector<S>::iterator it_begin, it_end;
        SIntersect()
            : distance_threshold(50000)
            , idx(0)
            , n_intervals(0)
            , last_q_start(std::numeric_limits<S>::min())
            , isSorted(true)
            { COUNTER = 0; }

        ~SIntersect() = default;

        void clear() {
            idx = 0;
            last_q_start = std::numeric_limits<S>::min();
            intervals.clear();
            data.clear();
        }

        void reserve(size_t n) {
            intervals.reserve(n);
            data.reserve(n);
        }

        size_t size() {
            return intervals.size();
        }

        void add(S start, S end, const T& value) {
            if (!intervals.empty() && start < intervals.back().start) {
                isSorted = false;
                throw std::runtime_error("yep");
            }
            intervals.emplace_back() = {start, end};
            data.push_back(value);
            starts.push_back(start);
            ends.push_back(end);
        }

        inline void _binary_search(S pos) noexcept {
            auto lower = last_q_start < pos
                         ? std::lower_bound(starts.begin() + idx, starts.end(), pos)
                         : std::lower_bound(starts.begin(), starts.begin() + idx, pos);
            if (lower != starts.begin() && (lower == starts.end() || *lower > pos)) {
                --lower;
            }
            idx = std::distance(starts.begin(), lower);
        }

        void index() {
            n_intervals = intervals.size();
//            it_begin = starts.begin();
//            it_end = starts.end();
            if (n_intervals < 2) {
                return;
            }
            // sort data by start, sort data by end and keep the indexes
    //        if (!isSorted) {
    //            std::sort(intervals.begin(), intervals.end(),
    //            [](const siInterval<S>& a, const siInterval<S>& b) {
    //                return a.start < b.start; });
    //            isSorted = true;
    //        }
    //        std::sort(intervals.begin(), intervals.end(),
    //            [](const siInterval<S>& a, const siInterval<S>& b) {
    //                return a.start < b.start; });
    //            isSorted = true;

    //        for (auto item: intervals) {
    //            std::cout << "(" << item.start << ", " << item.end <<"), ";
    //        } std::cout << std::endl;

            idx = 0;
            if (!branch_left.empty()) {
                branch_left.clear();
                branch_right.clear();
            }
            branch_left.resize(intervals.size(), -1);
            branch_right.resize(intervals.size(), -1);

            long int i, j;
            for (i=intervals.size() - 1; i >= 0; --i) {
                S start = intervals[i].start;
                S end = intervals[i].end;
                _binary_search(end);
                j = (long int)idx;
                last_q_start = start;
                for (; j >= 0; --j) {
                    if (i == j) {
                        continue;
                    }
                    S qStart = intervals[j].start;
                    S qEnd = intervals[j].end;
                    if (is_overlapping(start, end, qStart, qEnd)) {
                        if (qEnd > end) {
                            if (branch_right[i] < 0 || qStart > intervals[branch_right[i]].start) {
                                branch_right[i] = j;
                            }
                            if (branch_left[j] < 0 || qStart > intervals[branch_left[j]].start) {
                                branch_left[j] = i;
                            }
                        } else {  // less than or equal to
                            if (branch_left[i] < 0 || qStart > intervals[branch_left[i]].start) {
                                branch_left[i] = j;
                            }
                            if (branch_right[j] < 0 || qStart > intervals[branch_right[j]].start) {
                                if (branch_right[i] == j) {
                                    branch_right[i] = -1;
                                }
                                branch_right[j] = i;
                            }
                        }
                    } else {
                        break;
                    }
                }
            }
            idx = 0;
            return;
//            std::terminate();
            size_t jj = 0;
//            for (auto item: branch_left) {
//                std::cout << jj << " (" << intervals[jj].start << "-" << intervals[jj].end << "), left=" << item << ", right=" << branch_right[jj] << std::endl;
//                jj += 1;
//                if (jj > 18) {
//                    break;
//                }
//            } std::cout << std::endl;
//            std::terminate();
//            return;



            if (!branch_left.empty()) {
                branch_left.clear();
                branch_right.clear();
            }
            branch_left.resize(intervals.size(), -1);
            branch_right.resize(intervals.size(), -1);

            // fill out branches table
            std::vector<Interval<S, size_t>> intervals2;
            size_t index = 0;
            for (const auto& item : intervals) {
                intervals2.push_back(Interval<S, size_t>(item.start, item.end, index));
                index += 1;
            }
            IntervalTree<S, size_t> tree2(std::move(intervals2));
            std::vector<Interval<S, size_t>> res;

            for (auto& itv : intervals) {
                auto res = tree2.findOverlapping(itv.start, itv.end);
                S left_pos = std::numeric_limits<S>::min();
                S right_pos = std::numeric_limits<S>::min();
                for (const auto& r : res) {
                    if (r.value == idx) { continue; }
                    size_t v = r.value;
                    if (r.stop <= itv.end) {
                        if (intervals[v].start > left_pos || (intervals[v].start == left_pos && static_cast<long int>(idx) > branch_left[v])) {
                            left_pos = intervals[v].start;
                            branch_left[idx] = v;
                        }
                    } else {
                        if (intervals[v].start > right_pos || (intervals[v].start == right_pos && static_cast<long int>(idx) > branch_right[v])) {
                            right_pos = intervals[v].start;
                            branch_right[idx] = v;
                        }
                    }
                }
                idx += 1;
            }
            idx = 0;

//            jj = 0;
//            for (auto item: branch_left) {
//                std::cout << jj << " (" << intervals[jj].start << "-" << intervals[jj].end << "), left=" << item << ", right=" << branch_right[jj] << std::endl;
//                jj += 1;
//                if (jj > 18) {
//                    break;
//                }
//            }
        }

//        size_t _line_scan(S pos) {
    //        if (pos < last_q_start) {
    //            if (ends[idx].covered_idx_left == idx) {
    //                return 0;
    //            }
    //            while (idx > 0 && ends[idx].covered_end >= pos) {
    //                idx = ends[idx].covered_idx_left;
    //            }
    //        } else {
    //            while (idx < ends.size() && pos > ends[idx].end) {
    //                ++idx;
    //            }
    //        }
    //        return ends[idx].covered_idx_right - idx;
//            return 0;
//        }

        std::string intervalStr(siInterval<S>& v) {
            std::string s = "(" + std::to_string(v.start) + "-" + std::to_string(v.end) + ")";
            return s;
        }

        void search_overlap(S start, S end, std::vector<size_t>& found) {
            if (!n_intervals) {
                return;
            }
            if (!found.empty()) {
                found.clear();
            }

//            if (false || end - start < distance_threshold) {
//            if (false) {
//
//            } else {
                _binary_search(end);
                long int bl = branch_left[idx];
                long int max_right = branch_right[idx];
                long int min_right = (max_right < 0) ? std::numeric_limits<long int>::max() - 1 : max_right;
                long int start_idx = (bl >= 0 && intervals[bl].start <= end) ? std::max(bl, static_cast<long int>(idx)) : static_cast<long int>(idx);
                size_t i = static_cast<size_t>(start_idx);
                if (max_right < 0) {
                    while (i > 0) {
                        long int br = branch_right[i];
                        if (br >= 0) {
                            max_right = std::max(max_right, br);
                            min_right = std::min(min_right, br);
                            break;
                        }
                        if (is_overlapping_interval(start, end, intervals[i])) {
                            found.push_back(i);
                        } else if (intervals[i].end < start) {
                            break;
                        }
                        --i;
                    }
                }
                if (max_right >= 0) {
                    while (i > 0) {
                        if (is_overlapping_interval(start, end, intervals[i])) {
                            found.push_back(i);
                        } else if (intervals[i].end < start) {
                            break;
                        }
                        --i;
                    }
                }
                if (i == 0 && is_overlapping_interval(start, end, intervals[i])) {
                    found.push_back(i);
                }

                // Recurse branches
                if (max_right > start_idx) {
                    while (max_right >= 0) {
                        if (is_overlapping_interval(start, end, intervals[max_right])) {
                            found.push_back(max_right);
                            min_right = std::min(min_right, branch_right[max_right]);
                        }
                        max_right = branch_right[max_right];
                    }
                } else if (min_right < start_idx) {
                    long int i_right = static_cast<long int>(i);
                    while (min_right >= 0 && min_right < i_right) {
                        if (is_overlapping_interval(start, end, intervals[min_right])) {
                            found.push_back(min_right);
                        }
                        min_right = branch_right[min_right];
                    }
                }
//            }
            last_q_start = start;
        }
    };

}
