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
    cBpTree<int> tree(6, 2, 4, 5, 5);

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
    if(records <= 1000)tree.printTree();
    tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));

    printf("Insert BpTable done. Time: %.2fs, Throughput: %.2f mil. op/s.\n\n", time_span.count(), GetThroughput(records, time_span.count()));
    
    
    cTuple searchTupLow = cTuple(new int[6]{41, 0, -1, -1, -1, -1}, 6);
    cTuple searchTupHigh = cTuple(new int[6]{41, 1, -1, -1, -1, -1}, 6);
    int * result = new int[records * 6];
    int tupleCount1 = tree.searchRangeNoAlloc(searchTupLow, searchTupHigh, result, true);
    int tupleCount2 = tree.searchRangeNoAlloc(searchTupLow, searchTupLow, result, true);
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
        int tupleCount = tree.searchRangeNoAlloc(*tuplesLow[i], *tuplesHigh[i], result, 0);
        if(tupleCount > 0) queriesFound++;
        /*if(tupleCount == 0){
            tuplesLow[i]->printTuple();
            tuplesHigh[i]->printTuple();
            printf("---------------------------\n");
        }*/
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Select range BpTable done. Time: %.2fs, Throughput: %.2f k. op/s.\n", time_span.count(), GetThroughput(queries, time_span.count(), 1000));
    printf("Queries found: %d/%d\n\n", queriesFound, queries);
    queriesFound = 0;
    

    t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queries; i++) {
        int tupleCount = tree.searchRangeNoAlloc(*tuplesLow[i], *tuplesLow[i], result, 0);
        if(tupleCount > 0) queriesFound++;
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

    printf("Result mismatches: \n");
    for(int i = 0; i < queries; i++) {
        int tupleCount = tree.pointSearch(*tuplesLow[i]);
        int tupleCountX = tree.searchRangeNoAlloc(*tuplesLow[i], *tuplesLow[i], result, 0);
        if(tupleCount != tupleCountX) {
            printf("%d: %d, %d\n", i, tupleCount, tupleCountX);
            printf("Query: ");
            tuplesLow[i]->printTuple();
            printf("---------------------------\n");
        }

    }

    for(int i = 0; i < queries; i++) {
        delete tuplesLow[i];
        delete tuplesHigh[i];
    }
}
void deleteTestCase(){
    cBpTree<int> tree(2, 1, 1, 5, 5);
    // Insert some data
    srand(170400);
    int records = 20;

    cTuple maximum = cTuple(new int[2]{50, -1}, 2);

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < records; i++) {
        cTuple tuple(new int[2]{rand()% maximum.attributes[0], i}, 2);
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    if(records <= 1000)tree.printTree();
    //tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));

    int deleteVals[20] = {0, 1, 2, 4, 5, 5, 8, 12, 15, 18, 26, 28, 30, 36, 36, 37, 38, 39, 46, 46};
    for(int i = 0; i < 20; i++){
        cTuple tuple(new int[1]{deleteVals[i]}, 1);
        printf("Deleting: ");
        tuple.printTuple();
        tree.remove(tuple);
        tree.printTree();
    }
}
void deleteTestCaseRev(){
    cBpTree<int> tree(2, 1, 1, 5, 5);
    // Insert some data
    srand(170400);
    int records = 20;

    cTuple maximum = cTuple(new int[2]{50, -1}, 2);

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < records; i++) {
        cTuple tuple(new int[2]{rand()% maximum.attributes[0], i}, 2);
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    if(records <= 1000)tree.printTree();
    //tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));

    int deleteVals[20] = {0, 1, 2, 4, 5, 5, 8, 12, 15, 18, 26, 28, 30, 36, 36, 37, 38, 39, 46, 46};
    for(int i = 19; i >=0; i--){
        cTuple tuple(new int[1]{deleteVals[i]}, 1);
        printf("Deleting: ");
        tuple.printTuple();
        tree.remove(tuple);
        tree.printTree();
    }
}
void iteratorTestCase(){
    cBpTree<int> tree(2, 1, 1, 5, 5);
    // Insert some data
    srand(170400);
    int records = 50;

    cTuple maximum = cTuple(new int[2]{70, -1}, 2);

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < records; i++) {
        cTuple tuple(new int[2]{(rand()% maximum.attributes[0])+20, i}, 2);
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    if(records <= 1000)tree.printTree();
    //tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));

    cTuple<int> searchTup2 = cTuple(new int[1]{2}, 1);
    cTuple<int> searchTup20 = cTuple(new int[1]{20}, 1);
    cTuple<int> searchTup25 = cTuple(new int[1]{25}, 1);
    cTuple<int> searchTup40 = cTuple(new int[1]{40}, 1);
    cTuple<int> searchTup57 = cTuple(new int[1]{57}, 1);
    cTuple<int> searchTup59 = cTuple(new int[1]{59}, 1);
    cTuple<int> searchTup70 = cTuple(new int[1]{70}, 1);
    cTuple<int> searchTup88 = cTuple(new int[1]{88}, 1);
    cTuple<int> searchTup90 = cTuple(new int[1]{90}, 1);

    cBpTreeIteratorRange<int>* iterator = nullptr;
    //both left outside
    iterator = tree.searchRangeIterator(searchTup2, searchTup20);
    printf("Iterator test 2-20 skip count: %d\n", iterator->skip(-1));
    delete iterator;
    iterator = tree.searchRangeIterator(searchTup2, searchTup20);
    while(iterator->hasNext()){
        cTuple<int>* tuple = iterator->next();
        tuple->printTuple();
    }
    delete iterator;

    //left outside, right present
    iterator = tree.searchRangeIterator(searchTup20, searchTup25);
    printf("Iterator test 20-25 skip count: %d\n", iterator->skip(-1));
    delete iterator;
    iterator = tree.searchRangeIterator(searchTup20, searchTup25);
    while(iterator->hasNext()){
        cTuple<int>* tuple = iterator->next();
        tuple->printTuple();
    }
    delete iterator;

    //left present, right present
    iterator = tree.searchRangeIterator(searchTup25, searchTup57);
    printf("Iterator test 25-57 skip count: %d\n", iterator->skip(-1));
    delete iterator;
    iterator = tree.searchRangeIterator(searchTup25, searchTup57);
    while(iterator->hasNext()){
        cTuple<int>* tuple = iterator->next();
        tuple->printTuple();
    }
    delete iterator;

    //left present, right outside
    iterator = tree.searchRangeIterator(searchTup88, searchTup90);
    printf("Iterator test 88-90 skip count: %d\n", iterator->skip(-1));
    delete iterator;
    iterator = tree.searchRangeIterator(searchTup88, searchTup90);
    while(iterator->hasNext()){
        cTuple<int>* tuple = iterator->next();
        tuple->printTuple();
    }
    delete iterator;

    //both right outside
    iterator = tree.searchRangeIterator(searchTup90, searchTup90);
    printf("Iterator test 90-90 skip count: %d\n", iterator->skip(-1));
    delete iterator;
    iterator = tree.searchRangeIterator(searchTup90, searchTup90);
    while(iterator->hasNext()){
        cTuple<int>* tuple = iterator->next();
        tuple->printTuple();
    }
    delete iterator;

}
/*
void largeTestCase(){
    //TESTCASE 3 
    cBpTree<int> tree(6, 2, 4, 64, 32);

    // Insert some data
    srand(170400);
    int records = 10000000;
    float rangeMultiplierGen = 0.05;

    cTuple maximum = cTuple(new int[6]{20000, 500, -1, -1, -1, -1}, 6);

    cTuple<int>** tuplesInsert = new cTuple<int>*[records];
    for(int i = 0; i < records; i++) {
        tuplesInsert[i] = new cTuple<int>(new int[6]{rand()% maximum.attributes[0], rand()%maximum.attributes[1], i, i, i, i}, 6);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < records; i++) {
        if(!tree.insert(*tuplesInsert[i])){
            printf("Insertion failed!\n");
            break;
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    tree.printMetadata();
    printf("BpTree MB: %.3f\n\n",BytesToMB(tree.getBpTreeBytes()));

    printf("Insert BpTable done. Time: %.2fs, Throughput: %.2f mil. op/s.\n\n", time_span.count(), GetThroughput(records, time_span.count()));
    
    
    cTuple searchTupLow = cTuple(new int[6]{41, 0, -1, -1, -1, -1}, 6);
    int * result = new int[records * 6];
    int tupleCount1 = tree.searchRangeNoAlloc(searchTupLow, searchTupLow, result);
    delete [] result;

    result = new int[records * 6];
    int tupleCount2 = tree.searchRangeNoAlloc(searchTupLow, searchTupLow, result);
    delete [] result;

    int tupleCount3 = tree.pointSearch(searchTupLow);

    printf("Query: {41, 0}-{41, 1}, found: %d\n", tupleCount1);
    printf("Query: {41, 0}, found: %d\n", tupleCount2);
    printf("Query: {41, 0}, found: %d\n", tupleCount3);


    int queries = 1000;
    int queriesFound = 0;
    cTuple<int>** tuplesLow = new cTuple<int>*[queries];
    cTuple<int>** tuplesHigh = new cTuple<int>*[queries];
    for(int i = 0; i < queries; i++) {
        tuplesLow[i] = new cTuple<int>(
            new int[6]{
                rand()% maximum.attributes[0], 
                rand()%maximum.attributes[1],
                i, i, i, i}, 
            6);
        tuplesHigh[i] = new cTuple<int>(
            new int[6]{
                (rand() % (int)(maximum.attributes[0] * rangeMultiplierGen)) + tuplesLow[i]->attributes[0], 
                (rand() % (int)(maximum.attributes[1] * rangeMultiplierGen)) + tuplesLow[i]->attributes[1],
                i, i, i, i}, 
            6);
        if(tuplesLow[i]->isGT(*tuplesHigh[i])){
            cTuple<int>* temp = tuplesLow[i];
            tuplesLow[i] = tuplesHigh[i];
            tuplesHigh[i] = temp;
        }
    }
    queriesFound = 0;
    
    t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queries; i++) {
        int * result = new int[records * 6];
        int tupleCount = tree.searchRangeNoAlloc(*tuplesLow[i], *tuplesHigh[i], result);
        if(tupleCount > 0) queriesFound++;
        delete [] result;
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Select range BpTable done. Time: %.2fs, Throughput: %.2f k. op/s.\n", time_span.count(), GetThroughput(queries, time_span.count(), 1000));
    printf("Queries found: %d/%d\n\n", queriesFound, queries);
    queriesFound = 0;
    

    int * results1 = new int[queries];
    int * results2 = new int[queries];

    t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queries; i++) {
        int * result = new int[records * 6];
        int tupleCount = tree.searchRangeNoAlloc(*tuplesLow[i], *tuplesLow[i], result);
        results1[i] = tupleCount;
        if(tupleCount > 0) queriesFound++;
        delete [] result;
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
        results2[i] = tupleCount;
    }
    t2 = std::chrono::high_resolution_clock::now();
    time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Select point BpTable done, no results. Time: %.2fs, Throughput: %.2f k. op/s.\n", time_span.count(), GetThroughput(queries, time_span.count(), 1000));
    printf("Queries found: %d/%d\n\n", queriesFound, queries);
    queriesFound = 0;

    printf("Result mismatches: \n");
    for(int i = 0; i < queries; i++) {
        int bruteForceCount = 0;
        for(int k = 0; k < records; k++){
            if(tuplesInsert[k]->isEQ(*tuplesLow[i], 2)){
                bruteForceCount++;
            }
        }
        if(results1[i] != bruteForceCount || results2[i] != bruteForceCount){
            printf("%d: %d, %d, Brute: %d\n", i, results1[i], results2[i], bruteForceCount);
            printf("Query: ");
            tuplesLow[i]->printTuple();
            printf("---------------------------\n");
        }
    }

    for(int i = 0; i < queries; i++) {
        delete tuplesLow[i];
        delete tuplesHigh[i];
    }
    delete [] tuplesLow;
    delete [] tuplesHigh;
    delete [] results1;
    delete [] results2;
}
*/

