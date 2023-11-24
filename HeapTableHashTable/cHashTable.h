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

private:
    inline int HashValue(const TKey &key) const;

public:
    cHashTable(int size);
    ~cHashTable();

    bool Add(const TKey &key, const TData &data);
    bool Find(const TKey &key, TData &data) const;
    void PrintStat() const;
};

template<class TKey, class TData>
cHashTable<TKey, TData>::cHashTable(int size)
{
    mSize = size;
    mHashTable = new cHashTableNode<TKey, TData>*[size];
    for (int i = 0; i < mSize; i++)
    {
        mHashTable[i] = NULL;
    }
}

template<class TKey, class TData>
cHashTable<TKey, TData>::~cHashTable()
{
    for (int i = 0; i < mSize; i++)
    {
        if (mHashTable[i] != NULL)
        {
            delete mHashTable[i];
        }
    }
    delete[] mHashTable;
}

template<class TKey, class TData>
bool cHashTable<TKey, TData>::Add(const TKey &key, const TData &data)
{
    int hv = HashValue(key);

    if (mHashTable[hv] == NULL)
    {
        mHashTable[hv] = new cHashTableNode<TKey, TData>();
        mNodeCount++;
    }

    return mHashTable[hv]->Add(key, data, mItemCount, mNodeCount);
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
