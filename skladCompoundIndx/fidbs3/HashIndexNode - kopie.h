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
	int* ids;
	int position;
public:
	HashIndexNode();
	HashIndexNode(int len);
	~HashIndexNode();

	bool Add(char*& key, const TData& data, int& itemCount, int& nodeCount);
	bool AddStatistic(char*& key, int& itemCount, int& nodeCount, int keySize);
	bool AddIndex(char*& key, const TData& data, int& itemCount, int len, int keySize);
	int Select(char*& key, int keySize);
	int Select2(char*& key, int keySize);
};

template<class TData>
HashIndexNode<TData>::HashIndexNode()
{
	mNextNode = nullptr;
	mEmptyNode = true;
	mStatistic = 0;
}

template<class TData>
HashIndexNode<TData>::HashIndexNode(int len)
{
	mNextNode = nullptr;
	mEmptyNode = true;
	mStatistic = 0;
	ids = new int[len];
	position = 0;
}

template<class TData>
HashIndexNode<TData>::~HashIndexNode()
{
	if (mNextNode != nullptr)
	{
		delete mNextNode;
		mNextNode = nullptr;
	}

	for (int i = 0; i < position; i++) {
		if (ids[i] != nullptr) {
			delete ids[i];
		}
	}
	delete ids;
}

template<class TData>
bool HashIndexNode<TData>::Add(char*& key, const TData& data, int& itemCount, int& nodeCount)
{
	HashIndexNode<TData>* nNode = this;
	do
	{
		if (!nNode->mEmptyNode)
		{
			nNode = nNode->mNextNode;
		}
		else
		{
			nNode->mKey = key;
			nNode->mData = data;
			nNode->mEmptyNode = false;
			itemCount++;
			if (nNode->mNextNode == nullptr) {
				nNode->mNextNode = new HashIndexNode();
			}
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
			}
		}
		else
		{
			nNode->mKey = key;
			nNode->mStatistic++;
			nNode->mEmptyNode = false;
			itemCount++;
			if (nNode->mNextNode == nullptr) {
				nNode->mNextNode = new HashIndexNode();
			}
			return true;
		}

	} while (nNode != nullptr);
	return false;
}

template<class TData>
bool HashIndexNode<TData>::AddIndex(char*& key, const TData& data, int& itemCount, int len, int keySize)
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
				nNode->mStatistic++;
				nNode->ids[nNode->position] = data;
				nNode->position++;
				return true;
			}
			else {
				nNode = nNode->mNextNode;
			}
		}
		else
		{
			nNode->mKey = key;
			nNode->ids[nNode->position] = data;
			nNode->position++;
			nNode->mStatistic++;
			nNode->mEmptyNode = false;
			itemCount++;
			if (nNode->mNextNode == nullptr) {
				nNode->mNextNode = new HashIndexNode(len);
			}
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

template<class TData>
int HashIndexNode<TData>::Select2(char*& key, int keySize) {
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
				return nNode->position;
			}

		}
		nNode = nNode->mNextNode;
	} while (nNode != nullptr);
	return result;
}

