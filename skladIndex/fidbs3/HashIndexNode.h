#pragma once
template<class TData>
class HashIndexNode
{
private:
	bool mEmptyNode;
	char* mKey;
	TData mData;
	int mStatistic;
	HashIndexNode* mNextNode;
public:
	HashIndexNode();
	~HashIndexNode();

	bool Add(char*& key, const TData& data, int& itemCount, int& nodeCount);
	bool AddStatistic(char*& key, int& itemCount, int& nodeCount, int keySize);
	int Select(char*& key, int keySize);
};

template<class TData>
HashIndexNode<TData>::HashIndexNode()
{
	mNextNode = nullptr;
	mEmptyNode = true;
	mStatistic = 0;
}

template<class TData>
HashIndexNode<TData>::~HashIndexNode()
{
	if (mNextNode != nullptr)
	{
		delete mNextNode;
		mNextNode = nullptr;
	}
}

template<class TData>
bool HashIndexNode<TData>::Add(char*& key, const TData& data, int& itemCount, int& nodeCount)
{
	HashIndexNode<TData>* nNode = this;
	do
	{
		if (!nNode->mEmptyNode)
		{
			
			if (nNode->mNextNode == nullptr) {
				nNode->mNextNode = new HashIndexNode();
			}
			nNode = nNode->mNextNode;
			nodeCount++;
		}
		else
		{
			mKey = key;
			mData = data;
			mEmptyNode = false;
			itemCount++;
			return true;
		}

	} while (nNode != nullptr);
	return false;
}

template<class TData>
bool HashIndexNode<TData>::AddStatistic(char*& key, int& itemCount, int& nodeCount, int keySize)
{
	HashIndexNode<TData>* nNode = this;
	do
	{
		if (!nNode->mEmptyNode)
		{
			bool same = true;
			for (int i = 0; i < keySize; i++) {
				if (key[i] != mKey[i]) {
					same = false;
				}
			}

			if (same == true) {
				mStatistic++;
				return true;
			}
			else {
				nNode = nNode->mNextNode;
				nodeCount++;
			}
		}
		else
		{
			mKey = key;
			mStatistic++;
			mEmptyNode = false;
			itemCount++;
			return true;
		}

	} while (nNode != nullptr);
	return false;
}

template<class TData>
int HashIndexNode<TData>::Select(char*& key, int keySize) {
	HashIndexNode<TData>* nNode = this;
	int result = 0;
	do
	{
		if (!nNode->mEmptyNode)
		{
			bool same = true;
			for (int i = 0; i < keySize; i++) {
				if (key[i] != mKey[i]) {
					same = false;
				}
			}

			if (same == true) {
				result++;
			}
			
		}
		nNode = nNode->mNextNode;
	} while (nNode != nullptr);
	return result;
}

