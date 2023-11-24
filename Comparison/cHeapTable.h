#pragma once

#include <assert.h>
#include <stdlib.h>
#include <cstring>

template<class TKey, class TData>
class cHeapTable
{
private:
	int mCapacity;
	int mCount;
	char* mData;
	int const mRowSize = sizeof(TKey) + sizeof(TData);

private:
	inline char* GetRowPointer(int rowId) const;

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
	mData = new char[mRowSize * capacity];
}

template<class TKey, class TData>
cHeapTable<TKey, TData>::~cHeapTable()
{
	if (mData != NULL)
	{
		delete mData;
		mData = NULL;
		mCapacity = 0;
		mCount = 0;
	}
}

template<class TKey, class TData>
inline char* cHeapTable<TKey, TData>::GetRowPointer(int rowId) const
{
	return mData + rowId * mRowSize;
}

template<class TKey, class TData>
bool cHeapTable<TKey, TData>::Get(int rowId, TKey &key, TData &data) const
{
	bool ret = false;
	assert(rowId >= 0 && rowId < mCount);
	ret = true;

	char* p = GetRowPointer(rowId);
	key = *((TKey*)p);
	data = *((TData*)(p + sizeof(TKey)));

	return ret;
}

template<class TKey, class TData>
bool cHeapTable<TKey, TData>::Add(const TKey &key, const TData &data)
{
	if (mCount >= mCapacity) {
		return false;
	}

	char* offset = mData + mCount * mRowSize;

	memcpy(offset, &key, sizeof(TKey));
	memcpy(offset + sizeof(TKey), &data, sizeof(TData));

	mCount++;

	return true;
}

template <class TKey, class TData>
bool cHeapTable<TKey, TData>::Find(const TKey &key, TData &data) const
{
	//TKey currentKey;
	//TData currentData;
	char* offset;
	
	for (int i = 0; i < mCount; i++) {
		// Alternativa s Get - 8.63 op/s
		//Get(i, currentKey, currentData);

		//if (currentKey == key) {
		//	data = currentData;
		//	return true;
		//}

		// Alternativa s přímým přístupem do paměti - 74.47 op/s
		offset = mData + i * mRowSize;
		if (*reinterpret_cast<TKey*>(offset) == key){
			data = *reinterpret_cast<TData*>(offset + sizeof(TKey));
			return true;
		}

		// Alternativa s kopírováním bloků paměti - 28.09 op/s
		//memcpy(&currentKey, mData + i * mRowSize, sizeof(TKey));
		//if (currentKey == key){
		//	memcpy(&data, mData + i * mRowSize + sizeof(TKey), sizeof(TData));
		//	return true;
		//}

		// Alternativa ala C-style - 31.76 op/s
		//if (*(TKey*)(mData + i * mRowSize) == key){
		//	data = *(TData*)(mData + i * mRowSize + sizeof(TKey));
		//	return true;
		//}
	}

	// Pokus udělat cyklus jinak - 28.72 op/s
	//TKey* currentKeyPtr = reinterpret_cast<TKey*>(mData);
	//TData* currentDataPtr = reinterpret_cast<TData*>(mData + sizeof(TKey));
	//for (int i = 0; i < mCount; i++, currentKeyPtr += mRowSize / sizeof(TKey)) {
	//	if (*currentKeyPtr == key) {
	//		data = *currentKeyPtr;
	//		return true;
	//	}
	//}

	return false;
}