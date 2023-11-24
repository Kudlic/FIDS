
#pragma once
#include<iostream>
#include <tuple>
#include "BitMapIndex.h"
#include "HashIndex.h"

typedef int TData;

using namespace std;
class cRowHeapTable
{
private:
	char* mData;
	int mCapacity;
	int mRowSize;
	int mCount;
	int* mAtrsSize;
	int* mAttrLen;
	int mAtrCount;
	int mRecordBytes;
	int mKeySize;
	char* mBitmapRecord;
	BitMapIndex *bitMapIndex;
	HashIndex<TData>* hash;

public:
	cRowHeapTable(int atrs_size[], int len, int rowCount, int * attr_len, int atrCount, int recordBytes);
	~cRowHeapTable();

	bool Insert(char * rec);
	int SelectWithoutTuple(int* select, int len);
	int SelectIndexWithoutTuple(int* select, int len);
	void IndexRecordWithoutTuple(char* record, int* select, int len);
	void Set1Bit(char* record, int offset);
	void IndexRecord(char* record, tuple<unsigned int, unsigned int>* select, int len);
	static void GenerateRecord(char *rec, int * attr_size, int * attr_max_value, int attr_count);
	static void CreateRecord(char* rec, int* attr_size, int* attr_max_value, int attr_count, char*val);
	static void CreateHashRecord(char* rec, int* attr_size, int* attr_max_value, int attr_count, char* val);
	bool Match(unsigned int column, unsigned int value, unsigned int position);
	void CreateBitmapIndex(char *rec);
	void CreateHashIndex(char* rec, const TData& tData);
	int SelectHashIndex(char* rec);
};

