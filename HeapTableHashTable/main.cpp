#include <stdio.h>
#include <iostream>
#include <chrono>

#include "cHeapTable.h"
#include "cHashTable.h"

#define TKey int
#define TData int

using namespace std;
using namespace std::chrono;

float GetThroughput(int opsCount, float period, int unit = 10e6);
void heapTableTest(const int rowCount);
void hashTableTest(const int rowCount);

int main()
{
    const int RowCount = 10000000;
    heapTableTest(RowCount);
    printf("\n");

    hashTableTest(RowCount);
    printf("\n");

    return 0;
}

float GetThroughput(int opsCount, float period, int unit)
{
    return ((float)opsCount / unit) / period;
}

void heapTableTest(const int rowCount)
{
    cHeapTable<TKey, TData> *heapTable = new cHeapTable<TKey, TData>(rowCount);

    TKey key;
    TData data;

    // start insert heap table
    auto t1 = high_resolution_clock::now();

    for (int i = 0; i < rowCount; i++)
    {
        key = data = i;
        if (!heapTable->Add(key, data))
        {
            printf("Critical Error: Record %d insertion failed!\n", i);
        }
        if (i % 10000 == 0)
        {
            printf("#Record inserted: %d   \r", i);
        }
    }

    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HeapTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    // start scan heap table
    t1 = high_resolution_clock::now();

    for (int i = 0; i < rowCount; i++)
    {
        bool ret = heapTable->Get(i, key, data);
        if (!ret || key != i || data != i) {
            printf("Critical Error: Record %d not found!\n", i);
        }
        if (i % 10000 == 0)
        {
            printf("#Records retrieved: %d   \r", i);
        }
    }

    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Table scan done, HeapTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    // Start scanning heap table; Find is invoked 7 times
    const int findRowCount = 7;
    int findRow[findRowCount] = { 0, 1000000, 2000000, 3000000, 4000000, 5000000, 9999999 };
    t1 = high_resolution_clock::now();

    for (int i = 0; i < findRowCount; i++)
    {
        bool ret = heapTable->Get(findRow[i], key, data);
        if (!ret || key != findRow[i] || data != findRow[i]) {
            printf("Critical Error: Record %d not found!\n", findRow[i]);
        }
        if (i % 10000 == 0)
        {
            printf("#Records retrieved: %d   \r", i);
        }
    }

    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Table Find done, %dx, HeapTable. Time: %.8fs, Throughput: %.2f op/s.\n",
        findRowCount, time_span.count(), GetThroughput(findRowCount, time_span.count(), 1));

    delete heapTable;
}

void hashTableTest(const int rowCount)
{
    cHashTable<TKey, TData> *hashTable = new cHashTable<TKey, TData>(rowCount);

    TKey key;
    TData data;

    // start insert hash table
    auto t1 = high_resolution_clock::now();

    for (int i = 0; i < rowCount; i++)
    {
        key = data = i;
        if (!hashTable->Add(key, data))
        {
            printf("Critical Error: Record %d insertion failed!\n", i);
        }
        if (i % 10000 == 0)
        {
            printf("#Record inserted: %d   \r", i);
        }
    }

    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HashTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    // start scan hash table
    t1 = high_resolution_clock::now();

    for (int i = 0; i < rowCount; i++)
    {
        bool ret = hashTable->Find(i, data);
        if (!ret || data != i) {
            printf("Critical Error: Record %d not found!\n", i);
        }
        if (i % 10000 == 0)
        {
            printf("#Records retrieved: %d   \r", i);
        }
    }

    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Table scan done, HashTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    hashTable->PrintStat();

    delete hashTable;
}