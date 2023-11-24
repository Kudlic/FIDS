#include "cRowHeapTable.h"
#include "BitMapIndex.h"
#include <corecrt_memory.h>
#include <xmemory>
#include<iostream>
#include<tuple>
#include "HashIndex.h"

#define TData int

using namespace std;

cRowHeapTable::cRowHeapTable(int atrs_size[], int len, int rowCount, int *attr_len, int atrCount, int recordBytes)
{
	mData = new char[rowCount*len];
	mRowSize = len;
	mCount = 0;
	mCapacity = rowCount;
	mAtrsSize = atrs_size;
	mAtrCount = atrCount;
	mRecordBytes = recordBytes;
	mAttrLen = attr_len;
	bitMapIndex = new BitMapIndex(attr_len, mRecordBytes, rowCount);
	char* mBitmapRecord = new char[mRecordBytes];

	int nodeSize = 0;
	mKeySize = 0;

	for (int i = 0; i < atrCount; i++) {
		if (attr_len[i] != -1) {
			mKeySize++;
			nodeSize += attr_len[i];
		}
	}

	int* hashKeyLen = new int[mKeySize];
	int pom = 0;
	int pom2 = 0;
	for (int i = 0; i < atrCount; i++) {
		if (attr_len[i] != -1) {
			hashKeyLen[pom] = ceil(log2(attr_len[i - 1])) + pom2;
			pom2 = hashKeyLen[pom];
			pom++;
		}
	}

	hash = new HashIndex<TData>(mCapacity, nodeSize, mKeySize, hashKeyLen);
}

cRowHeapTable::~cRowHeapTable() {
	delete mData;
	mData = nullptr;
	delete mAtrsSize;
	mAtrsSize = nullptr;
}

bool cRowHeapTable::Insert(char* rec)
{
	if (mCount < mCapacity) {
		unsigned int offset = mCount * mRowSize;
		char* p = mData + offset;
		memcpy(p, rec, mRowSize);	
		mCount++;
		return true;
	}
	return false;
}

int cRowHeapTable::SelectWithoutTuple(int* select, int len)
{
	int result = 0;
	bool pom = true;
	for (int i = 0; i < mCount; i++) {
		for (int k = 0; k < len; k++) {
			int a = select[k];
			if (select[k] != -1) {
				if (Match(k, select[k], i) == false) {
					pom = false;
					break;
				}
			}
		}
		if (pom == true) {
			result++;
		}
		else {
			pom = true;
		}
	}
	return result;
}

int cRowHeapTable::SelectIndexWithoutTuple(int* select, int len)
{
	char* record = new char[mRecordBytes];
	IndexRecordWithoutTuple(record, select, len);
	return bitMapIndex->Select(record);
}

void cRowHeapTable::IndexRecordWithoutTuple(char* record, int* select, int len)
{
	int count = 0;
	char* pom = new char[1];
	memset(pom, 0, 1);
	memset(record, 0, mRecordBytes);
	int offset = 0;
	for (int i = 0; i < mAtrCount; i++) {
		if (mAttrLen[i] != -1) {
			if (select[i] == -1) {
				offset += mAttrLen[i];
				if (offset >= CHAR_BIT) {
					count++;
					memset(pom, 0, 1);
					offset -= CHAR_BIT;
				}
			}
			else {
				unsigned char order = select[i];
				if (offset + mAttrLen[i] < CHAR_BIT) {
					Set1Bit(pom, order + offset);
					offset += mAttrLen[i];
				}
				else if (offset + order < CHAR_BIT) {
					Set1Bit(pom, order + offset);
					record[count] = pom[0];
					count++;
					memset(pom, 0, 1);
					offset = offset + mAttrLen[i] - CHAR_BIT;
				}
				else {
					record[count] = pom[0];
					count++;
					memset(pom, 0, 1);
					Set1Bit(pom, order + offset - CHAR_BIT);
					offset = offset + mAttrLen[i] - CHAR_BIT;
				}
			}			
			if (pom[0] != 0) {
				record[count] = pom[0];
			}
		}
	}
}


void cRowHeapTable::Set1Bit(char* record, int order)
{
	unsigned char v = 1 << (order % CHAR_BIT);
	record[order / CHAR_BIT] |= v;
}

