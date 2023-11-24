#pragma once
#include "HashIndexNode.h"
#include <iostream>
template<class TData>
class HashIndex
{
private:
	int mRowCount;
	int  mNodeSize;
	int mKeySize;
	int mItemCount;
	int mNodeCount;
	int mItemCountS;
	int mNodeCountS;
	int* mAttrLen;
	HashIndexNode<TData>** index;
	HashIndexNode<TData>** statistics;
public:
	HashIndex(int rowCount, int nodeSize, int keySize, int* attr_len);
	~HashIndex();
	void Add(char*& key, const TData& data);
	int Select(char*& key);
	int HashValue(char* key);
};

template<class TData>
HashIndex<TData>::HashIndex(int rowCount, int nodeSize, int keySize, int* attr_len) {
	mRowCount = ceil((double)rowCount / nodeSize);
	mNodeSize = nodeSize;
	mKeySize = keySize;
	mAttrLen = attr_len;
	index = new HashIndexNode<TData>*[mRowCount];
	statistics = new HashIndexNode<TData>*[mRowCount];
	mItemCount = 0;
	mItemCountS = 0;
	mNodeCount = 0;
	mNodeCountS = 0;
	for (int i = 0; i < mRowCount; i++)
	{
		index[i] = nullptr;
		statistics[i] = nullptr;
	}
}

template<class TData>
HashIndex<TData>::~HashIndex() {

}

template<class TData>
void HashIndex<TData>::Add(char*& key, const TData& data) {
	int hv = HashValue(key);
	if (index[hv] == nullptr)
	{
		index[hv] = new HashIndexNode<TData>();
		mNodeCount++;
	}
	if (statistics[hv] == nullptr)
	{
		statistics[hv] = new HashIndexNode<TData>();
		mNodeCountS++;
	}

	bool a = index[hv]->Add(key, data, mItemCount, mNodeCount);
	bool b = statistics[hv]->AddStatistic(key, mItemCountS, mNodeCountS, mKeySize);
}

template<class TData>
int HashIndex<TData>::Select(char*& key) {
	int hv = HashValue(key);
	char a = key[0];
	char b = key[1];
	char c = key[2];
	int d = index[hv]->Select(key, mKeySize);
	return d;
}



template<class TData>
int HashIndex<TData>::HashValue(char* key)
{
	int val = 0;
	for (int i = 0; i < mKeySize; i++) {
		val |= key[i] << mAttrLen[i];
	}
	return val;
}

