#pragma once
class BitMapIndex
{
private:
	char* index;
	int* mAttr_len;
	int mCount;
	int mCapacity;
	int mRowsize;
	int BITCOUNT;
	int bitcount;
public:
	BitMapIndex(int attr_len[], int len, int rowCount);
	~BitMapIndex();
	unsigned char GetByte(char* rec, int position, int* mAtrsSize);
	void CreateRecord(char* rec, int* mAtrsSize, int len);
	void Set1Bit(char* record, int offset);
	int Select(char* select);
	// attr len -1, -1, 2, 6, 5
};

