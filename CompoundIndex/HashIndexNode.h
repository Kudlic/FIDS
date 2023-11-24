#pragma once
template<class TData>
class HashIndexNode
{
private:
	bool mEmptyNode;
	char* mData;
	//char* mRec;

	int mStatistic;
	HashIndexNode* mNextNode;
	int* ids;

	int position;
	int mKeySize;
	int mDataValueSize;
	int mSize;
	int mCount;
public:
	HashIndexNode(int keySize, int dataValueSize, int nodeSize);
	HashIndexNode(int len);
	~HashIndexNode();
	
	bool AddStatistic(char*& key, int& itemCount, int& nodeCount);
	bool Add(char*& key, const TData& data, int& itemCount, int& nodeCount);
	int Select(int* data, char*& key, int keySize);
	int SelectStatistics(char*& key, int keySize);
};

template<class TData>
HashIndexNode<TData>::HashIndexNode(int keySize, int dataValueSize, int nodeSize)
{
	mNextNode = nullptr;
	mEmptyNode = true;
	mStatistic = 0;
	position = 0;
	mKeySize = keySize;
	mDataValueSize = dataValueSize;
	mSize = nodeSize;
	mData = new char[nodeSize];

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

	delete mData;
}

template<class TData>
bool HashIndexNode<TData>::AddStatistic(char*& key, int& itemCount, int& nodeCount)
{
	bool found = true;
	HashIndexNode<TData>* nNode = this;
	do
	{
		//pointer into data
		char* rec = nNode->mData;

		//iterate over all records of node
		for (int i = 0; i < nNode->mCount; i++) {
			//check if key is on rec pointer, if not, set found to false and proceed
			for (int k = 0; k < mKeySize; k++) {
				if (rec[k] != key[k]) {
					found = false;
					break;
				}
			}

			if (found) {
				// get statistics
				int data = int((unsigned char)(rec[mKeySize]) |
					(unsigned char)(rec[mKeySize + 1]) << 8 |
					(unsigned char)(rec[mKeySize + 2]) << 16 |
					(unsigned char)(rec[mKeySize + 3]) << 24);
				data++;

				// add statistics
				for (size_t k = 0; k < mDataValueSize; k++)
				{
					rec[mKeySize + k] = (static_cast<unsigned int>(data) >> (8 * k)) & 0xff;
				}
				return true;
			}
			else {
				found = true;
			}

			//move pointer
			rec += mKeySize + mDataValueSize;
		}

		// If we did not yet find the according record and ther is no space,
		// lets create a new one and jump to it
		if (!nNode->mEmptyNode)
		{
			if (nNode->position >= mSize) {
				if (nNode->mNextNode == nullptr) {
					nNode->mNextNode = new HashIndexNode(mKeySize, mDataValueSize, mSize);
					nodeCount++;
				}
				nNode = nNode->mNextNode;
				continue;
			}
		}

		// create new
		
		// add keys
		memcpy(nNode->mData + nNode->position, key, mKeySize);
		nNode->position += mKeySize;

		// add data
		int data = 1;//need address of the int
		memcpy(nNode->mData + nNode->position, &data, sizeof(int));
		nNode->position += sizeof(int);

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
			if (nNode->position >= mSize) {
				if (nNode->mNextNode == nullptr) {
					nNode->mNextNode = new HashIndexNode(mKeySize, mDataValueSize, mSize);
					nodeCount++;
				}
				nNode = nNode->mNextNode;
				continue;
			}
		}
		//printf("add position: %d, maxPosition: %d\n", nNode->position, nNode->mSize);
		// add keys
		memcpy(nNode->mData + nNode->position, key, mKeySize);
		nNode->position += mKeySize;

		// add data
		memcpy(nNode->mData + nNode->position, &data, mDataValueSize);
		nNode->position += mDataValueSize;

		nNode->mCount++;
		nNode->mEmptyNode = false;
		itemCount++;

		return true;

	} while (nNode != nullptr);
	return false;
}

template<class TData>
int HashIndexNode<TData>::Select(int*data, char*& key, int keySize) {
	HashIndexNode<TData>* nNode = this;
	int result = 0;
	do
	{
		if (!nNode->mEmptyNode)
		{
			for (int i = 0; i < nNode->mCount; i++) {
				bool same = true;
				for (int k = 0; k < keySize; k++) {
					if (key[k] != nNode->mData[i*(mKeySize+mDataValueSize)+k]) {
						same = false;
						break;
					}
				}
				if (same == true) {			
					// get data
					int idata = 
						int((unsigned char)(nNode->mData[i * (mKeySize + mDataValueSize) + mKeySize]) |
						(unsigned char)(nNode->mData[i * (mKeySize + mDataValueSize) + mKeySize + 1]) << 8 |
						(unsigned char)(nNode->mData[i * (mKeySize + mDataValueSize) + mKeySize + 2]) << 16 |
						(unsigned char)(nNode->mData[i * (mKeySize + mDataValueSize) + mKeySize + 3]) << 24);
					data[result] = idata;
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
			for (int i = 0; i < nNode->mCount; i++) {
				bool same = true;
				for (int k = 0; k < keySize; k++) {
					if (key[k] != nNode->mData[i * (mKeySize + mDataValueSize) + k]) {
						same = false;
						break;
					}
				}

				if (same == true) {
					return int((unsigned char)(nNode->mData[i * (mKeySize + mDataValueSize) + mKeySize]) |
						(unsigned char)(nNode->mData[i * (mKeySize + mDataValueSize) + mKeySize + 1]) << 8 |
						(unsigned char)(nNode->mData[i * (mKeySize + mDataValueSize) + mKeySize + 2]) << 16 |
						(unsigned char)(nNode->mData[i * (mKeySize + mDataValueSize) + mKeySize + 3]) << 24);
				}
			}
		}
		nNode = nNode->mNextNode;
	} while (nNode != nullptr);
	return result;
}


