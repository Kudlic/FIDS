#pragma once

#include "bitMapIndex.h"

class cRowHeapTable
{
private:
    cSchema* schema;
    uint mSize;
    char* mData;
    uint mCapacity;
    uint mCount;
    bitMapIndex* mIndex;

public:
    int getRowSize();
    int getBitmapBytes();
    int getHeapBytes();
    cSchema* getSchema();
    cRowHeapTable(cSchema* schema, uint rowCount);
    ~cRowHeapTable();
    inline char* GetRowPointer(uint rowId) const;
    bool Insert(char* row);
    bool Get(uint rowId, char* row) const;
    void PrintRow(uint rowId) const;
    bool CreateBitmapIndex();
    int Select(int *attrs, uint attrsCount, bool print = false);
    int SelectBitmap(int * attrs, uint attrsCount, bool print = false);
};
