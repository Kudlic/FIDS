#pragma once
#include<iostream>
#include<bitset>
#include "cSchema.h"
//#include "cRowHeapTable.h"

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
    bitMapIndex(cSchema *schema, int attr_idices[], int rowCount/*, cRowHeapTable* rowHeapTable*/) {
        this->attr_indices = attr_idices;
        this->schema = schema;
        for (int i = 0; i < schema->GetAttrCount(); i++) {
            if(schema->GetAttrValueCount()[i] == -1){continue;}
            mBitSize += schema->GetAttrValueCount()[i];
        }
        mByteSize = (mBitSize + 7) / 8;
        mData = new char[mByteSize * rowCount];
        memset(mData, 0, mByteSize * rowCount);
        mCount = 0;
        mCapacity = rowCount;
        //this->rowHeapTable = rowHeapTable;
    }
    ~bitMapIndex() {
        if (mData != NULL) {
            delete mData;
            mData = NULL;
            mCapacity = 0;
            mCount = 0;
        }
    }
    int getBitmapBytes(){
        return mByteSize * mCapacity;
    }
    bool Insert(char* row) {
        if (mCount >= mCapacity) {
            return false;
        }
        //startpos
        char* p = mData + mCount * mByteSize;

        uint byteIndex = 0;
        uint bitIndex = 0;
        uint offset = 0;
        uint rowOffset = 0;

        for (int i = 0; i < schema->GetAttrCount(); i++) {
            if(schema->GetAttrValueCount()[i] == -1){
                rowOffset += schema->GetAttrSize()[i];
                continue;
            }
            char attrValue = row[rowOffset];
            p[byteIndex + ((attrValue+bitIndex)/8)]  |= (1 << (bitIndex+attrValue) % 8);
            offset += schema->GetAttrValueCount()[i];
            rowOffset += schema->GetAttrSize()[i];
            byteIndex = offset / 8;
            bitIndex = offset % 8;

        }
        //print mByteSize bytes on p in binary
        /*
            for (int j = 0; j < mByteSize; j++) {
                std::cout << std::bitset<8>(p[j]) << " ";
            }
            std::cout << std::endl;
        */

        mCount++;
        return true;
    }

    int Select(int * attrs, uint attrsCount, int* resultRowIds = nullptr) {
        char search[mByteSize] = { 0 };
        char mask[mByteSize] = { 0 };

        uint byteIndex = 0;
        uint bitIndex = 0;
        uint offset = 0;

        //Fill search similarly to Insert, fill mask with 1s for bits corresponding to searched attributes
        for (int i = 0; i < attrsCount; i++) {
            //if attribute is not searchable
            if(attrs[i] == -1 ){
                //if attribute is indexed but not queried
                if(schema->GetAttrSize()[i] == 1){
                    offset += schema->GetAttrValueCount()[i];
                    byteIndex = offset / 8;
                    bitIndex = offset % 8;
                }
                //else attribute is not indexed
                continue;
            }
            search[byteIndex + ((attrs[i]+bitIndex)/8)]  |= (1 << (bitIndex+attrs[i]) % 8);

            //fill mask with 1s from offset to offset + attr_len[j]
            for(int k = 0; k < schema->GetAttrValueCount()[i]; k++){
                mask[byteIndex + ((k+bitIndex)/8)]  |= (1 << (bitIndex+k) % 8);
            }                

            offset += schema->GetAttrValueCount()[i];
            byteIndex = offset / 8;
            bitIndex = offset % 8;
        }
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
};

