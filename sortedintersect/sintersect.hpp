
#include <vector>
#include <algorithm>


bool is_overlapping(int x1, int x2, int y1, int y2) {
    return std::max(x1, y1) <= std::min(x2, y2);
}


struct Interval {
    int end;
    int covered_end;
};



class SIntersect {
    public:
    SIntersect() = default;
    ~SIntersect() = default;

    std::vector<int> starts;
    std::vector<Interval> ends;
    int last_r_start, last_q_start, current_r_end, current_r_start, distance_threshold;
    size_t index;

    void add(int start, int end) {
        starts.push_back(start);
        ends.emplace_back() = {end, current_r_end};
        current_r_end = std::max(current_r_end, end);
        last_r_start = start;
    }

    void _line_scan(int pos) {
        if (pos < starts[index]) {
            while (index > 0 && pos <= starts[index]) {
                --index;
            }
            while (true) {
                if (ends[index].covered_end >= pos) {
                    if (index > 0) {
                        --index;
                    } else {
                        break;
                    }
                } else {
                    if (index < ends.size() - 1) {
                        ++index;
                    } else {
                        break;
                    }
                }
            }
        } else {
            while (index < ends.size() && pos > ends[index].end) {
                ++index;
            }
        }
    }

    void _binary_search(int pos) {
        auto lower = std::upper_bound(starts.begin(), starts.end(), pos);
        index = lower - starts.begin();
        index = (index > 0) ? index - 1 : index;
        if (pos >= starts[index]) {
            while (true) {
                if (ends[index].covered_end >= pos) {
                    if (index > 0) {
                        index -= 1;
                    } else {
                        break;
                    }
                } else {
                    if (index < ends.size() - 1) {
                        index += 1;
                    } else {
                        break;
                    }
                }
            }
        }
    }

    void _set_reference_index(int pos) {
        if (std::abs(pos - last_q_start) > distance_threshold) {
            _binary_search(pos);
        } else {
            _line_scan(pos);
        }
    }


    void search_point(int pos) {

    }

};