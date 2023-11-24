#pragma once

#include <assert.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include "bitMapIndex.h"
#include "HashIndex.h"

typedef int TData;

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
    int getRowSize(){
        return mSize;
    }
    int getBitmapBytes(){
        return mIndex->getBitmapBytes();
    }
    int getHeapBytes(){
        return mSize * mCount;
    }
    int getHashBytes(){
        return hash->getHashBytes();
    }
    int getHashStatBytes(){
        return hash->getHashBytesS();
    }
    cSchema* getSchema(){
        return schema;
    }
    int countBitsForDecimal(int decimalNumber) {
        if (decimalNumber <= 0) {
            return 1; // 0 requires 1 bit to represent.
        }

        int bits = static_cast<int>(std::ceil(std::log2(decimalNumber + 1)));
        return bits;
    }
    cRowHeapTable(cSchema* schema, uint rowCount)
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
    ~cRowHeapTable()
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
    inline char* GetRowPointer(uint rowId) const
    {
        return mData + rowId * mSize;
    }
    bool Insert(char* row)
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
    bool Get(uint rowId, char* row) const
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
    void PrintRow(uint rowId) const
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
    bool CreateBitmapIndex()
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
    int Select(int *attrs, uint attrsCount, bool print = false)
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
    int SelectBitmap(int * attrs, uint attrsCount, bool print = false)
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
    
    //input is ordered array of attribute indices that are to be indexed
    bool CreateHashIndex(int* attr_idices, int indices_count, int nodeSize = 2048){
        this->indexedAttr = attr_idices;
        this->indexedAttrCount = indices_count;
        int processed = 0;
        int x = 0;
        int mKeySizeBit = 0;

        //Calculation on key size
        /*
        while(processed < indices_count && x < schema->GetAttrCount()){
            if(schema->GetAttrValueCount()[x] != -1 && attr_idices[processed] == x) {
                mKeySizeBit += countBitsForDecimal(schema->GetAttrValueCount()[x]);
                processed ++;
            }
            x++;
        }
        int mKeySize = mKeySizeBit / 8;
        if(mKeySizeBit % 8 != 0) mKeySize++;
        */
        int mKeySize = 0;
        while(processed < indices_count && x < schema->GetAttrCount()){
            if( attr_idices[processed] == x) {
                mKeySize+= schema->GetAttrSize()[x];
                processed ++;
            }
            x++;
        }

	    hash = new HashIndex<TData>(schema, nodeSize, mKeySize, mCount);

        for (int i = 0; i < mCount; i++)
        {
            char *p = GetRowPointer(i);
            int attrProcessed = 0;

            char * key = new char[mKeySize];
            int offset = 0;
            int keyOffset = 0;
            for (int j = 0; j < schema->GetAttrCount(); j++)
            {
                if(schema->GetAttrValueCount()[j] != -1 && attr_idices[attrProcessed] == j) {
                    memcpy(key + keyOffset, p + offset, schema->GetAttrSize()[j]);
                    keyOffset += schema->GetAttrSize()[j];
                    attrProcessed++;
                }
                
                offset += schema->GetAttrSize()[j];
            }
            //printf("Inserting key: %d, %d, %d,  HV: %d\n", key[0], key[1], key[2], hash->HashValue(key));
            
            if(!(hash->Insert(key, i))) return false;
            delete[] key;
        }
        return true;
    }
    int SelectHash(int * attrs, uint attrsCount, bool print = false){
        bool allIndexed = true;
        int idxAttrOffset = 0;
        int hashQueryOffset = 0;
        char *hashQuery = new char[indexedAttrCount];
        for(int i = 0; i < attrsCount; i++){
            if(attrs[i] == -1 && indexedAttr[idxAttrOffset] != i){
                continue;
            }
            else if(attrs[i] != -1 && indexedAttr[idxAttrOffset] == i){
                // Copy only the first byte of the integer to hashQuery
                memcpy(hashQuery + hashQueryOffset, (char*)&attrs[i], 1);
                idxAttrOffset++;
                hashQueryOffset += schema->GetAttrSize()[i];
            }
            else {
                allIndexed = false;
                break;
            }
        }
        if(!allIndexed){
            return SelectBitmap(attrs, attrsCount, print);
        }
        else{
            int * resultRowIds = new int[mCount];
            int result = hash->Select(resultRowIds, hashQuery);
            delete []resultRowIds;
            return result; 
        }
    }

	int SelectHashStatistics(int* attrs, uint attrsCount){
        //First check, whether query contains all and only indexed attributes
        bool allIndexed = true;
        int idxAttrOffset = 0;
        int hashQueryOffset = 0;
        char *hashQuery = new char[indexedAttrCount];
        for(int i = 0; i < attrsCount; i++){
            int attr1 = attrs[i];
            int attr2 = indexedAttr[idxAttrOffset];
            if(attrs[i] == -1 && indexedAttr[idxAttrOffset] != i){
                continue;
            }
            else if(attrs[i] != -1 && indexedAttr[idxAttrOffset] == i){
                // Copy only the first byte of the integer to hashQuery
                memcpy(hashQuery + hashQueryOffset, (char*)&attrs[i], 1);
                idxAttrOffset++;
                hashQueryOffset += schema->GetAttrSize()[i];
            }
            else {
                allIndexed = false;
                break;
            }
        }
        if(!allIndexed){
            return SelectBitmap(attrs, attrsCount);
        }
        else{
            /*
            printf("Hash query: ");
            for(int i = 0; i < hashQueryOffset; i++){
                printf("%d ", hashQuery[i]);
            }
            printf("\n");
            */
            
            int * resultRowIds = new int[mCount];
            return hash->SelectStatistics(hashQuery);
        }
    }

};
