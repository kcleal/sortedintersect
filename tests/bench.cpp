
#include "IITree.hpp"
#include "IntervalTree.h"
#include "sintersect.hpp"


#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <utility>
#include <unordered_set>


struct BedInterval {
    int start;
    int end;
};



void load_intervals(const std::string& intervals_file,
                    const std::string& queries_file,
                    std::vector<BedInterval>& intervals,
                    std::vector<BedInterval>& queries,
                    bool shuffle) {
    intervals.clear();
    queries.clear();
    std::ifstream intervals_stream(intervals_file);
    std::ifstream queries_stream(queries_file);
    if (!intervals_stream || !queries_stream) {
        throw std::runtime_error("Failed to open input files");
    }

    std::string line;
    while (std::getline(intervals_stream, line)) {
        std::istringstream iss(line);
        std::string token;
        std::getline(iss, token, '\t');  // Skip the first token (chromosome)
        std::getline(iss, token, '\t');
        int start = std::stoi(token);
        std::getline(iss, token, '\t');
        int end = std::stoi(token);
        intervals.emplace_back(BedInterval{std::min(start, end), std::max(start, end)});
    }

    while (std::getline(queries_stream, line)) {
        std::istringstream iss(line);
        std::string token;
        std::getline(iss, token, '\t');
        std::getline(iss, token, '\t');
        int start = std::stoi(token);
        std::getline(iss, token, '\t');
        int end = std::stoi(token);
        queries.emplace_back(BedInterval{std::min(start, end), std::max(start, end)});
    }

    if (shuffle) {
//        std::random_device rd;
        std::mt19937 g(12345);
        std::shuffle(queries.begin(), queries.end(), g);
    } else {
        std::sort(queries.begin(), queries.end(), [](const BedInterval& a, const BedInterval& b) {
            return (a.start < b.start);
//            return (a.start < b.start && a.end < b.end);
//            return (a.start < b.start || (a.start == b.start && a.end < b.end));
        });
    }

    std::sort(intervals.begin(), intervals.end(), [](const BedInterval& a, const BedInterval& b) {
        return (a.start < b.start);
//        return (a.start < b.start && a.end < b.end);
//        return (a.start < b.start || (a.start == b.start && a.end < b.end));
    });
    std::cout << " N ref intervals " << intervals.size() << " N queries " << queries.size() << std::endl;

}


