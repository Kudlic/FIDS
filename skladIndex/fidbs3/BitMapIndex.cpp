#include "BitMapIndex.h"
#include<iostream>

using namespace std;

BitMapIndex::BitMapIndex(int attr_len[], int len, int rowCount)
{
	bitcount = len;
	index = new char[rowCount*bitcount];
	mRowsize = len;
	mCount = 0;
	mCapacity = rowCount*bitcount;
	mAttr_len = attr_len;
	memset(index, 0, rowCount*bitcount);
}

BitMapIndex::~BitMapIndex()
{
	delete index;
	index = nullptr;
	delete mAttr_len;
	mAttr_len = nullptr;
}

unsigned char BitMapIndex::GetByte(char* rec, int position, int* mAtrsSize)
{
	for (int i = 0; i < position; i++) {
		rec += mAtrsSize[i];
	}
	return *(rec)-'0';
}

void print(char* record)
{
	for (int i = 0; i < 8; i++) {
		unsigned char v = record[0] & 1 << i;
		if (v == 0) {
			cout << "0";
		}
		else {
			cout << "1";
		}
	}

	cout << " ";

	for (int i = 0; i < 8; i++) {
		unsigned char v = record[1] & 1 << i;
		if (v == 0) {
			cout << "0";
		}
		else {
			cout << "1";
		}
	}

	cout << " ";
	for (int i = 0; i < 8; i++) {
		unsigned char v = record[2] & 1 << i;
		if (v == 0) {
			cout << "0";
		}
		else {
			cout << "1";
		}
	}

	cout << " ";
	for (int i = 0; i < 8; i++) {
		unsigned char v = record[3] & 1 << i;
		if (v == 0) {
			cout << "0";
		}
		else {
			cout << "1";
		}
	}

	cout << " ";
	for (int i = 0; i < 8; i++) {
		unsigned char v = record[4] & 1 << i;
		if (v == 0) {
			cout << "0";
		}
		else {
			cout << "1";
		}
	}

	cout << " ";
	for (int i = 0; i < 8; i++) {
		unsigned char v = record[5] & 1 << i;
		if (v == 0) {
			cout << "0";
		}
		else {
			cout << "1";
		}
	}

	cout << endl;
}

void BitMapIndex::CreateRecord(char* rec, int* mAtrsSize, int len)
{
	int oldcount = mCount;
	int offset = 0;
	char* record = new char[mRowsize];
	memset(record, 0, mRowsize);
	for (int i = 0; i < len; i++) {
		if (mAttr_len[i] != -1) {
			unsigned char order = GetByte(rec, i, mAtrsSize);
			if (offset + mAttr_len[i] < CHAR_BIT) {
				Set1Bit(record, order+offset);
				offset += mAttr_len[i];
			}
			else if (offset + order < CHAR_BIT) {
				Set1Bit(record, order + offset);
				index[mCount] = record[0];
				mCount++;
				memset(record, 0, mRowsize);
				offset = offset + mAttr_len[i] - CHAR_BIT;
			}
			else {
				index[mCount] = record[0];
				mCount++;
				memset(record, 0, mRowsize);
				Set1Bit(record, order + offset-CHAR_BIT);
				offset = offset + mAttr_len[i] - CHAR_BIT;
			}
		}
	}
	if (record[0] != 0) {
		index[mCount] = record[0];
		mCount++;
		memset(record, 0, mRowsize);
	}
	
	mCount=oldcount+bitcount;
	//print(index);
	//int a = 3;
}

int BitMapIndex::Select(char* select)
{
	int result = 0;
	bool match;
	for (int i = 0; i < mCount; i += mRowsize) {
		match = true;
		//print(select);
		//print(index + i);
		for (int k = 0; k < mRowsize; k++) {
			//int a = (index[i+k] | index[i+k]);
			//int b = (index[i+k] | select[k]);
			if ((index[i + k] | index[i + k]) != (index[i + k] | select[k])) {
				match = false;
				break;
			}
		}
		if (match == true) {
			result++;
		}
	}
	return result;
}

void BitMapIndex::Set1Bit(char* record, int order)
{
	unsigned char v = 1 << (order % CHAR_BIT);
	record[order / CHAR_BIT] |= v;
}


