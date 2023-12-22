#pragma once
#include "bitMapIndex.h"
#include "HashIndex.h"

typedef int TData;
typedef unsigned int uint;

class cRowHeapTable
{
private:
    cSchema* schema; //schema of the table, containting attribute number, sizes and cardinalities
    uint mSize; //size of a row
    char* mData; //pointer to data storage
    uint mCapacity; //max number of rows
    uint mCount; //current number of rows
    bitMapIndex* mIndex; //pointer to bitmap index
    HashIndex<TData>* hash; //pointer to hash index
	int* indexedAttr;
	int indexedAttrCount;

public:
    int getRowSize();
    int getBitmapBytes();
    int getHeapBytes();
    int getHashBytes();
    int getHashStatBytes();
    cSchema* getSchema();
    int countBitsForDecimal(int decimalNumber);
    cRowHeapTable(cSchema* schema, uint rowCount);
    ~cRowHeapTable();
    inline char* GetRowPointer(uint rowId) const;
    bool Insert(char* row);
    bool Get(uint rowId, char* row) const;
    void PrintRow(uint rowId) const;
    bool CreateBitmapIndex();
    int Select(int *attrs, uint attrsCount, bool print = false);
    int SelectBitmap(int * attrs, uint attrsCount, bool print = false);
    bool CreateHashIndex(int* attr_idices, int indices_count, int nodeSize = 2048);
    int SelectHash(int * attrs, uint attrsCount, bool print = false);
	int SelectHashStatistics(int* attrs, uint attrsCount);
};