void run_tools(std::vector<BedInterval>& intervals, std::vector<BedInterval>& queries) {
    size_t found;
    int index;
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::microseconds;

    high_resolution_clock::time_point t1;
    microseconds ms_int;
    std::vector<size_t> a, b;
    a.reserve(10000);
    b.reserve(10000);

    sortedIntersect::SIntersect<int, int> itv;
    std::cout << "\n SIntersect \n";
    itv = sortedIntersect::SIntersect<int, int>();
    //                    branch_right


//    itv.add(10, 20, -1);
//    itv.add(11, 12, -1);
//    itv.add(13, 14, -1);
//    itv.add(15, 16, -1);
//    itv.add(25, 29, -1);
//    itv.index();
//    itv.search_overlap(17, 30, a);

//    itv.add(1, 2, -1);
//    itv.add(3, 8, -1);
//    itv.add(5, 7, -1);
//    itv.add(7, 20, -1);  // 7
//    itv.add(9, 10, -1);  // 3
//    itv.add(13, 15, -1); // 6
//    itv.add(15, 16, -1); // 3
//    itv.add(19, 30, -1); // -1
//    itv.add(22, 24, -1);
//    itv.add(24, 25, -1);
//    itv.add(26, 28, -1);
//    itv.add(32, 39, -1);
//    itv.add(34, 36, -1);
//    itv.add(38, 40, -1);
//    itv.index();
//    itv.search_overlap(17, 21, a);

//    itv.add(0, 250000000, -1);
//    itv.add(55, 1055, -1);
//    itv.add(115, 1115, -1);
//    itv.add(130, 1130, -1);
//    itv.add(281, 1281, -1);
//    itv.add(639, 1639, -1);
//    itv.add(842, 1842, -1);
//    itv.add(999, 1999, -1);
//    itv.add(1094, 2094, -1);
//    itv.add(1157, 2157, -1);
//    itv.add(1161, 2161, -1);
//    itv.add(1265, 2265, -1);
//    itv.add(1532, 2532, -1);
//    itv.add(1590, 2590, -1);
//    itv.add(1665, 2665, -1);
//    itv.add(1945, 2945, -1);
//    itv.add(2384, 3384, -1);
//    itv.add(2515, 3515, -1);
//    itv.index();
//    itv.search_overlap(1377, 2377, a);
//
//    std::cout << " Found:\n";
//    for (auto item : a) {
//        std::cout << item << " - " << itv.intervals[item].start << " " << itv.intervals[item].end << std::endl;
//    }
//    return;



    std::cout << "\n SIntersect \n";
    index = 0;
    found = 0;
    t1 = high_resolution_clock::now();
//    itv = SIntersect<int, int>();
    itv.add(0, 250000000, -1);

    std::unordered_set<int> mySet;

    for (const auto& item : intervals) {
        itv.add(item.start, item.end, index);
        index += 1;
    }
    itv.index();
    ms_int = duration_cast<microseconds>(high_resolution_clock::now() - t1);
    std::cout << ms_int.count() << " construct ms" << std::endl;


    t1 = high_resolution_clock::now();
    for (const auto& item : queries) {
        itv.search_overlap(item.start, item.end, a);
        found += a.size();
//        for (auto & item: a) {
//            mySet.insert(queries[item].start);
//        }
    }

//    return;
    ms_int = duration_cast<microseconds>(high_resolution_clock::now() - t1);
    std::cout << ms_int.count() << "ms, " << found << std::endl;
    std::cout << itv.COUNTER << std::endl;

    std::cout << "\n IITree \n";
    IITree<int, int> tree;
    index = 0;
    found = 0;
    t1 = high_resolution_clock::now();
    tree.add(0, 250000000, -1);
    for (const auto& item : intervals) {
        tree.add(item.start, item.end, index);
        index += 1;
    }
    tree.index();
    std::cout << ms_int.count() << " construct ms" << std::endl;

    t1 = high_resolution_clock::now();
    for (const auto& item : queries) {
        tree.overlap(item.start, item.end, b);
        found += b.size();
    }
    ms_int = duration_cast<microseconds>(high_resolution_clock::now() - t1);
    std::cout << ms_int.count() << "ms, " << found << std::endl;


    std::cout << "\n IntervalTree \n";
    std::vector<Interval<int, int>> intervals2;
    index = 0;
    found = 0;
    t1 = high_resolution_clock::now();
    intervals2.push_back(Interval<int, int>(0, 250000000, -1));
    for (const auto& item : intervals) {
        intervals2.push_back(Interval<int, int>(item.start, item.end, index));
        index += 1;
    }
    IntervalTree<int, int> tree2(std::move(intervals2));
    std::cout << ms_int.count() << " construct ms" << std::endl;

    std::vector<Interval<int, int>> results;
    results.reserve(10000);

    int missing = 0;

    t1 = high_resolution_clock::now();
    for (const auto& item : queries) {
        auto result = tree2.findOverlapping(item.start, item.end);
        results.insert(results.end(), result.begin(), result.end());
        found += result.size();
//        if (mySet.find(item.start) == mySet.end()) {
//            missing += 1;
//            std::cerr << " missing " << item.start << " " << item.end << std::endl;
//        }

    }

    ms_int = duration_cast<microseconds>(high_resolution_clock::now() - t1);
    std::cout << ms_int.count() << "ms, " << found << std::endl;


    std::cerr << missing << std::endl;
}

int main(int argc, char *argv[]) {

    std::vector<BedInterval> intervals;
    std::vector<BedInterval> queries;
    bool shuffle = false;
//    bool shuffle = true;

    std::cout << "\n****** Reads+genes2 ******\n";
    load_intervals("a.bed", "b.bed", intervals, queries, shuffle);
    run_tools(intervals, queries);

//    run_tools(queries, intervals);

//    std::cout << "\n\n***** Random2 ******\n";
//    load_intervals("a.bed", "b.bed", intervals, queries, shuffle);
//    run_tools(intervals, queries);
//    run_tools(queries, intervals);


}