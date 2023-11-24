#pragma once
#include <stdio.h>

class cMemory
{
private:
    int mCapacity;
    int mSize;
    char* mData;

public:
    cMemory(int capacity);
    ~cMemory();
    inline char* New(int size);
    void PrintStat() const;
};

char* cMemory::New(int size)
{
    char* mem = NULL;
    if (mSize + size >= mCapacity)
    {
        printf("Critical Error: cMemory::New(): There is no memory.\n");
        mem = NULL;
    }
    else {
        mem = mData + mSize;
        mSize += size;
    }
    return mem;
}
cMemory::cMemory(int capacity) {
    mData = new char[capacity];
    mCapacity = capacity;
    mSize = 0;
}

cMemory::~cMemory() {
    if (mData != NULL) {
        delete[] mData;
        mCapacity = 0;
        mSize = 0;
    }
}

void cMemory::PrintStat() const {
    printf("cMemory::PrintStat(): Capacity: %d, Size: %d, Utilization: %.2f.\n",
           mCapacity, mSize, (float)mSize / mCapacity);
}


