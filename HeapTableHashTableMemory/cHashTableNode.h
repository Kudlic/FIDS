#pragma once

#include "cHashTableNode.h"
#include "cMemory.h"

template<class TKey, class TData>
class cHashTableNode
{
private:
    bool mEmptyNode;
    TKey mKey;
    TData mData;
    cHashTableNode* mNextNode;

public:
    cHashTableNode();
    ~cHashTableNode();

    bool Add(const TKey &key, const TData &data, cMemory *memory, int &itemCount, int &nodeCount);
    bool Find(const TKey &key, TData &data) const;
    bool AddIterative(const TKey &key, const TData &data, cMemory *memory, int &itemCount, int &nodeCount);
	bool FindIterative(const TKey &key, TData &data) const;

    cHashTableNode* GetNextNode() const { return mNextNode; }
    TKey GetMKey() const { return mKey; }
    TData GetMData() const { return mData; }
    bool GetEmptyNode() const { return mEmptyNode; }
    void SetNextNode(cHashTableNode* nextNode) { mNextNode = nextNode; }
    void SetMKey(TKey key) { mKey = key; }
    void SetMData(TData data) { mData = data; }
    void SetEmptyNode(bool emptyNode) { mEmptyNode = emptyNode; }
};

template<class TKey, class TData>
cHashTableNode<TKey, TData>::cHashTableNode()
{
    mNextNode = NULL;
    mEmptyNode = true;
}

template<class TKey, class TData>
cHashTableNode<TKey, TData>::~cHashTableNode()
{
    if (mNextNode != NULL)
    {
        delete mNextNode;
        mNextNode = NULL;
    }
}

template<class TKey, class TData>
bool cHashTableNode<TKey, TData>::Add(const TKey &key, const TData &data, cMemory *memory, int &itemCount, int &nodeCount)
{
    bool ret = true;

    if (!mEmptyNode) {
        if (mKey == key) {
            ret = false;
        }
        else {
            if (mNextNode == NULL) {
                if(memory == NULL){
                    mNextNode = new cHashTableNode<TKey, TData>();
                }
                else{
                    char* mem = memory->New(sizeof(cHashTableNode<TKey, TData>));
                    mNextNode = new(mem) cHashTableNode<TKey, TData>();
                }
                nodeCount++;
            }
            ret = mNextNode->Add(key, data, memory, itemCount, nodeCount);
        }
    }
    else {
        mKey = key;
        mData = data;
        mEmptyNode = false;
        itemCount++;

        ret = true;
    }
    return ret;
}

template<class TKey, class TData>
bool cHashTableNode<TKey, TData>::Find(const TKey &key, TData &data) const
{
    if (mEmptyNode) {
        return false; // Key not found
    }
    else if (mKey == key) {
        data = mData;
        return true; // Found the key, return the corresponding data.
    }
    else if (mNextNode != NULL) {
        return mNextNode->Find(key, data);
    }
    return false; // Key not found in the chain.
}

template<class TKey, class TData>
bool cHashTableNode<TKey, TData>::AddIterative(const TKey &key, const TData &data, cMemory *memory, int &itemCount, int &nodeCount)
{
	cHashTableNode<TKey, TData>* currentNode = this;

    while (currentNode != nullptr)
    {
        if (!currentNode->mEmptyNode)
        {
            if (currentNode->mKey == key)
            {
                return false;
            }

            if (currentNode->mNextNode == nullptr)
            {
                if (memory != nullptr)
                {
                    char* mem = memory->New(sizeof(cHashTableNode<TKey, TData>));
                    currentNode->mNextNode = new (mem) cHashTableNode<TKey, TData>();
                }
                else
                {
                    currentNode->mNextNode = new cHashTableNode<TKey, TData>();
                }
                nodeCount++;
            }

            currentNode = currentNode->mNextNode;
        }
        else
        {
            currentNode->mKey = key;
            currentNode->mData = data;
            currentNode->mEmptyNode = false;
            itemCount++;
            return true;
        }
    }

    return false;
}

template<class TKey, class TData>
bool cHashTableNode<TKey, TData>::FindIterative(const TKey &key, TData &data) const
{
	// TODO
	const cHashTableNode<TKey, TData>* currentNode = this;

    while (currentNode != nullptr)
    {
        if (!currentNode->mEmptyNode && currentNode->mKey == key)
        {
            data = currentNode->mData;
            return true;
        }

        currentNode = currentNode->mNextNode;
    }

    return false;
}