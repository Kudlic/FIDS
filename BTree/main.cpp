#include <iostream>
#include "cBpTree.h"
#include <stdlib.h>
#include <chrono>


float GetThroughput(int opsCount, float period, int unit = 1000000)
{
    return ((float)opsCount / unit) / period;
}
float BytesToMB(int bytes){
    return (float)bytes / 1024 / 1024;
}

int main() {
    // Create a B+ tree with 2 indexed attributes and 2 non-indexed attributes
    // and maximum 3 elements in a inner node and 8 elements in a leaf node

//TEST 1 recasting
/*
    cTuple<int> tuple(new int[4]{2897, 54863, 71389, 111111}, 4);
    char* tuplePtr = new char[16];
    memcpy(tuplePtr, tuple.attributes, 16);
    cTuple<int> tuple2((int*)tuplePtr, 4);

    tuple.printTuple();
    tuple2.printTuple();
*/
//TEST 2 pointer serialisation
/*
    cTuple<int> *tuple = new cTuple<int>(new int[4]{2897, 54863, 71389, 111111}, 4);
    printf("Tuple pointer: %p\n", tuple);
    printf("Tuple address: %p\n", &tuple);

    char* tuplePtr = new char[16];
    printf("Sizeof tuplePtr: %d\n", sizeof(tuple));
    printf("Sizeof tuple: %d\n", sizeof(cTuple<int>));
    memcpy(tuplePtr, &tuple, sizeof(tuple));

    //print tuplePtr in hexa
    for(int i = 0; i < 16; i++){
        printf("%02x", tuplePtr[i]);
    }
    printf("\n");

    cTuple<int> *tuple2;
    memcpy(&tuple2, tuplePtr, sizeof(char*));

    printf("Tuple2 pointer: %p\n", tuple2);
    printf("Tuple2 address: %p\n", &tuple2);
    tuple->printTuple();
    tuple2->printTuple();
*/
//TEST 3 deleting
/*
    for(int c = 0; c < 4000000; c++){
        cTuple maximum = cTuple(new int[6]{100, 50, 5, 15, -1, -1}, 6);
    }
    for(int t = 0; t < 10; t++){
        cBpTree<int>* testTree = new cBpTree<int> (6,4,2,4,4);
        cTuple maximum = cTuple(new int[6]{100, 50, 5, 15, -1, -1}, 6);
        int records = 500000;
        for(int i = 0; i < records; i++) {
            cTuple tuple(new int[6]{rand()% maximum.attributes[0], rand()%maximum.attributes[1], rand()%maximum.attributes[2], rand()%maximum.attributes[3], i, i/2}, 6);
            if(!testTree->insert(tuple)){
                printf("Insertion failed!\n");
                break;
            }
        }
        //testTree->printBpTree();
        delete testTree;
    }
    return 0;
*/
//TESTCASE 1
/*
    cBpTree<int> tree(4, 3, 1, 8, 8);

    // Insert some data
    srand(170400);
    int records = 1000000;

    cTuple maximum = cTuple(new int[4]{100, 50, 5, -1}, 4);

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < records; i++) {
        cTuple tuple(new int[4]{rand()% maximum.attributes[0], rand()%maximum.attributes[1], i%maximum.attributes[2], i}, 4);
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    //tree.printBpTree();
    tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));

    printf("Insert BpTable done. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(records, time_span.count()));
    
    cTuple searchTup = cTuple(new int[4]{24, 7, 0, 49}, 4);
    printf("Query: {24, 7}, found: %d\n", tree.pointSearch(searchTup) ? 1:0);
    int queries = 1000;
    int queriesFound = 0;
    t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queries; i++) {
        cTuple tuple(new int[4]{rand()% maximum.attributes[0], rand()%maximum.attributes[1], i%maximum.attributes[2], i}, 4);
        if(tree.pointSearch(tuple)) queriesFound++;
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Select point BpTable done. Time: %.2fs, Throughput: %.2f k. op/s.\n", time_span.count(), GetThroughput(queries, time_span.count(), 1000));
    */

    /*
    t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queries; i++) {
        cTuple tupleLow(
            new int[4]{
                rand()%50, 
                rand()%20, 
                i%5, 
                i*7}, 
            4);
        cTuple tupleHigh(
            new int[4]{
                (rand()%(std::max(50 - tupleLow.attributes[0], 1))) + tupleLow.attributes[0], 
                (rand()%(std::max(20 - tupleLow.attributes[1], 1))) + tupleLow.attributes[1], 
                (rand()%(std::max(5 - tupleLow.attributes[0], 1))) + tupleLow.attributes[2], 
                i*7}, 
            4);
        tree.searchLinkedList(tupleLow, tupleHigh);
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Select range BpTable, linked list done. Time: %.2fs, Throughput: %.2f k. op/s.\n", time_span.count(), GetThroughput(queries, time_span.count(), 1000));
    */

//TESTCASE 2
/*
    cBpTree<int> tree(2, 1, 1, 6, 5);
    int* vals = new int[9]{1,11,17,12,5,13,6,7,8};
    for(int i = 0; i < 9; i++) {
        cTuple tuple(new int[2]{vals[i], i}, 2);
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
        tree.printBpTree();
        printf("\n");
        //tree.printMetadata();
    }
*/
//TESTCASE 3 

    cBpTree<int> tree(6, 2, 4, 8, 8);

    // Insert some data
    srand(170400);
    int records = 200;

    cTuple maximum = cTuple(new int[6]{50, 5, -1, -1, -1, -1}, 6);

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < records; i++) {
        cTuple tuple(new int[6]{rand()% maximum.attributes[0], rand()%maximum.attributes[1], i, i, i, i}, 6);
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    if(records <= 1000)tree.printBpTree();
    tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));

    printf("Insert BpTable done. Time: %.2fs, Throughput: %.2f mil. op/s.\n\n", time_span.count(), GetThroughput(records, time_span.count()));
    
    
    cTuple searchTupLow = cTuple(new int[6]{41, 0, -1, -1, -1, -1}, 6);
    cTuple searchTupHigh = cTuple(new int[6]{41, 1, -1, -1, -1, -1}, 6);
    int * result;
    int all;
    int tupleCount1 = tree.searchRange(searchTupLow, searchTupHigh, result, all, true);
    if(all>=0) delete [] result;

    int tupleCount2 = tree.searchRange(searchTupLow, searchTupLow, result, all, true);
    if(all>=0) delete [] result;

    int tupleCount3 = tree.pointSearch(searchTupLow);

    printf("Query: {41, 0}-{41, 1}, found: %d\n", tupleCount1);
    printf("Query: {41, 0}, found: %d\n", tupleCount2);
    printf("Query: {41, 0}, found: %d\n", tupleCount3);


    int queries = 1000;
    int queriesFound = 0;
    cTuple<int>** tuplesLow = new cTuple<int>*[queries];
    cTuple<int>** tuplesHigh = new cTuple<int>*[queries];
    for(int i = 0; i < queries; i++) {
        tuplesLow[i] = new cTuple<int>(new int[6]{rand()% maximum.attributes[0], rand()%maximum.attributes[1], i, i, i, i}, 6);
        tuplesHigh[i] = new cTuple<int>(new int[6]{rand()% maximum.attributes[0], rand()%maximum.attributes[1], i, i, i, i}, 6);
        if(tuplesLow[i]->isGT(*tuplesHigh[i])){
            cTuple<int>* temp = tuplesLow[i];
            tuplesLow[i] = tuplesHigh[i];
            tuplesHigh[i] = temp;
        }
    }
    queriesFound = 0;
    
    t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queries; i++) {
        int * result;
        int all;
        int tupleCount = tree.searchRange(*tuplesLow[i], *tuplesHigh[i], result, all);
        if(tupleCount > 0) queriesFound++;
        if(all >= 0) delete [] result;
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Select range BpTable done. Time: %.2fs, Throughput: %.2f k. op/s.\n", time_span.count(), GetThroughput(queries, time_span.count(), 1000));
    printf("Queries found: %d/%d\n\n", queriesFound, queries);
    queriesFound = 0;
    

    t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queries; i++) {
        int * result;
        int all;
        int tupleCount = tree.searchRange(*tuplesLow[i], *tuplesLow[i], result, all);
        if(tupleCount > 0) queriesFound++;
        if(all >= 0) delete [] result;
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Select point BpTable done. Time: %.2fs, Throughput: %.2f k. op/s.\n", time_span.count(), GetThroughput(queries, time_span.count(), 1000));
    printf("Queries found: %d/%d\n\n", queriesFound, queries);
    queriesFound = 0;

    t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queries; i++) {
        int tupleCount = tree.pointSearch(*tuplesLow[i]);
        if(tupleCount > 0) queriesFound++;
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Select point BpTable done, no results. Time: %.2fs, Throughput: %.2f k. op/s.\n", time_span.count(), GetThroughput(queries, time_span.count(), 1000));
    printf("Queries found: %d/%d\n\n", queriesFound, queries);
    queriesFound = 0;

    for(int i = 0; i < queries; i++) {
        delete tuplesLow[i];
        delete tuplesHigh[i];
    }

    return 0;
}
