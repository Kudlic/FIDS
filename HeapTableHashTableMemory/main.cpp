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
void hashTableTest(const int rowCount, cMemory *memory);
void hashTableTestNoRecursion(const int rowCount, cMemory *memory);
void hashTableTestIterative(const int rowCount, cMemory *memory);



int main()
{
    const int RowCount = 10000000;
    
    heapTableTest(RowCount);
    printf("\n");

    hashTableTest(RowCount);
    printf("\n");
    

    cMemory *memory = new cMemory(1000000000);
    printf("HashTable test with cMemory.\n");
    hashTableTest(RowCount, memory);
    delete memory;
    printf("\n");

    memory = new cMemory(1000000000);
    printf("HashTable test with cMemory iterative.\n");
    hashTableTestIterative(RowCount, memory);
    delete memory;
    printf("\n");

    memory = new cMemory(1000000000);
    printf("HashTable test with cMemory and no recursion.\n");
    hashTableTestNoRecursion(RowCount, memory);
    delete memory;
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
	int keys[] = { 0, 100000, 1000000, 2500000, 5000000, 8000000, 10000001 };
	t1 = high_resolution_clock::now();

	for (int i = 0; i < findRowCount; i++) {
		bool ret = heapTable->Find(keys[i], data);
		if (ret != true || keys[i] != data) {
			printf("cHeapTable::Find, Key: %d is not found.\n", keys[i]);
		}
	}

	t2 = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(t2 - t1);
	printf("Table Find done, %dx, HeapTable. Time: %.6fs, Throughput: %.2f op/s.\n", findRowCount, time_span.count(), GetThroughput(findRowCount, time_span.count(), 1));

    delete heapTable;
}

void hashTableTest(const int rowCount)
{
    cHashTable<TKey, TData> *hashTable = new cHashTable<TKey, TData>(rowCount/2);

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

void hashTableTest(const int rowCount, cMemory *memory)
{
    cHashTable<TKey, TData> *hashTable = new cHashTable<TKey, TData>(rowCount/2, memory);

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
    //memory->PrintStat();

    delete hashTable;
}

void hashTableTestNoRecursion(const int rowCount, cMemory *memory)
{
    cHashTable<TKey, TData> *hashTable = new cHashTable<TKey, TData>(rowCount/2, memory);

    TKey key;
    TData data;

    // start insert hash table
    auto t1 = high_resolution_clock::now();

    for (int i = 0; i < rowCount; i++)
    {
        key = data = i;
        if (!hashTable->AddNoRecursion(key, data))
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
        bool ret = hashTable->FindNoRecursion(i, data);
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
    //memory->PrintStat();

    delete hashTable;
}

void hashTableTestIterative(const int rowCount, cMemory *memory)
{
    cHashTable<TKey, TData> *hashTable = new cHashTable<TKey, TData>(rowCount/2, memory);

    TKey key;
    TData data;

    // start insert hash table
    auto t1 = high_resolution_clock::now();

    for (int i = 0; i < rowCount; i++)
    {
        key = data = i;
        if (!hashTable->AddIterative(key, data))
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
        bool ret = hashTable->FindIterative(i, data);
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
    //memory->PrintStat();

    delete hashTable;
}