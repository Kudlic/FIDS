#include <iostream>
#include "cBpTree.h"
#include <stdlib.h>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <random>

int benchmarks = 0;
float GetThroughput(int opsCount, float period, int unit = 1000000)
{
    return ((float)opsCount / unit) / period;
}
float BytesToMB(int bytes){
    return (float)bytes / 1024 / 1024;
}

void smallTestCase(){
    //TESTCASE 3 
    cBpTree<int, int> tree(2, 5, 5);

    // Insert some data
    srand(170400);
    cTuple<int> * tupleContainer;

    int recordsNum = 256 * 256;

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < 256; i++) {
        for(int j = 0; j < 256; j++) {
            int* data = new int[2]{i, j};
            tupleContainer->setTuple(data, 2);
            if(!tree.insert(*tupleContainer)){
                printf("Insertion failed!\n");
                break;
            }
            delete data;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));

    printf("Insert BpTable done. Time: %.2fs, Throughput: %.2f mil. op/s.\n\n", time_span.count(), GetThroughput(recordsNum, time_span.count()));
    
    //memory clean up
}

void zAddressTranslationTest(){
    cBpTree<int, int> tree(2, 5, 5);
    cTuple<int> tuple1 = cTuple<int>(new int[2]{0xFF, 0x00}, 2);

    char * zAddress;
    tree.transformDataToZAddress(tuple1.getAttributes(), zAddress);
    cTuple<char> tuple2 = cTuple<char>(zAddress, tree.getMetadata()->zAddressBytes, true);

    int * unZAddressed;
    tree.transformZAddressToData(tuple2.getAttributes(), unZAddressed);
    cTuple<int> tuple3 = cTuple<int>(unZAddressed, 2);

    tuple1.printTupleHex();
    tuple2.printTupleHex();
    tuple3.printTupleHex();
}
int main() {
    zAddressTranslationTest();
    smallTestCase();
    return 0;
}
