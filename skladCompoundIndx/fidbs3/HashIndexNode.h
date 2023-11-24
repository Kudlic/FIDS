#pragma once
template<class TData>
class HashIndexNode
{
private:
	bool mEmptyNode;
	char* mKey;
	TData mData;
	char* mDatas;
	char* mRec;
	int mStatistic;
	HashIndexNode* mNextNode;
	int* ids;
	int position;
	int mKeySize;
	int mDataSize;
	int mSize;
	int mCount;
public:
	HashIndexNode(int keySize, int dataSize);
	HashIndexNode(int len);
	~HashIndexNode();

	
	bool AddStatistic(char*& key, int& itemCount, int& nodeCount, int keySize);
	bool Add(char*& key, const TData& data, int& itemCount, int& nodeCount);
	bool AddIndex(char*& key, const TData& data, int& itemCount, int len, int keySize);
	int Select(int* datas, char*& key, int keySize);
	int SelectStatistics(char*& key, int keySize);
	int Select2(char*& key, int keySize);
};

template<class TData>
HashIndexNode<TData>::HashIndexNode(int keySize, int dataSize)
{
	mNextNode = nullptr;
	mEmptyNode = true;
	mStatistic = 0;
	position = 0;
	mKeySize = keySize;
	mDataSize = dataSize;
	mSize = 5*1024 / (mKeySize + mDataSize);
	mDatas = new char[5*1024];
	mRec = mDatas;
	mCount = 0;
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

	delete mDatas;
}

template<class TData>
bool HashIndexNode<TData>::AddStatistic(char*& key, int& itemCount, int& nodeCount, int keySize)
{
	bool notfound = true;
	HashIndexNode<TData>* nNode = this;
	do
	{
		char* rec = nNode->mDatas;
		for (int i = 0; i < nNode->mCount; i++) {
			for (int k = 0; k < keySize; k++) {
				if (rec[k] != key[k]) {
					notfound = false;
					break;
				}
			}

			if (notfound == true) {
				// get statistics
				int data = int((unsigned char)(rec[mKeySize]) |
					(unsigned char)(rec[mKeySize + 1]) << 8 |
					(unsigned char)(rec[mKeySize + 2]) << 16 |
					(unsigned char)(rec[mKeySize + 3]) << 24);
				data++;

				// add statistics
				for (size_t k = 0; k < mDataSize; k++)
				{
					rec[mKeySize + k] = (static_cast<unsigned int>(data) >> (8 * k)) & 0xff;
				}
				return true;
			}
			else {
				notfound = true;
			}


			rec += mKeySize + mDataSize;
		}

		// not empty
		if (!nNode->mEmptyNode)
		{
			if (nNode->mCount >= mSize) {
				if (nNode->mNextNode == nullptr) {
					nNode->mNextNode = new HashIndexNode(mKeySize, mDataSize);
				}
				nNode = nNode->mNextNode;
				continue;
			}
		}

		// create new
		
		// add keys
		for (int i = 0; i < mKeySize; i++) {
			nNode->mRec[0] = key[i];
			nNode->mRec++;
		}

		int data = 1;
		// add statistics
		for (size_t i = 0; i < mDataSize; i++)
		{
			nNode->mRec[0] = (static_cast<unsigned int>(data) >> (8 * i)) & 0xff;
			nNode->mRec++;
		}

		nNode->mCount++;
		nNode->mEmptyNode = false;
		itemCount++;

		return true;

	} while (nNode != nullptr);
	return false;
}

template<class TData>
bool HashIndexNode<TData>::Add(char*& key, const TData& data, int& itemCount, int& nodeCount)
{
	HashIndexNode<TData>* nNode = this;
	do
	{
		if (!nNode->mEmptyNode)
		{
			if (nNode->mCount >= mSize) {
				if (nNode->mNextNode == nullptr) {
					nNode->mNextNode = new HashIndexNode(mKeySize, mDataSize);
				}
				nNode = nNode->mNextNode;
				continue;
			}
		}

		// add keys
		for (int i = 0; i < mKeySize; i++) {
			nNode->mDatas[nNode->position] = key[i];
			nNode->position++;
		}

		// add data
		for (size_t i = 0; i < mDataSize; i++)
		{
			nNode->mDatas[nNode->position] = (static_cast<unsigned int>(data) >> (8 * i)) & 0xff;
			nNode->position++;
		}

		nNode->mCount++;
		nNode->mEmptyNode = false;
		itemCount++;

		return true;

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
int HashIndexNode<TData>::Select(int*datas, char*& key, int keySize) {
	HashIndexNode<TData>* nNode = this;
	int result = 0;
	do
	{
		if (!nNode->mEmptyNode)
		{
			for (int i = 0; i < mCount; i++) {
				bool same = true;
				for (int k = 0; k < keySize; k++) {
					if (key[k] != nNode->mDatas[i*(mKeySize+mDataSize)+k]) {
						same = false;
					}
				}
				if (same == true) {			
					// get data
					int data = int((unsigned char)(nNode->mDatas[i * (mKeySize + mDataSize) + mKeySize]) |
						(unsigned char)(nNode->mDatas[i * (mKeySize + mDataSize) + mKeySize + 1]) << 8 |
						(unsigned char)(nNode->mDatas[i * (mKeySize + mDataSize) + mKeySize + 2]) << 16 |
						(unsigned char)(nNode->mDatas[i * (mKeySize + mDataSize) + mKeySize + 3]) << 24);
					datas[result] = data;
					result++;				
				}
			}
		}
		nNode = nNode->mNextNode;
	} while (nNode != nullptr);
	return result;
}

template<class TData>
int HashIndexNode<TData>::SelectStatistics(char*& key, int keySize) {
	HashIndexNode<TData>* nNode = this;
	int result = 0;
	do
	{
		if (!nNode->mEmptyNode)
		{
			for (int i = 0; i < mCount; i++) {
				bool same = true;
				for (int k = 0; k < keySize; k++) {
					if (key[k] != nNode->mDatas[i * (mKeySize + mDataSize) + k]) {
						same = false;
						break;
					}
				}

				if (same == true) {
					return int((unsigned char)(nNode->mDatas[i * (mKeySize + mDataSize) + mKeySize]) |
						(unsigned char)(nNode->mDatas[i * (mKeySize + mDataSize) + mKeySize + 1]) << 8 |
						(unsigned char)(nNode->mDatas[i * (mKeySize + mDataSize) + mKeySize + 2]) << 16 |
						(unsigned char)(nNode->mDatas[i * (mKeySize + mDataSize) + mKeySize + 3]) << 24);
				}
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