void cRowHeapTable::IndexRecord(char* record, tuple<unsigned int, unsigned int>* select, int len)
{
	int count = 0;
	char* pom = new char[1];
	memset(pom, 0, 1);
	memset(record, 0, mRecordBytes);
	int offset = 0;
	for (int i = 0; i < mAtrCount; i++) {
		if (mAttrLen[i] != -1) {
			for (int k = 0; k < len; k++) {
				if (get<0>(select[k]) == i) {
					unsigned char order = get<1>(select[k]);
					int test = mAttrLen[i];
					if (offset + mAttrLen[i] < CHAR_BIT) {
						Set1Bit(pom, order + offset);
						offset += mAttrLen[i];
					}
					else if (offset + order < CHAR_BIT) {
						Set1Bit(pom, order + offset);
						record[count] = pom[0];
						count++;
						memset(pom, 0, 1);
						offset = offset + mAttrLen[i] - CHAR_BIT;
					} else {
						record[count] = pom[0];
						count++;
						memset(pom, 0, 1);
						offset = offset + mAttrLen[i] - CHAR_BIT;
					}
					
				}
			}
			if (pom[0] != 0) {
				record[count] = pom[0];
			}
			
		}

	}
}

bool cRowHeapTable::Match(unsigned int column, unsigned int value, unsigned int position) {
	int offset = 0;
	for (int i = 0; i < column; i++) {
		offset += mAtrsSize[i];
	}
	if (mData[offset+mRowSize*position] - '0' == (char)value) {
		return true;
	}
	return false;
}

void cRowHeapTable::CreateBitmapIndex(char* rec)
{
	bitMapIndex->CreateRecord(rec, mAtrsSize, mAtrCount);
}

void cRowHeapTable::CreateHashIndex(char* rec, const TData& tData)
{
	
	char* key = new char[mKeySize];
	int k = 0;
	int offset = 0;
	for (int i = 0; i < mAtrCount; i++) {
		if (mAttrLen[i] != -1) {
			int a = rec[offset]-'0';
			key[k] = (char)a;
			k++;
		}
		offset += mAtrsSize[i];
	}
	hash->Add(key, tData);
}

int cRowHeapTable::SelectHashIndex(char* rec)
{

	char* key = new char[mKeySize];
	int k = 0;
	int offset = 0;
	for (int i = 0; i < mAtrCount; i++) {
		if (mAttrLen[i] != -1) {
			int a = rec[i] - '0';
			key[k] = (char)a;
			k++;
		}
		offset += mAtrsSize[i];
	}
	return hash->Select(key);
}

void cRowHeapTable::GenerateRecord(char* rec, int* attr_size, int* attr_max_value, int attr_count)
{
	int offset = 0;
	for (int i = 0; i < attr_count; i++) {

			for (int k = 0; k < attr_size[i]; k++) {
				if (attr_max_value[i] == -1) {
					rec[offset] = (char)rand() % 255;
				}
				else {
					int max = attr_max_value[i];
					int r = rand() % attr_max_value[i];
					rec[offset] = (char)r;
					char v = rec[offset];
					int a = 1;
				}
				offset++;
			}
		}
}

void cRowHeapTable::CreateRecord(char* rec, int* attr_size, int* attr_max_value, int attr_count, char*val)
{
	int offset = 0;
	int offset2 = 0;
	for (int i = 0; i < attr_count; i++) {

		for (int k = 0; k < attr_size[i]; k++) {
			if (val[offset2] == ';') {
				offset2++;
			}
			rec[offset] = val[offset2];
			
			offset++;
			offset2++;
		}
	}
}

void cRowHeapTable::CreateHashRecord(char* rec, int* attr_size, int* attr_max_value, int attr_count, char* val)
{
	int offset = 0;
	int offset2 = 0;
	for (int i = 0; i < attr_count; i++) {

		for (int k = 0; k < attr_size[i]; k++) {
			char a = val[offset2];
			if (val[offset2] == ';') {
				offset2++;
			}

			if (val[offset2] == '-') {
				rec[offset] = '-1';
				char b = rec[offset];
				offset++;
				offset2 += 3;
			}
			else {
				rec[offset] = val[offset2];
				offset++;
				offset2++;
			}
			
		}
	}
}
