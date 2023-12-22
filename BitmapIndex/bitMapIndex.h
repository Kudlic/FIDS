#pragma once
class cRowHeapTable;
class bitMapIndex 
{
private:
    char* mData;
    int mBitSize;
    int mByteSize;

    int* attr_len;
    int* attr_indices;

    int attr_count;
    int mCount;
    int mCapacity;

    //cRowHeapTable* rowHeapTable;
public:
    bitMapIndex(int attr_len[], int attr_idices[], int len, int rowCount/*, cRowHeapTable* rowHeapTable*/);
    ~bitMapIndex();
    bool Insert(const char* row);
    int Select(uint attrs[][2], uint attrsCount, int* resultRowIds = nullptr);
};

