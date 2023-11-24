#pragma once

#include <assert.h>
#include <stdlib.h>

template<class TKey, class TData>
class cRecord
{
public:
    TKey key;
    TData data;
};

template<class TKey, class TData>
class cHeapTable
{
private:
    int mCapacity;
    int mCount;
    cRecord<TKey, TData>** mData; // Array of pointers to cRecord instances.

public:
    cHeapTable(int capacity);
    ~cHeapTable();

    bool Add(const TKey &key, const TData &data);
    bool Get(int rowId, TKey &key, TData &data) const;
    bool Find(const TKey &key, TData &data) const;
};

template<class TKey, class TData>
cHeapTable<TKey, TData>::cHeapTable(int capacity)
{
    mCapacity = capacity;
    mCount = 0;
    mData = new cRecord<TKey, TData>*[mCapacity];

    for (int i = 0; i < mCapacity; i++) {
        mData[i] = nullptr;
    }
}

template<class TKey, class TData>
cHeapTable<TKey, TData>::~cHeapTable()
{
    for (int i = 0; i < mCount; i++) {
        delete mData[i];
    }
    delete[] mData;
}

template<class TKey, class TData>
bool cHeapTable<TKey, TData>::Add(const TKey &key, const TData &data)
{
    if (mCount >= mCapacity) {
        return false;
    }

    cRecord<TKey, TData>* record = new cRecord<TKey, TData>;
    record->key = key;
    record->data = data;

    mData[mCount] = record;
    mCount++;

    return true;
}

template<class TKey, class TData>
bool cHeapTable<TKey, TData>::Get(int rowId, TKey &key, TData &data) const
{
    if (rowId < 0 || rowId >= mCount) {
        return false;
    }

    key = mData[rowId]->key;
    data = mData[rowId]->data;

    return true;
}

template<class TKey, class TData>
bool cHeapTable<TKey, TData>::Find(const TKey &key, TData &data) const
{
    for (int i = 0; i < mCount; i++)
    {
        if (mData[i]->key == key)
        {
            data = mData[i]->data;
            return true; // Found the key, return the corresponding data.
        }
    }

    return false; // Key not found in the table.
}
