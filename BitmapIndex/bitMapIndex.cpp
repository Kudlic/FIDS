#include<iostream>
#include<bitset>
#include <cstring>
#include "bitMapIndex.h"

bitMapIndex::bitMapIndex(int attr_len[], int attr_idices[], int len, int rowCount) {
    this->attr_len = attr_len;
    this->attr_indices = attr_idices;
    this->attr_count = len;
    for (int i = 0; i < len; i++) {
        mBitSize += attr_len[i];
    }
    mByteSize = (mBitSize + 7) / 8;
    mData = new char[mByteSize * rowCount];
    memset(mData, 0, mByteSize * rowCount);
    mCount = 0;
    mCapacity = rowCount;
    //this->rowHeapTable = rowHeapTable;
}
bitMapIndex::~bitMapIndex() {
    if (mData != NULL) {
        delete mData;
        mData = NULL;
        mCapacity = 0;
        mCount = 0;
    }
}

bool bitMapIndex::Insert(const char* row) {
    if (mCount >= mCapacity) {
        return false;
    }
    //startpos
    char* p = mData + mCount * mByteSize;

    uint byteIndex = 0;
    uint bitIndex = 0;
    uint offset = 0;

    for (int i = 0; i < attr_count; i++) {
        char attrValue = row[i];
        p[byteIndex + ((attrValue+bitIndex)/8)]  |= (1 << (bitIndex+attrValue) % 8);
        offset += attr_len[i];

        byteIndex = offset / 8;
        bitIndex = offset % 8;

    }
    mCount++;
    return true;
}

int bitMapIndex::Select(uint attrs[][2], uint attrsCount, int* resultRowIds) {
    char search[mByteSize] = { 0 };
    char mask[mByteSize] = { 0 };

    //Fill search similarly to Insert, fill mask with 1s for bits corresponding to searched attributes

    //iterate over every search attribute
    for (int i = 0; i < attrsCount; i++) {
        uint byteIndex = 0;
        uint bitIndex = 0;
        uint offset = 0;
        //get index in heapTable and searchValue
        char attrIndex = attrs[i][0];
        char attrValue = attrs[i][1];
        //match index with index in bitmap
        for(int j = 0; j < attr_count; j++)
        {
            int test = attr_indices[j];
            //if found, set bits in mask and search
            if(attr_indices[j] == attrIndex){
                search[byteIndex + ((attrValue+bitIndex)/8)]  |= (1 << (bitIndex+attrValue) % 8);

                //fill mask with 1s from offset to offset + attr_len[j]
                for(int k = 0; k < attr_len[j]; k++){
                    mask[byteIndex + ((k+bitIndex)/8)]  |= (1 << (bitIndex+k) % 8);
                }
                break;
            }
            offset += attr_len[j];
            byteIndex = offset / 8;
            bitIndex = offset % 8;
        }

    }
    //iterate over mData and try to match search with masked data
    int count = 0;
    if (resultRowIds != nullptr){
        for (int i = 0; i < mCount; i++) {
            char* p = mData + i * mByteSize;
            bool found = true;
            for (int j = 0; j < mByteSize; j++) {
                if ((p[j] & mask[j]) != search[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                resultRowIds[count] = i;                
                count++;
            }
        }
    }
    else{
        for (int i = 0; i < mCount; i++) {
            char* p = mData + i * mByteSize;
            bool found = true;
            for (int j = 0; j < mByteSize; j++) {
                if ((p[j] & mask[j]) != search[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                count++;
            }
        }
    }
    return count;
}


