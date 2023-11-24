#pragma once

#include "cHashTableNode.h"
#include "cMemory.h"

template<class TKey, class TData>
class cHashTable
{
private:
    int mSize;
    cHashTableNode<TKey, TData>** mHashTable;
    int mItemCount;
    int mNodeCount;
    cMemory * mMemory;

private:
    inline int HashValue(const TKey &key) const;

public:
    cHashTable(int size);
    cHashTable(int size, cMemory * memory);
    ~cHashTable();

    bool Add(const TKey &key, const TData &data);
    bool Find(const TKey &key, TData &data) const;
    void PrintStat() const;

    bool AddNoRecursion(const TKey &key, const TData &data);
    bool FindNoRecursion(const TKey &key, TData &data) const;

    bool AddIterative(const TKey &key, const TData &data);
	bool FindIterative(const TKey &key, TData &data) const;
};

template<class TKey, class TData>
cHashTable<TKey, TData>::cHashTable(int size)
{
    mItemCount = 0;
    mNodeCount = 0;
    mSize = size;
    mHashTable = new cHashTableNode<TKey, TData>*[size];
    for (int i = 0; i < mSize; i++)
    {
        mHashTable[i] = NULL;
    }
}

template<class TKey, class TData>
cHashTable<TKey, TData>::cHashTable(int size, cMemory * memory)
{
    mItemCount = 0;
    mNodeCount = 0;
    mSize = size;
    mHashTable = new cHashTableNode<TKey, TData>*[size];
    for (int i = 0; i < mSize; i++)
    {
        mHashTable[i] = NULL;
    }
    mMemory = memory;
}

template<class TKey, class TData>
cHashTable<TKey, TData>::~cHashTable()
{
    if( mMemory = NULL){
        for (int i = 0; i < mSize; i++)
        {
            if (mHashTable[i] != NULL)
            {
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
        if(mMemory == NULL){
            mHashTable[hv] = new cHashTableNode<TKey, TData>();
        }
        else{
            char * mem = mMemory->New(sizeof(cHashTableNode<TKey, TData>));
            mHashTable[hv] = new(mem) cHashTableNode<TKey, TData>();
        }
        mNodeCount++;
    }

    return mHashTable[hv]->Add(key, data, mMemory, mItemCount, mNodeCount);
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::Find(const TKey &key, TData &data) const
{
    int hv = HashValue(key);

    if (mHashTable[hv] == NULL)
    {
        return false; // Key not found
    }
    return mHashTable[hv]->Find(key, data);
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::AddNoRecursion(const TKey& key, const TData& data)
{
    int hv = HashValue(key);

    if (mHashTable[hv] == NULL)
    {
        if (mMemory == NULL) {
            mHashTable[hv] = new cHashTableNode<TKey, TData>();
        }
        else {
            char* mem = mMemory->New(sizeof(cHashTableNode<TKey, TData>));
            mHashTable[hv] = new(mem) cHashTableNode<TKey, TData>();
        }
        mNodeCount++;
    }

    cHashTableNode<TKey, TData>* current = mHashTable[hv];

    while (current->GetNextNode() != NULL) {
        if (current->GetMKey() == key && !current->GetEmptyNode()) {
            return false; // Key already exists
        }
        current = current->GetNextNode();
    }

    if (current->GetMKey() == key && !current->GetEmptyNode()) {
        return false; // Key already exists
    }

    current->SetNextNode(new cHashTableNode<TKey, TData>());
    current = current->GetNextNode();
    current->SetMKey(key);
    current->SetMData(data);
    mItemCount++;

    return true;
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::FindNoRecursion(const TKey& key, TData& data) const
{
    int hv = HashValue(key);

    cHashTableNode<TKey, TData>* current = mHashTable[hv];

    while (current != NULL) {
        if (current->GetMKey() == key) {
            data = current->GetMData();
            return true; // Key found
        }
        current = current->GetNextNode();
    }

    return false; // Key not found
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

		currentNode = currentNode->GetNextNode();
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
