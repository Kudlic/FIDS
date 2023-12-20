#pragma once

#include <assert.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <string>
#include "bitMapIndex.h"
#include "cRowHeapTable.h"

int cRowHeapTable::getRowSize()
{
    return mSize;
}

int cRowHeapTable::getBitmapBytes()
{
    return mIndex->getBitmapBytes();
}

int cRowHeapTable::getHeapBytes()
{
    return mSize * mCount;
}

cSchema* cRowHeapTable::getSchema()
{
    return schema;
}

cRowHeapTable::cRowHeapTable(cSchema* schema, uint rowCount)
{
    this->schema = schema;
    mSize = 0;
    for (uint i = 0; i < schema->GetAttrCount(); i++)
    {
        mSize += schema->GetAttrSize()[i];
    }
    mCapacity = rowCount;
    mCount = 0;
    mData = new char[mSize * rowCount];
}

cRowHeapTable::~cRowHeapTable()
{
    if (mData != NULL)
    {
        delete []mData;
        mData = NULL;
        mCapacity = 0;
        mCount = 0;
        delete mIndex;
    }
}

inline char* cRowHeapTable::GetRowPointer(uint rowId) const
{
    return mData + rowId * mSize;
}

bool cRowHeapTable::Insert(char* row)
{
    if (mCount >= mCapacity) {
        return false;
    }

    char* p = GetRowPointer(mCount);

    
    uint offset = 0;
    for (uint i = 0; i < schema->GetAttrCount(); i++)
    {
        memcpy(p + offset, row + offset, schema->GetAttrSize()[i]);
        offset += schema->GetAttrSize()[i];
    }

    mCount++;
    return true;
}

bool cRowHeapTable::Get(uint rowId, char* row) const
{
    bool ret = false;
    assert(rowId >= 0 && rowId < mCount);
    ret = true;

    char* p = GetRowPointer(rowId);

    uint offset = 0;
    for (uint i = 0; i < schema->GetAttrCount(); i++)
    {
        memcpy(row + offset, p + offset, schema->GetAttrSize()[i]);
        offset += schema->GetAttrSize()[i];
    }

    return ret;
}

void cRowHeapTable::PrintRow(uint rowId) const
{
    char* row = new char[mSize];
    if (Get(rowId, row))
    {
        uint offset = 0;
        for (uint i = 0; i < schema->GetAttrCount(); i++)
        {
            if (schema->GetAttrSize()[i] > 1) {
                std::cout << "\"" << std::string(row + offset, schema->GetAttrSize()[i]) << "\"; ";
            } else {
                std::cout << static_cast<int>(*(row + offset)) << "; ";
            }
            offset += schema->GetAttrSize()[i];
        }
        std::cout << std::endl;
    }
    delete[] row;
}

bool cRowHeapTable::CreateBitmapIndex()
{
    int *attr_idices = new int[schema->GetAttrCount()];
    int processed = 0;
    int x = 0;
    while(processed < schema->GetAttrCount()){
        if(schema->GetAttrValueCount()[x] != -1) {
            attr_idices[processed] = x;
            processed ++;
        }
        x++;
    }
    mIndex = new bitMapIndex(schema, attr_idices, mCapacity/*, this*/);

    for (int i = 0; i < mCount; i++)
    {
        char *p = GetRowPointer(i);
        int inserted = 0;
        if(!(mIndex->Insert(p))) return false;
    }
    return true;
}

int cRowHeapTable::Select(int *attrs, uint attrsCount, bool print)
{
    if (!print){
        bool ret = false;
        uint count = 0;
        for (uint i = 0; i < mCount; i++)
        {
            bool found = true;
            char* p = GetRowPointer(i);
            uint offset = 0;
            for (uint j = 0; j < attrsCount; j++)
            {
                if (attrs[j]!=-1 && memcmp(p + offset, attrs+j, schema->GetAttrSize()[j]) != 0)
                {
                    found = false;
                    break;
                }
                offset += schema->GetAttrSize()[j];
            }
            if (found)
            {
                count++;
            }
        }
        return count;
    } 
    else {
        bool ret = false;
        uint count = 0;
        int *resultRowIds = new int[mCount];

        for (uint i = 0; i < mCount; i++)
        {
            bool found = true;
            char* p = GetRowPointer(i);
            uint offset = 0;
            for (uint j = 0; j < attrsCount; j++)
            {
                if (attrs[j]!=-1 && memcmp(p + offset, attrs+j, schema->GetAttrSize()[j]) != 0)
                {
                    found = false;
                    break;
                }
                offset += schema->GetAttrSize()[j];
            }

            if (found)
            {
                resultRowIds[count] = i;
                count++;
            }
        }      
        printf("Query results (%d): \n", count);
        for(int i = 0; i < count; i++){
            PrintRow(resultRowIds[i]);
        }
        
        delete[] resultRowIds;    
        return count;
    }
}

int cRowHeapTable::SelectBitmap(int * attrs, uint attrsCount, bool print)
{
    if (print){
        int *resultRowIds = new int[mCount];
        int count = mIndex->Select(attrs, attrsCount, resultRowIds);
        printf("Query results (%d): \n", count);
        for(int i = 0; i < count; i++){
            PrintRow(resultRowIds[i]);
        }
        delete[] resultRowIds;
        return count;
    }
    else{
        return mIndex->Select(attrs, attrsCount);
    }
}
