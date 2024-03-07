#ifndef CZADDRUTILS_H
#define CZADDRUTILS_H
#include <iostream>
#include <bitset>
#include <cstddef>
#include "cTreeMetadata.h"

//BSR algorithms seem to count with not fitting the data
//Can be normally sliced using pointer of another type. Have prepared masks to zero potential junk.


// cZAddrUtils - provides methods for ZAddress manipulation

class cZAddrUtils {
    protected:
        cTreeMetadata* metadata;
    public:
        cZAddrUtils(cTreeMetadata* metadata);
        ~cZAddrUtils();
        static bool getZAddressBit(char* zAddress, std::size_t bitIndex);
        virtual void transformDataToZAddress(char* data, char*& za) = 0;
        virtual void transformZAddressToData(char* za, char*& data) = 0;
};
cZAddrUtils::cZAddrUtils(cTreeMetadata* metadata){
    this->metadata = metadata;
}
cZAddrUtils::~cZAddrUtils(){
}
bool cZAddrUtils::getZAddressBit(char* zAddress, std::size_t bitIndex){
	char mask = 1 << bitIndex % 8;
	uint charOrder = bitIndex / 8; // / Int8BitLength; deleno 8, bit order je absolutni pozice v z adress
	uint bit = zAddress[charOrder] & mask;
	return bit > 0;
}

template <typename T>
class cZAddrUtilsTemplate : public cZAddrUtils {
    public:
        cZAddrUtilsTemplate(cTreeMetadata* metadata);
        void transformDataToZAddress(char* data, char*& za);
        void transformZAddressToData(char* za, char*& data);
        
};
template <typename T>
cZAddrUtilsTemplate<T>::cZAddrUtilsTemplate(cTreeMetadata* metadata) : cZAddrUtils(metadata){
}
template <typename T>
void cZAddrUtilsTemplate<T>::transformDataToZAddress(char* data, char*& za){
    if(za == nullptr)
        za = new char[metadata->zAddressBytes];
    T* dataBlock = (T*)data;
	uint bitOrder = 0;
	T* zAddrBlock = (T*)za;
	memset(za, 0, metadata->zAddressBytes);

    uint TBitSize = sizeof(T)*8;

	for (uint i = 0; i < TBitSize; i++)
	{
		T mask = 1 << i; //mask saying what bit to pick out
		for (uint j = 0; j < this->metadata->n; j++, bitOrder++)
		{
			T bit = dataBlock[j] & mask;
			if (bit > 0)
			{
				uint blockPosition = bitOrder / TBitSize; // pozice bloku v ramci pole
				uint shift = bitOrder % TBitSize; //bit order v ramci bloku
				zAddrBlock[blockPosition] |= (1 << shift);
			}
		}
	}
}
template <typename T>
void cZAddrUtilsTemplate<T>::transformZAddressToData(char* za, char*& data){
    if(data == nullptr)
        data = (char*) new T[metadata->n];
    T* zAddrBlock = (T*)za;
    uint bitOrder = 0;
    T* dataBlock = (T*)data;
    memset(data, 0, metadata->n);

    uint TBitSize = sizeof(T)*8;

    for (uint i = 0; i < TBitSize; i++)
    {
        T mask = 1 << i; //bit we are extracting
        for (uint j = 0; j < this->metadata->n; j++, bitOrder++)
        {
            uint blockPosition = bitOrder / TBitSize; // pozice bloku v ramci pole
            uint shift = bitOrder % TBitSize; //bit order v ramci bloku
            
            T bit = zAddrBlock[blockPosition] & (1 << shift); //
            if (bit > 0)
            {
                dataBlock[j] |= (mask);
            }
        }
    }
}

#endif // CZADDRUTILS_H