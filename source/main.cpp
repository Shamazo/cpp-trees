/** @file main.cpp
 * The main file for running bench marks.
 */

#include "spdlog/spdlog.h"
#include "btree.h"
#include <vector>
#include <chrono>

int benchmark_load(size_t size){
    std::vector<int> keys;
    for (int i=0; i<size; i++){
        keys.push_back(i*5);
    }
    std::vector<int> vals;
    for (int i=0; i<size; i++){
        vals.push_back(i);
    }
    Btree* tree = new Btree;
    auto start = std::chrono::system_clock::now();
    tree->load(keys, vals);

    auto end = std::chrono::system_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    spdlog::info("Took {} seconds to load {:L} key/value pairs",  seconds.count(), size);

    return 0;
}

int benchmark_point_queries(size_t size, size_t num_queries){
    std::vector<int> keys;
    for (int i=0; i<size; i++){
        keys.push_back(i*5);
    }
    std::vector<int> vals;
    for (int i=0; i<size; i++){
        vals.push_back(i);
    }
    Btree* tree = new Btree;

    tree->load(keys, vals);
    std::vector<std::vector<int>> all_vals;
    auto start = std::chrono::system_clock::now();
    for (int i =0; i < num_queries; i++){
        std::vector vals = tree->get_vals((i * 141) % size, (i * 141) % size + 1);
        all_vals.push_back(vals);
    }

    auto end = std::chrono::system_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    spdlog::info("Took {} seconds to run {} point queries on  {:L} key/value pairs",  seconds.count(), num_queries, size);
    for (int i =0; i < num_queries; i++) {
        all_vals[i][0] = -1;
    }
    return 0;
}


int main(int argc, char* argv[])
{
    benchmark_load(10000000);
    benchmark_point_queries(100000000, 100000);
    return 0;
}


