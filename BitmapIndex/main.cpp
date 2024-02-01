#include <stdio.h>
#include <iostream>
#include <chrono>
#include <cstring>

#include "cRowHeapTable.h"

#define TKey int
#define TData int

using namespace std;
using namespace std::chrono;

float GetThroughput(int opsCount, float period, int unit = 1000000);
void RowHeapTableTest(const int rowCount);
void RowHeapTableBenchmark(const int rowCount);


int main()
{
    const int RowCount = 1e7;
    
    RowHeapTableTest(10);
    printf("\n");

    RowHeapTableBenchmark(RowCount);

    return 0;
}

float GetThroughput(int opsCount, float period, int unit)
{
    return ((float)opsCount / unit) / period;
}

void RowHeapTableTest(const int rowCount)
{
    /*
    fname CHAR(10),
    lanem CHAR(10),
    gender BYTE,
    ranking BYTE,
    department BYTE
    */
    uint * schema = new uint[5] {10,10,1,1,1};
    cRowHeapTable *rowHeapTable = new cRowHeapTable(schema, 5, rowCount);

    char fNames[20][10] = { "John", "Mary", "Robert", "Patricia", "Michael", "Jennifer", "William", "Linda", "David", "Elizabeth", "Richard", "Barbara", "Joseph", "Susan", "Thomas", "Jessica", "Charles", "Sarah", "Geralt", "Karen" };
    char lNames[20][10] = { "Smith", "Johnson", "Williams", "Brown", "Jones", "Miller", "Davis", "Garcia", "Rodriguez", "Wilson", "Martinez", "Anderson", "Taylor", "Thomas", "Hernandez", "Moore", "Martin", "Jackson", "Thompson", "White" };

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i<rowCount; i++){
        char row[23] = { 0 };
        int fNameIndex = rand() % 20;
        int lNameIndex = rand() % 20;
        memcpy(row, fNames[fNameIndex], 10);
        memcpy(row + 10, lNames[lNameIndex], 10);
        row[20] = rand() % 2 == 0 ? 0 : 1;
        row[21] = rand() % 3  ;
        row[22] = rand() % 5  ;
        rowHeapTable->Insert(row);
    }    
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HeapTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    if (rowCount <= 100){
        for (int i = 0; i < rowCount; i++)
        {
            rowHeapTable->PrintRow(i);
        }
    }
    uint query[][2] = {{2, 0}, {3, 1}};

    int attr_len[] = {2,3,5};
    rowHeapTable->CreateBitmapIndex(attr_len, sizeof(attr_len)/sizeof(int), rowCount);

    rowHeapTable->Select(query, 2, true);
    rowHeapTable->SelectBitmap(query, 2, true);
    rowHeapTable->SelectMask(query, 2, false);
    delete rowHeapTable;
}
void RowHeapTableBenchmark(const int rowCount)
{
    /*
    fname CHAR(10),
    lanem CHAR(10),
    gender BYTE,
    ranking BYTE,
    department BYTE
    */
    uint * schema = new uint[5] {10,10,1,1,1};
    cRowHeapTable *rowHeapTable = new cRowHeapTable(schema, 5, rowCount);

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i<rowCount; i++){
        char * row = new char[rowHeapTable->getRowSize()];
        int fNameIndex = rand() % 20;
        int lNameIndex = rand() % 20;
        memcpy(row, "fname", 5);
        memcpy(row + 10, "lname", 5);
        row[20] = i % 2 ;
        row[21] = i % 3 ;
        row[22] = i % 5 ;
        rowHeapTable->Insert(row);
        delete []row;
    }    
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HeapRowTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    t1 = high_resolution_clock::now();
    int attr_len[] = {2,3,5};
    rowHeapTable->CreateBitmapIndex(attr_len, sizeof(attr_len)/sizeof(int), rowCount);
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("bitMapIndex creation done. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    int queryCnt = 1000;
    uint (*query)[3][2] = new uint [queryCnt][3][2];

    for(int i = 0; i < queryCnt; i++){
        query[i][0][0] = 2;
        query[i][0][1] = i % 2;
        query[i][1][0] = 3;
        query[i][1][1] = i % 3;
        query[i][2][0] = 4;
        query[i][2][1] = i % 5;
    }
    int * results1 = new int[queryCnt];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        results1[i] = rowHeapTable->SelectBitmap(query[i], 3);
    }
    //printf("\n");
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select Bitmap done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));


    int * results2 = new int[queryCnt];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        results2[i] = rowHeapTable->Select(query[i], 3);
    }
    //printf("\n");
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select rowHeapTable done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
/*
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        rowHeapTable->Select(query[i], 3);
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select rowHeapTable done. Time: %.6fs, Throughput: %.2f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
*/
    //print results1 and results2 in tuples (result1[i], result2[i]]), (...), ...
    /*
    printf("Results: ");
    for(int i = 0; i < queryCnt; i++){
        printf("(%d, %d), ", results1[i], results2[i]);
        if(i % 10 == 0 && i != 0)
            printf("\n");
    }
    */
   delete []query;
    delete []results1;
    delete []results2;
    delete rowHeapTable;
}

