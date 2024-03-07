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
    cBpTree<int, int> tree(2, 8, 8);

    // Insert some data
    srand(170400);
    cTuple<int> * tupleContainer = new cTuple<int>(2, true);

    int jMax, iMax;
    iMax = 8;
    jMax = 8;
    int recordsNum = iMax * jMax;

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < iMax; i++) {
        for(int j = 0; j < jMax; j++) {
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
    
    tree.printTreeTuples();
    tree.printTreeHex();
    //memory clean up
}

void zAddressTranslationTest(){
    cBpTree<int, int> tree(2, 5, 5);
    cTuple<int> tuple1 = cTuple<int>(new int[2]{0x7, 0x1}, 2);

    char * zAddress = nullptr;
    tree.getZTools()->transformDataToZAddress((char*)tuple1.getAttributes(), zAddress);
    cTuple<int> tuple2 = cTuple<int>((int*)zAddress, tree.getMetadata()->n, true);

    char *unZAddressed = nullptr; // Change the declaration to reference type
    tree.getZTools()->transformZAddressToData((char*)tuple2.getAttributes(), unZAddressed); // Use the address of operator to pass the reference

    cTuple<int> tuple3 = cTuple<int>((int*)unZAddressed, 2, true);

    tuple1.printTupleHex();
    tuple2.printTupleHex();
    tuple3.printTupleHex();
}
int main() {
    zAddressTranslationTest();
    smallTestCase();
    return 0;
}
