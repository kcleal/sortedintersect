
#include "sintersect.hpp"
#include <iostream>



int main(int argc, char *argc[]) {

    std::cout << "\n SIntersect \n";
    itv = SIntersect<int, int>();
//    itv.add(0, 10000, -1);
    itv.add(0, 10, -1);
    itv.add(5, 20, -1);
    itv.add(6, 7, -1);
    itv.add(11, 12, -1);
    itv.add(14, 17, -1);

//    itv.add(50, 200, -1);
//    itv.add(50, 300, -1);
    itv.index();


//    itv.search_overlap(20, 51, a);

}