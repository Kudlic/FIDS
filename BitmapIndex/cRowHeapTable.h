#pragma once
#include "bitMapIndex.h"

class cRowHeapTable
{
private:
    uint* mAtrsSize;
    uint mAtrsLen;
    uint mSize;
    char* mData;
    uint mCapacity;
    uint mCount;
    bitMapIndex* mIndex;

public:
    int getRowSize();
    cRowHeapTable(uint* atrs_size, uint len, uint rowCount);
    ~cRowHeapTable();
    inline char* GetRowPointer(uint rowId) const;
    bool Insert(char* row);
    bool Get(uint rowId, char* row) const;
    void PrintRow(uint rowId) const;
    bool CreateBitmapIndex(int attr_len[], int attr_count, int rowCount);
    int Select(uint attrs[][2], uint attrsCount, bool print = false);
    int SelectMask(uint attrs[][2], uint attrsCount, bool print = false);
    int SelectBitmap(uint attrs[][2], uint attrsCount, bool print = false);

};