//records is array of record counts
//with each loop we will fill tree to records[i] and then do x queries
void benchmark(int* records, int recordsSize, int queries, int n1, int n2, int nodeSize, char delimiter = '|'){
    int n = n1+n2;
    cBpTree<int> tree(n, n1, n2, nodeSize, nodeSize);
    cTuple<int>** tuplesInsert = new cTuple<int>*[records[recordsSize-1]];
    cTuple<int>** tuplesLow = new cTuple<int>*[queries];
    cTuple<int>** tuplesHigh = new cTuple<int>*[queries];

    int rangeMultiplierGen = 0.05;
    int maxValRange = 4092;
    srand(n);
    for(int i = 0; i < records[recordsSize-1]; i++) {
        int * attributes = new int[n];
        int valRange = maxValRange;
        for(int j = 0; j < n1; j++){
            attributes[j] = rand() % valRange;
            valRange = std::max(valRange/2, 1);
        }
        for(int j = n1; j < n; j++){
            attributes[j] = i;
        }
        tuplesInsert[i] = new cTuple<int>(attributes, n);
    }
    for(int i = 0; i < queries; i++) {
        int * attributesLow = new int[n];
        int * attributesHigh = new int[n];
        int valRange = maxValRange;
        attributesLow[0] = rand() % std::max((int)(valRange-(valRange*rangeMultiplierGen)),1);
        attributesHigh[0] = (rand() % std::max((int)(valRange * rangeMultiplierGen), 1))+ attributesLow[0];
        for(int j = 1; j < n1; j++){
            valRange = std::max(valRange/2, 1);
            attributesLow[j] = rand() % (valRange);
            attributesHigh[j] = rand() % (valRange);
        }
        for(int j = n1; j < n; j++){
            attributesLow[j] = i;
            attributesHigh[j] = i;
        }
        tuplesLow[i] = new cTuple<int>(attributesLow, n);
        tuplesHigh[i] = new cTuple<int>(attributesHigh, n);
        if(tuplesLow[i]->isGT(*tuplesHigh[i])){
            cTuple<int>* temp = tuplesLow[i];
            tuplesLow[i] = tuplesHigh[i];
            tuplesHigh[i] = temp;
        }
    }
    if(benchmarks == 0)
        printf("Config %c TupleCount %c BpTree size %c Data size %c Insert time (s) %c Insert k. op/s %c RangeQuery time (s) %c RangeQuery k. op/s %c PointQuery time (s) %c PointQuery k. op/s\n", delimiter, delimiter, delimiter, delimiter, delimiter, delimiter, delimiter, delimiter, delimiter);
    benchmarks++;
    for(int m = 0; m < recordsSize; m++){
        printf("\"n1: %d, n2: %d, nodeSize: %d, queries: %d\"%c ", n1, n2, nodeSize, queries, delimiter);
        fflush(stdout);
        auto t1 = std::chrono::high_resolution_clock::now();
        for(int i = tree.getTupleCount(); i < records[m]; i++) {
            if(!tree.insert(*tuplesInsert[i])){
                printf("Insertion failed!\n");
                break;
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span_insert = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        printf("%d %c %.3f MB %c %.3f MB %c %.3f %c %.3f %c ", 
            records[m], delimiter, 
            BytesToMB(tree.getBpTreeBytes()), delimiter, 
            BytesToMB(sizeof(int)*n*records[m]), delimiter, 
            time_span_insert.count(), delimiter,
            GetThroughput(records[m], time_span_insert.count(), 1000), delimiter);
        fflush(stdout);
        //tree.printBpTree();
        int * result1 = new int[records[m] * n];
        t1 = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < queries; i++) {
            //printf("Query: %d\n", i);
            //tuplesLow[i]->printTuple();
            //tuplesHigh[i]->printTuple();
            int tupleCount = tree.searchRangeNoAlloc(*tuplesLow[i], *tuplesHigh[i], result1, 0);
        }
        t2 = std::chrono::high_resolution_clock::now();
        delete [] result1;
        std::chrono::duration<double> time_span_range = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        printf("%.3f %c %.3f %c ", 
            time_span_range.count(), delimiter, 
            GetThroughput(queries, time_span_range.count(), 1000), delimiter);
        fflush(stdout);

        int * result2 = new int[records[m] * n];
        t1 = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < queries; i++) {
            int tupleCount = tree.searchRangeNoAlloc(*tuplesLow[i], *tuplesLow[i], result2, 0);
        }
        t2 = std::chrono::high_resolution_clock::now();
        delete [] result2;
        std::chrono::duration<double> time_span_point = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        printf("%.3f %c %.3f\n", 
            time_span_point.count(), delimiter, 
            GetThroughput(queries, time_span_point.count(), 1000));
        fflush(stdout);
    }

    for(int i = 0; i < records[recordsSize-1]; i++) {
        delete tuplesInsert[i];
    }

    for(int i = 0; i < queries; i++) {
        delete tuplesLow[i];
        delete tuplesHigh[i];
    }
    delete [] tuplesInsert;
    delete [] tuplesLow;
    delete [] tuplesHigh;
}

int main() {
    //iteratorTestCase();
    deleteTestCase();
    //deleteTestCaseRev();
    //largeTestCase();
    
    /*
    int recSize = 5;
    int * records = new int[recSize]{int(1e3), int(1e4), int(1e5), int(1e6), int(4e6)};
    benchmark(records, recSize, 1000, 2,  2, 16,  ',');
    benchmark(records, recSize, 1000, 2,  2, 32,  ',');
    benchmark(records, recSize, 1000, 2,  2, 64,  ',');
    benchmark(records, recSize, 1000, 2,  2, 128, ',');
    benchmark(records, recSize, 1000, 4,  8, 64,  ',');
    benchmark(records, recSize, 1000, 4,  8, 128, ',');
    benchmark(records, recSize, 1000, 8,  8, 64,  ',');
    benchmark(records, recSize, 1000, 8,  8, 128, ',');
    benchmark(records, recSize, 1000, 16, 4, 64,  ',');
    benchmark(records, recSize, 1000, 16, 4, 128, ',');
    */


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
    return 0;
}
