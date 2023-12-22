#include <assert.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <string>
#include "cRowHeapTable.h"

int cRowHeapTable::getRowSize(){
    return mSize;
}
cRowHeapTable::cRowHeapTable(uint* atrs_size, uint len, uint rowCount)
{
    mAtrsSize = atrs_size;
    mAtrsLen = len;
    mSize = 0;
    for (uint i = 0; i < len; i++)
    {
        mSize += atrs_size[i];
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
    for (uint i = 0; i < mAtrsLen; i++)
    {
        memcpy(p + offset, row + offset, mAtrsSize[i]);
        offset += mAtrsSize[i];
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
    for (uint i = 0; i < mAtrsLen; i++)
    {
        memcpy(row + offset, p + offset, mAtrsSize[i]);
        offset += mAtrsSize[i];
    }

    return ret;
}
void cRowHeapTable::PrintRow(uint rowId) const
{
    char* row = new char[mSize];
    if (Get(rowId, row))
    {
        uint offset = 0;
        for (uint i = 0; i < mAtrsLen; i++)
        {
            if (mAtrsSize[i] > 1) {
                std::cout << "\"" << std::string(row + offset, mAtrsSize[i]) << "\"; ";
            } else {
                std::cout << static_cast<int>(*(row + offset)) << "; ";
            }
            offset += mAtrsSize[i];
        }
        std::cout << std::endl;
    }
    delete[] row;
}

bool cRowHeapTable::CreateBitmapIndex(int attr_len[], int attr_count, int rowCount)
{
    int *attr_idices = new int[attr_count];
    int processed = 0;
    int x = 0;
    while(processed < attr_count && x < mAtrsLen){
        if(mAtrsSize[x] == 1) {
            attr_idices[processed] = x;
            processed ++;
        }
        x++;
    }
    mIndex = new bitMapIndex(attr_len, attr_idices, attr_count, rowCount/*, this*/);

    for (int i = 0; i < rowCount; i++)
    {
        char *p = GetRowPointer(i);
        char row[attr_count] = { 0 };
        int inserted = 0;
        uint offset = 0;

        for(int j =0; j < mAtrsLen && inserted < attr_count; j++){
            if(mAtrsSize[j] == 1){
                memcpy(row + inserted, p + offset, 1);
                inserted ++;
            }
            offset += mAtrsSize[j];
        }
        if(!(mIndex->Insert(row))) return false;
    }
    return true;
}
int cRowHeapTable::Select(uint attrs[][2], uint attrsCount, bool print)
{
    if (!print){
        bool ret = false;
        uint count = 0;
        for (uint i = 0; i < mCount; i++)
        {
            bool found = true;
            char* p = GetRowPointer(i);

            for (uint j = 0; j < attrsCount; j++)
            {
                uint attrIndex = attrs[j][0];
                uint attrValue = attrs[j][1];

                uint offset = 0;
                for (uint k = 0; k < attrIndex; k++)
                {
                    offset += mAtrsSize[k];
                }

                if (memcmp(p + offset, &attrValue, mAtrsSize[attrIndex]) != 0)
                {
                    found = false;
                    break;
                }
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

            for (uint j = 0; j < attrsCount; j++)
            {
                uint attrIndex = attrs[j][0];
                uint attrValue = attrs[j][1];

                uint offset = 0;
                for (uint k = 0; k < attrIndex; k++)
                {
                    offset += mAtrsSize[k];
                }

                if (memcmp(p + offset, &attrValue, mAtrsSize[attrIndex]) != 0)
                {
                    found = false;
                    break;
                }
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
int cRowHeapTable::SelectMask(uint attrs[][2], uint attrsCount, bool print)
{
    if (!print){
        uint count = 0;
        char* search = new char[mSize];
        memset(search, 0, mSize);
        char* mask = new char[mSize];
        memset(mask, 0, mSize);
        //for each searched attribute, set bytes in search and mask
        //mask will have 1s in bytes where we need to search
        //search will be constructed as a row to compare with
        for(int i = 0; i < attrsCount; i++){
            uint attrIndex = attrs[i][0];
            uint attrValue = attrs[i][1];
            uint offset = 0;
            for (uint k = 0; k < attrIndex; k++)
            {
                offset += mAtrsSize[k];
            }
            for( int j = 0; j < mAtrsSize[attrIndex]; j++)
            {
                mask[offset + j] = 0xFF;
            }
            memcpy(search + offset, &attrValue, mAtrsSize[attrIndex]);

        }

        for (uint i = 0; i < mCount; i++)
        {
            bool found = true;
            char* p = GetRowPointer(i);
            for(int j = 0; j < mSize; j++){
                if((p[j] & mask[j]) != search[j]){
                    found = false;
                    break;
                }
            }
            if (found)
            {
                count++;
            }
        }
        return count;
    } 
    else{
        uint count = 0;
        char* search = new char[mSize];
        memset(search, 0, mSize);
        char* mask = new char[mSize];
        memset(mask, 0, mSize);
        int *resultRowIds = new int[mCount];

        //for each searched attribute, set bytes in search and mask
        //mask will have 1s in bytes where we need to search
        //search will be constructed as a row to compare with
        for(int i = 0; i < attrsCount; i++){
            uint attrIndex = attrs[i][0];
            uint attrValue = attrs[i][1];
            uint offset = 0;
            for (uint k = 0; k < attrIndex; k++)
            {
                offset += mAtrsSize[k];
            }
            for( int j = 0; j < mAtrsSize[attrIndex]; j++)
            {
                mask[offset + j] = 0xFF;
            }
            memcpy(search + offset, &attrValue, mAtrsSize[attrIndex]);

        }

        for (uint i = 0; i < mCount; i++)
        {
            bool found = true;
            char* p = GetRowPointer(i);
            for(int j = 0; j < mSize; j++){
                if((p[j] & mask[j]) != search[j]){
                    found = false;
                    break;
                }
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
int cRowHeapTable::SelectBitmap(uint attrs[][2], uint attrsCount, bool print)
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