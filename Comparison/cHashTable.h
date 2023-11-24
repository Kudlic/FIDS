#pragma once

#include "cHashTableNode.h"

template<class TKey, class TData>
class cHashTable
{
private:
	int mSize;
	cHashTableNode<TKey,TData>** mHashTable;
	int mItemCount = 0;
	int mNodeCount = 0;
	cMemory *mMemory;

private:
	inline int HashValue(const TKey &key) const;
	cHashTable();

public:
	cHashTable(int size);
	cHashTable(int size, cMemory *memory);
	~cHashTable();

	bool Add(const TKey &key, const TData &data);
	bool AddIterative(const TKey &key, const TData &data);
	bool Find(const TKey &key, TData &data) const;
	bool FindIterative(const TKey &key, TData &data) const;
	void PrintStat() const;
};

template<class TKey, class TData>
cHashTable<TKey,TData>::cHashTable()
{
	mSize = 1000000;
	mHashTable = new cHashTableNode<TKey,TData>*[1000000];
	for (int i = 0; i < mSize; i++)
	{
		mHashTable[i] = NULL;
	}
}

template<class TKey, class TData>
cHashTable<TKey,TData>::cHashTable(int size)
{
	mSize = size;
	mHashTable = new cHashTableNode<TKey,TData>*[size];
	for (int i = 0; i < mSize; i++)
	{
		mHashTable[i] = NULL;
	}
}

template<class TKey, class TData>
cHashTable<TKey,TData>::cHashTable(int size, cMemory *memory)
	: mSize(size), mMemory(memory)
{
	mSize = size;
	mHashTable = new cHashTableNode<TKey,TData>*[size];
	for (int i = 0; i < mSize; i++)
	{
		mHashTable[i] = NULL;
	}
	mMemory = memory;
}

template<class TKey, class TData>
cHashTable<TKey, TData>::~cHashTable()
{
	// TODO
	if (mMemory == NULL) {
		for (int i = 0; i < mSize; i++) {
			if (mHashTable[i] != NULL) {
				delete mHashTable[i];
			}
		}
	}
	delete mHashTable;
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::Add(const TKey &key, const TData &data)
{
	int hv = HashValue(key);

	if (mHashTable[hv] == NULL)
	{
		if (mMemory == NULL)
		{
			mHashTable[hv] = new cHashTableNode<TKey, TData>();
		}
		else {
			char* mem = mMemory->New(sizeof(cHashTableNode<TKey, TData>));
			mHashTable[hv] = new (mem)cHashTableNode<TKey, TData>();
		}
		mNodeCount++;
	}

	return mHashTable[hv]->Add(key, data, mMemory, mItemCount, mNodeCount);
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::AddIterative(const TKey &key, const TData &data)
{
	int hv = HashValue(key);

	if (mHashTable[hv] == NULL)
	{
		if (mMemory == NULL)
		{
			mHashTable[hv] = new cHashTableNode<TKey, TData>();
		}
		else {
			char* mem = mMemory->New(sizeof(cHashTableNode<TKey, TData>));
			mHashTable[hv] = new (mem)cHashTableNode<TKey, TData>();
		}
		mNodeCount++;
	}

	return mHashTable[hv]->AddIterative(key, data, mMemory, mItemCount, mNodeCount);
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::Find(const TKey &key, TData &data) const
{
	// TODO
	int hash = HashValue(key);

	cHashTableNode<TKey, TData>* currentNode = mHashTable[hash];

	while (currentNode)
	{
		if (currentNode->Find(key, data))
		{
			return true;
		}

		currentNode = currentNode->GetNext();
	}

	return false;
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::FindIterative(const TKey &key, TData &data) const
{
	// TODO
	int hash = HashValue(key);

	cHashTableNode<TKey, TData>* currentNode = mHashTable[hash];

	while (currentNode)
	{
		if (currentNode->FindIterative(key, data))
		{
			return true;
		}

		currentNode = currentNode->GetNext();
	}

	return false;
}

template<class TKey, class TData>
inline int cHashTable<TKey, TData>::HashValue(const TKey &key) const
{
	return key % mSize;
}

template<class TKey, class TData>
void cHashTable<TKey, TData>::PrintStat() const
{
	printf("HashTable Statistics: Size: %d, ItemCount: %d, NodeCount: %d, Avg. Items/Slot: %.2f.\n",
		mSize, mItemCount, mNodeCount, (float)mItemCount / mSize);
}