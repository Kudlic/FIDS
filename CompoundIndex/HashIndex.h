#pragma once
#include "HashIndexNode.h"
#include "cSchema.h"
#include <iostream>
template<class TData>
class HashIndex
{
private:
	int mRowCount;
	int mNodeSize;
	int mKeySize;
	int mDataSize;
	int mNodeDataSize;

	int mItemCount;
	int mNodeCount;
	int mItemCountS;
	int mNodeCountS;

	cSchema *mSchema;

	HashIndexNode<TData>** index;
	HashIndexNode<TData>** statistics;
public:
	HashIndex(cSchema *schema, int nodeSize, int keySize, int rowCount);
	~HashIndex();
	bool Insert(char*& key, const TData& data);
	int Select(int* ids, char*& key);
	int SelectStatistics(char*& key);
	int HashValue(char* key);
	int getHashBytes(){
		return mNodeDataSize * mNodeCount;
	}
	int getHashBytesS(){
		return mNodeDataSize * mNodeCountS;
	}
};

template<class TData>
//schema, ByteSize of one node data, ByteSize of key, rowCount
HashIndex<TData>::HashIndex(cSchema *schema, int nodeSize, int keySize, int rowCount) {
	mRowCount = rowCount;
	mNodeSize = nodeSize;
	mKeySize = keySize;
	mDataSize = sizeof(TData);
	mNodeDataSize = (mDataSize + mKeySize) * mNodeSize;
	schema = mSchema;
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
	for (int i = 0; i < mRowCount; i++) {
		if (index[i] != nullptr) {
			delete index[i];
		}
	}
	for (int i = 0; i < mRowCount; i++) {
		if (statistics[i] != nullptr) {
			delete statistics[i];
		}
	}
	delete index;
	delete statistics;
}

template<class TData>
bool HashIndex<TData>::Insert(char*& key, const TData& data) {
	int hv = HashValue(key);
	if (index[hv] == nullptr)
	{
		index[hv] = new HashIndexNode<TData>(mKeySize, mDataSize, mNodeDataSize);
		mNodeCount++;
	}
	if (statistics[hv] == nullptr)
	{
		statistics[hv] = new HashIndexNode<TData>(mKeySize, mDataSize, mNodeDataSize);
		mNodeCountS++;
	}

	bool d = index[hv]->Add(key, data, mItemCount, mNodeCount);
	bool e = statistics[hv]->AddStatistic(key, mItemCountS, mNodeCountS);
	return true;
}

template<class TData>
int HashIndex<TData>::Select(int* ids, char*& key) {
	int hv = HashValue(key);
	if(index[hv] == nullptr) return 0;
	int d = index[hv]->Select(ids, key, mKeySize);
	return d;
}

template<class TData>
int HashIndex<TData>::SelectStatistics(char*& key) {
	int hv = HashValue(key);
	if(index[hv] == nullptr) return 0;
	int d = statistics[hv]->SelectStatistics(key, mKeySize);
	return d;
}

template<class TData>
int HashIndex<TData>::HashValue(char* key)
{
	u_int hash = 17;
    u_int multiplier = 31;

    for (size_t i = 0; i < mKeySize; i++) {
        hash = (hash * multiplier) + key[i];
    }

    // Additional bit mixing (optional).
    return hash % mRowCount;

}

