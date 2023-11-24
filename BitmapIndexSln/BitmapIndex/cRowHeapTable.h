#pragma once

#include <assert.h>
#include <stdlib.h>
#include <cstring>
#include "bitMapIndex.h"

class cRowHeapTable
{
private:
    uint32_t* mAtrsSize;
    uint32_t mAtrsLen;
    uint32_t mSize;
    char* mData;
    uint32_t mCapacity;
    uint32_t mCount;
    bitMapIndex* mIndex;

public:
    cRowHeapTable(uint32_t* atrs_size, uint32_t len, uint32_t rowCount)
    {
        mAtrsSize = atrs_size;
        mAtrsLen = len;
        mSize = 0;
        for (uint32_t i = 0; i < len; i++)
        {
            mSize += atrs_size[i];
        }
        mCapacity = rowCount;
        mCount = 0;
        mData = new char[mSize * rowCount];
    }

    ~cRowHeapTable()
    {
        if (mData != NULL)
        {
            delete mData;
            mData = NULL;
            mCapacity = 0;
            mCount = 0;
            delete mIndex;
        }
    }

    inline char* GetRowPointer(uint32_t rowId) const
    {
        return mData + rowId * mSize;
    }

    bool Insert(const char* row)
    {
        if (mCount >= mCapacity) {
            return false;
        }

        char* p = GetRowPointer(mCount);

        
        uint32_t offset = 0;
        for (uint32_t i = 0; i < mAtrsLen; i++)
        {
            memcpy(p + offset, row + offset, mAtrsSize[i]);
            offset += mAtrsSize[i];
        }

        mCount++;
        return true;
    }

    bool Get(uint32_t rowId, char* row) const
    {
        bool ret = false;
        assert(rowId >= 0 && rowId < mCount);
        ret = true;

        char* p = GetRowPointer(rowId);

        uint32_t offset = 0;
        for (uint32_t i = 0; i < mAtrsLen; i++)
        {
            memcpy(row + offset, p + offset, mAtrsSize[i]);
            offset += mAtrsSize[i];
        }

        return ret;
    }
    void PrintRow(uint32_t rowId) const
    {
        char* row = new char[mSize];
        if (Get(rowId, row))
        {
            uint32_t offset = 0;
            for (uint32_t i = 0; i < mAtrsLen; i++)
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

    bool CreateBitmapIndex(int attr_len[], int attr_count, int rowCount)
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
            char *row = new char[attr_count];
            memset(row, 0, attr_count);
            int inserted = 0;
            uint32_t offset = 0;

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
    
/*
    bool Find(const TData* row) const
    {
        char* p = GetRowPointer(0);
        for (uint32_t i = 0; i < mCount; i++)
        {
            bool found = true;
            uint32_t offset = 0;
            for (uint32_t j = 0; j < mRowCount; j++)
            {
                if (memcmp(row + j, p + offset, mAtrsSize[j]) != 0)
                {
                    found = false;
                    break;
                }
                offset += mAtrsSize[j];
            }
            if (found)
            {
                return true;
            }
            p = p + mSize;
        }

        return false;
    }
*/
    int Select(uint32_t **attrs, uint32_t attrsCount, bool print = false)
    {
        if (!print){
            bool ret = false;
            uint32_t count = 0;
            for (uint32_t i = 0; i < mCount; i++)
            {
                bool found = true;
                char* p = GetRowPointer(i);

                for (uint32_t j = 0; j < attrsCount; j++)
                {
                    uint32_t attrIndex = attrs[j][0];
                    uint32_t attrValue = attrs[j][1];

                    uint32_t offset = 0;
                    for (uint32_t k = 0; k < attrIndex; k++)
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
            uint32_t count = 0;
            int *resultRowIds = new int[mCount];

            for (uint32_t i = 0; i < mCount; i++)
            {
                bool found = true;
                char* p = GetRowPointer(i);

                for (uint32_t j = 0; j < attrsCount; j++)
                {
                    uint32_t attrIndex = attrs[j][0];
                    uint32_t attrValue = attrs[j][1];

                    uint32_t offset = 0;
                    for (uint32_t k = 0; k < attrIndex; k++)
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
    int SelectMask(uint32_t **attrs, uint32_t attrsCount, bool print = false)
    {
        if (!print){
            uint32_t count = 0;
            char* search = new char[mSize];
            memset(search, 0, mSize);
            char* mask = new char[mSize];
            memset(mask, 0, mSize);
            //for each searched attribute, set bytes in search and mask
            //mask will have 1s in bytes where we need to search
            //search will be constructed as a row to compare with
            for(int i = 0; i < attrsCount; i++){
                uint32_t attrIndex = attrs[i][0];
                uint32_t attrValue = attrs[i][1];
                uint32_t offset = 0;
                for (uint32_t k = 0; k < attrIndex; k++)
                {
                    offset += mAtrsSize[k];
                }
                for( int j = 0; j < mAtrsSize[attrIndex]; j++)
                {
                    mask[offset + j] = 0xFF;
                }
                memcpy(search + offset, &attrValue, mAtrsSize[attrIndex]);

            }

            for (uint32_t i = 0; i < mCount; i++)
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
            uint32_t count = 0;
            char* search = new char[mSize];
            memset(search, 0, mSize);
            char* mask = new char[mSize];
            memset(mask, 0, mSize);
            int *resultRowIds = new int[mCount];

            //for each searched attribute, set bytes in search and mask
            //mask will have 1s in bytes where we need to search
            //search will be constructed as a row to compare with
            for(int i = 0; i < attrsCount; i++){
                uint32_t attrIndex = attrs[i][0];
                uint32_t attrValue = attrs[i][1];
                uint32_t offset = 0;
                for (uint32_t k = 0; k < attrIndex; k++)
                {
                    offset += mAtrsSize[k];
                }
                for( int j = 0; j < mAtrsSize[attrIndex]; j++)
                {
                    mask[offset + j] = 0xFF;
                }
                memcpy(search + offset, &attrValue, mAtrsSize[attrIndex]);

            }

            for (uint32_t i = 0; i < mCount; i++)
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
    int SelectBitmap(uint32_t **attrs, uint32_t attrsCount, bool print = false)
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
    

};
