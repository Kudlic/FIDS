#pragma once
#include "cSchema.h"

class bitMapIndex 
{
private:
    char* mData;
    int mBitSize;
    int mByteSize;

    int* attr_indices;
    cSchema* schema;

    int mCount;
    int mCapacity;

    //cRowHeapTable* rowHeapTable;
public:
    bitMapIndex(cSchema *schema, int attr_idices[], int rowCount/*, cRowHeapTable* rowHeapTable*/);
    ~bitMapIndex();
    int getBitmapBytes();
    bool Insert(char* row);
    int Select(int * attrs, uint attrsCount, int* resultRowIds = nullptr);
};
