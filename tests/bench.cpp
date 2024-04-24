
#include "IITree.hpp"
//#include "iitii.hpp"
#include "sintersect.hpp"
//#include "AIList.h"

#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <utility>


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
            return a.start < b.start;
        });
    }

    std::sort(intervals.begin(), intervals.end(), [](const BedInterval& a, const BedInterval& b) {
        return a.start < b.start;
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

    SIntersect<int, int> itv;

    std::cout << "\n SIntersect \n";
    itv = SIntersect<int, int>();

    itv.add(1, 7, -1);
    itv.add(6, 14, -1);
    itv.add(7, 9, -1);
    itv.add(11, 13, -1);
    itv.add(11, 12, -1);
    itv.add(12, 13, -1);
    itv.add(12, 22, -1);
    itv.add(14, 17, -1);

    itv.index();

    itv.search_overlap(8, 12, a);
    for (auto item : a) {
        std::cout << " - " << itv.starts[item] << " " << itv.ends[item].end << std::endl;
    }
    return;

    std::cout << "\n SIntersect \n";
    itv = SIntersect<int, int>();
//    itv.add(0, 250000000, -1);
    index = 0;
    for (const auto& item : intervals) {
        itv.add(item.start, item.end, index);
        index += 1;
    }
    itv.index();

    t1 = high_resolution_clock::now();
    found = 0;
    for (const auto& item : queries) {
        itv.search_overlap(item.start, item.end, a);
        found += a.size();
    }
    ms_int = duration_cast<microseconds>(high_resolution_clock::now() - t1);
    std::cout << ms_int.count() << "ms, " << found << std::endl;
    std::cout << " COUNTER " << itv.COUNTER << std::endl;
//    return;
    std::cout << "\n IITree \n";
    IITree<int, int> tree;
    index = 0;
//    tree.add(0, 250000000, -1);
    for (const auto& item : intervals) {
        tree.add(item.start, item.end, index);
        index += 1;
    }
    tree.index();

    t1 = high_resolution_clock::now();
    found = 0;
    for (const auto& item : queries) {
        tree.overlap(item.start, item.end, b);
        found += b.size();
    }
    ms_int = duration_cast<microseconds>(high_resolution_clock::now() - t1);
    std::cout << ms_int.count() << "ms, " << found << std::endl;


      // first arg is position type
//    p_iitii::builder br;
//    index = 0;
//    for (const auto& item : intervals) {
//        br.add(item.start, item.end);
//        index += 1;
//    }
//    p_iitii db = br.build(1);

//    t1 = high_resolution_clock::now();
//    found = 0;
//    for (const auto& item : queries) {
//        std::vector<const intpair*> results = db.overlap(item.start, item.end);
//        found += results.size();
//    }
//    ms_int = duration_cast<microseconds>(high_resolution_clock::now() - t1);
//    std::cout << ms_int.count() << "ms, " << found << std::endl;


}

int main(int argc, char *argv[]) {

    std::vector<BedInterval> intervals;
    std::vector<BedInterval> queries;
    bool shuffle = false;
//    bool shuffle = true;

    std::cout << "\n****** Reads+genes2 ******\n";
    load_intervals("chr1_ucsc_genes.bed", "chr1_reads.bed", intervals, queries, shuffle);
    run_tools(intervals, queries);

//    run_tools(queries, intervals);

//    std::cout << "\n\n***** Random2 ******\n";
//    load_intervals("a.bed", "b.bed", intervals, queries, shuffle);
//    run_tools(intervals, queries);
//    run_tools(queries, intervals);


}