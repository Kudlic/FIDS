#ifndef CZADDRUTILS_H
#define CZADDRUTILS_H
#include <iostream>
#include <bitset>
#include <cstddef>
#include "cTreeMetadata.h"

#define uint unsigned int
#define u_int64_t unsigned long long
#define u_int32_t unsigned int
#define u_int16_t unsigned short
#define u_int8_t unsigned char

//BSR algorithms seem to count with not fitting the data
//Can be normally sliced using pointer of another type. Have prepared masks to zero potential junk.


// cZAddrUtils - provides methods for ZAddress manipulation
#define Int8ByteLength 1
#define Int16ByteLength 2
#define Int32ByteLength 4
#define Int64ByteLength 8

class cZAddrUtils {
    private:
        typedef bool (cZAddrUtils::*IsInRectangleFnc)(char*, char*, char*);
        typedef bool (cZAddrUtils::*IsIntersectedFnc)(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2);
        typedef bool (cZAddrUtils::*IsIntersected_ZrQr_blockFnc)(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
    protected:
        //section for helper variables
        //64b vars
        uint mBsrSteps64; //počet iterací pro BSR algoritmy
        uint64_t mBsrDefaultMask64;  //plné 0xFF bytů
        uint64_t mBsrFirstZaBlockMask64;  //Maska pro MSB, který může mít prázdné byty v bloku, Nevychází-li přesně byty všech dimenzí na byty všech bloků, jsou na začátek doplněny nuly
        uint mRob64, mLob64; // Pro BSR neřešíme, jen pro fasty, o kolik bloků dříve je třeba se podívat pro rekonstrukci masky
        uint mRobShift64, mLobShift64; 
        uint64_t* mBsrMaskArrayL64;
        uint64_t* mBsrMaskArrayH64;
        uint64_t* mBsrShiftMaskArray64;
        uint64_t* mMaskArrayAL64;
        uint64_t* mMaskArrayAH64;
        //32b vars
        uint mBsrSteps32;
        uint32_t mBsrDefaultMask32; 
        uint32_t mBsrFirstZaBlockMask32;
        uint mRob32, mLob32;
        uint mRobShift32, mLobShift32;
        uint32_t* mBsrMaskArrayL32;
        uint32_t* mBsrMaskArrayH32;
        uint32_t* mBsrShiftMaskArray32; 
        uint32_t* mMaskArrayAL32;
        uint32_t* mMaskArrayAH32;
        //8b vars
        uint mBsrSteps8;
        uint8_t mBsrDefaultMask8;
        uint8_t mBsrFirstZaBlockMask8;
        uint mRob8, mLob8;
        uint mRobShift8, mLobShift8;
        uint8_t* mBsrMaskArrayL8;
        uint8_t* mBsrMaskArrayH8;
        uint8_t* mBsrShiftMaskArray8;
        uint8_t* mMaskArrayAL8;
        uint8_t* mMaskArrayAH8;

        //metadata link
        cTreeMetadata* metadata;
        //section for func pointers
        IsInRectangleFnc isInRectangle;
        IsIntersectedFnc isIntersected;
        IsIntersected_ZrQr_blockFnc isIntersected_ZrQr_block;
    public:
        cZAddrUtils(cTreeMetadata* metadata);
        ~cZAddrUtils();
        static bool getZAddressBit(char* zAddress, std::size_t bitIndex);
        static bool BitScanReverse64asm(unsigned long * index, uint64_t mask);
        //virtual methods, that need to be initialised with type T in cZAddrUtilsTemplate
        virtual void transformDataToZAddress(char* data, char*& za) = 0;
        virtual void transformZAddressToData(char* za, char*& data) = 0;

        bool IsInRectangle(char* za_t, char* za_ql, char* za_qh);
        bool IsIntersected(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2);
        bool IsIntersected_ZrQr_block(char* zi_a, char* zi_b, char* za_ql, char* za_qh);

        bool IsInRectangle_bsr_8(char* za_t, char* za_ql, char* za_qh);
        bool IsInRectangle_fast_8(char* za_t, char* za_ql, char* za_qh);
        bool IsInRectangle_bsr_32(char* za_t, char* za_ql, char* za_qh);
        bool IsInRectangle_fast_32(char* za_t, char* za_ql, char* za_qh);
        bool IsInRectangle_bsr_64(char* za_t, char* za_ql, char* za_qh);
        bool IsInRectangle_fast_64(char* za_t, char* za_ql, char* za_qh);

        bool IsIntersected_bsr_64(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2);
        bool IsIntersected_fast_64(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2);
        
        bool IsIntersected_ZrQr_block_bsr_8(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
        bool IsIntersected_ZrQr_block_fast_8(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
        bool IsIntersected_ZrQr_block_bsr_32(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
        bool IsIntersected_ZrQr_block_fast_32(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
        bool IsIntersected_ZrQr_block_bsr_64(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
        bool IsIntersected_ZrQr_block_fast_64(char* zi_a, char* zi_b, char* za_ql, char* za_qh);

		//whole number comparators, using 64b blocks
		bool isZAddrEQ(char* za1, char* za2);
		bool isZAddrLT(char* za1, char* za2);
		bool isZAddrLEQT(char* za1, char* za2);
		bool isZAddrGT(char* za1, char* za2);
		bool isZAddrGEQT(char* za1, char* za2);
		int cmpZAddress(char* za1, char* za2);

		//setters for function pointers
		void setIsInRectangleFast8(){isInRectangle = &cZAddrUtils::IsInRectangle_fast_8;}
		void setIsInRectangleBsr8(){isInRectangle = &cZAddrUtils::IsInRectangle_bsr_8;}
		void setIsInRectangleFast32(){isInRectangle = &cZAddrUtils::IsInRectangle_fast_32;}
		void setIsInRectangleBsr32(){isInRectangle = &cZAddrUtils::IsInRectangle_bsr_32;}
};
cZAddrUtils::cZAddrUtils(cTreeMetadata* metadata){
    this->metadata = metadata;
    //Steps: number of iterations for BSR algorithms, also number of blocks needed to cover ZAddress
    mBsrSteps64 = (metadata->n * metadata->attributeSize + Int64ByteLength - 1) / Int64ByteLength;
    mBsrSteps32 = (metadata->n * metadata->attributeSize + Int32ByteLength - 1) / Int32ByteLength;
    mBsrSteps8  =  metadata->n * metadata->attributeSize;

    //Default mask: 0xFF bytes
    mBsrDefaultMask64 = 0xFFFFFFFFFFFFFFFF;
    mBsrDefaultMask32 = 0xFFFFFFFF;
    mBsrDefaultMask8  = 0xFF;

    //First block mask: mask for MSB, which can have empty bytes in block
    mBsrFirstZaBlockMask64 = 0xFFFFFFFFFFFFFFFF >> (Int64ByteLength - (metadata->n * metadata->attributeSize % Int64ByteLength)) * 8;
    mBsrFirstZaBlockMask32 = 0xFFFFFFFF >> (Int32ByteLength - (metadata->n * metadata->attributeSize % Int32ByteLength)) * 8;
    mBsrFirstZaBlockMask8  = 0xFF;//>> (Int8ByteLength - (metadata->n * metadata->attributeSize % Int8ByteLength)) * 8;

    //Rob and Lob: for fast algorithms, how many blocks earlier we need to look for mask reconstruction
    if(metadata->n <= (Int64ByteLength*8)){//should not happen if we use 64b Fast algorithm
        mRob64 = 1;
        mLob64 = 1;
        mRobShift64 = (Int64ByteLength*8) % metadata->n;
        mLobShift64 = metadata->n - mRobShift64;
    }
    else{
        mRob64 = metadata->n / (Int64ByteLength * 8)+1;
        mLob64 = metadata->n / (Int64ByteLength * 8);
        mLobShift64 = metadata->n % (Int64ByteLength * 8);
        mRobShift64 = (Int64ByteLength * 8) - mLobShift64;
    }
    mBsrMaskArrayL64 = new uint64_t[mBsrSteps64+mRob64];
    mBsrMaskArrayH64 = new uint64_t[mBsrSteps64+mRob64];

    if(metadata->n <= (Int32ByteLength*8)){
        mRob32 = 1;
        mLob32 = 1;
        mRobShift32 = (Int32ByteLength*8) % metadata->n;
        mLobShift32 = metadata->n - mRobShift32;
    }
    else{
        mRob32 = metadata->n / (Int32ByteLength * 8)+1;
        mLob32 = metadata->n / (Int32ByteLength * 8);
        mLobShift32 = metadata->n % (Int32ByteLength * 8);
        mRobShift32 = (Int32ByteLength * 8) - mLobShift32;
    }
    mBsrMaskArrayL32 = new uint32_t[mBsrSteps32+mRob32];
    mBsrMaskArrayH32 = new uint32_t[mBsrSteps32+mRob32];

    if(metadata->n <= (Int8ByteLength*8)){
        mRob8 = 1;
        mLob8 = 1;
        mRobShift8 = (Int8ByteLength*8) % metadata->n;
        mLobShift8 = metadata->n - mRobShift8;
    }
    else{
        mRob8 = metadata->n / (Int8ByteLength * 8)+1;
        mLob8 = metadata->n / (Int8ByteLength * 8);
        mLobShift8 = metadata->n % (Int8ByteLength * 8);
        mRobShift8 = (Int8ByteLength * 8) - mLobShift8;
    }
    mBsrMaskArrayL8 = new uint8_t[mBsrSteps8+mRob8];
    mBsrMaskArrayH8 = new uint8_t[mBsrSteps8+mRob8];

    //Shift mask array: array of masks for next block
    mBsrShiftMaskArray64 = new uint64_t[64];
    mBsrShiftMaskArray32 = new uint32_t[32];
    mBsrShiftMaskArray8 = new uint8_t[8];
    for (uint i = 0; i < 64; i++){
		unsigned int offset = i % metadata->n;
		mBsrShiftMaskArray64[i] = mBsrDefaultMask64;

		uint64_t setBit = 1;
		for (uint j = 0; j < 64; j++)
		{
			if (j % metadata->n == offset) 
                mBsrShiftMaskArray64[i] ^= setBit;
			setBit <<= 1;
		}
	}
    for (uint i = 0; i < 32; i++){
        unsigned int offset = i % metadata->n;
        mBsrShiftMaskArray32[i] = mBsrDefaultMask32;

        uint32_t setBit = 1;
        for (uint j = 0; j < 32; j++)
        {
            if (j % metadata->n == offset) 
                mBsrShiftMaskArray32[i] ^= setBit;
            setBit <<= 1;
        }
    }
    for (uint i = 0; i < 8; i++){
        unsigned int offset = i % metadata->n;
        mBsrShiftMaskArray8[i] = mBsrDefaultMask8;

        uint8_t setBit = 1;
        for (uint j = 0; j < 8; j++)
        {
            if (j % metadata->n == offset) 
                mBsrShiftMaskArray8[i] ^= setBit;
            setBit <<= 1;
        }
    }
    //Mask arrays for fast algorithms
    mMaskArrayAL64 = new uint64_t[mBsrSteps64+mRob64];
    mMaskArrayAH64 = new uint64_t[mBsrSteps64+mRob64];
    mMaskArrayAL32 = new uint32_t[mBsrSteps32+mRob32];
    mMaskArrayAH32 = new uint32_t[mBsrSteps32+mRob32];
    mMaskArrayAL8 = new uint8_t[mBsrSteps8+mRob8];
    mMaskArrayAH8 = new uint8_t[mBsrSteps8+mRob8];

    //Set function pointers based on dim number
    if(metadata->n <= 8){
        isInRectangle = &cZAddrUtils::IsInRectangle_bsr_8;
        isIntersected = &cZAddrUtils::IsIntersected_bsr_64;
        isIntersected_ZrQr_block = &cZAddrUtils::IsIntersected_ZrQr_block_bsr_32;
    }
    else if(metadata->n <= 32){
        isInRectangle = &cZAddrUtils::IsInRectangle_fast_8;
        isIntersected = &cZAddrUtils::IsIntersected_bsr_64;
        isIntersected_ZrQr_block = &cZAddrUtils::IsIntersected_ZrQr_block_bsr_32;
    }
    else if(metadata->n <= 64){
        isInRectangle = &cZAddrUtils::IsInRectangle_fast_32;
        isIntersected = &cZAddrUtils::IsIntersected_bsr_64;
        isIntersected_ZrQr_block = &cZAddrUtils::IsIntersected_ZrQr_block_fast_32;
    }
    else{
        isInRectangle = &cZAddrUtils::IsInRectangle_fast_64;
        isIntersected = &cZAddrUtils::IsIntersected_fast_64;
        isIntersected_ZrQr_block = &cZAddrUtils::IsIntersected_ZrQr_block_fast_32;
    }
}
cZAddrUtils::~cZAddrUtils(){
    delete[] mBsrMaskArrayL64;
    delete[] mBsrMaskArrayH64;
    delete[] mBsrMaskArrayL32;
    delete[] mBsrMaskArrayH32;
    delete[] mBsrMaskArrayL8;
    delete[] mBsrMaskArrayH8;
    delete[] mBsrShiftMaskArray64;
    delete[] mBsrShiftMaskArray32;
    delete[] mBsrShiftMaskArray8;
    delete[] mMaskArrayAL64;
    delete[] mMaskArrayAH64;
    delete[] mMaskArrayAL32;
    delete[] mMaskArrayAH32;
    delete[] mMaskArrayAL8;
    delete[] mMaskArrayAH8;
}
bool cZAddrUtils::IsInRectangle(char* za_t, char* za_ql, char* za_qh){
    if(isInRectangle) return (this->*isInRectangle)(za_t, za_ql, za_qh);
    else std::cerr << "No function pointer set\n";
	return false;
}
bool cZAddrUtils::IsIntersected(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2){
    if(isIntersected) return (this->*isIntersected)(za_ql1, za_qh1, za_ql2, za_qh2);
    else std::cerr << "No function pointer set\n";
	return false;
}
bool cZAddrUtils::IsIntersected_ZrQr_block(char* zi_a, char* zi_b, char* za_ql, char* za_qh){
    if(isIntersected_ZrQr_block) return (this->*isIntersected_ZrQr_block)(zi_a, zi_b, za_ql, za_qh);
    else std::cerr << "No function pointer set\n";
	return false;
}
bool cZAddrUtils::getZAddressBit(char* zAddress, std::size_t bitIndex){
	char mask = 1 << bitIndex % 8;
	uint charOrder = bitIndex / 8; // / Int8BitLength; deleno 8, bit order je absolutni pozice v z adress
	uint bit = zAddress[charOrder] & mask;
	return bit > 0;
}


#if defined(__GNUC__) || defined(__clang__) // Check for GCC or Clang
bool cZAddrUtils::BitScanReverse64asm(unsigned long* index, uint64_t mask)
{
	if (mask == 0)
		return false;
	__asm__("bsr %1, %0" : "=r" (*index) : "r" (mask)); // inline assembly to perform bsr
	return true;
}
#elif defined(_MSC_VER) // Check for MSVC
#include <intrin.h>
bool cZAddrUtils::BitScanReverse64asm(unsigned long* index, uint64_t mask)
{
	if (mask == 0)
		return false;

	unsigned long idx;
	_BitScanReverse64(&idx, mask); // Use _BitScanReverse64 intrinsic for x64
	*index = idx;

	return true;
}
#else
#error Unsupported compiler
#endif
bool cZAddrUtils::IsInRectangle_bsr_64(char* za_t, char* za_ql, char* za_qh)
{
	//za_t je pole bloku bodu, za_ql a za_qh jsou pole bloku rohu obdelniku
	//k ulozeni pozice navracene BSR
	unsigned long posL, posH;

	//nastaveni masek horni a dolni zAdresy
	uint64_t maskL = mBsrDefaultMask64;
	uint64_t maskH = mBsrDefaultMask64;
	//bsrSteps line 335
	for (int i = mBsrSteps64 - 1; i >= 0; i--)
	{
		//nacteni bloku z adresy
		uint64_t block = *((uint64_t*)(za_t)+i);
		uint64_t blockL = *((uint64_t*)(za_ql)+i);
		uint64_t blockH = *((uint64_t*)(za_qh)+i);

	    //posunuti masky
		if (mRobShift64 < 64)
		{
			maskL = maskL << mRobShift64 | maskL >> mLobShift64;
			maskH = maskH << mRobShift64 | maskH >> mLobShift64;
		}
		else
		{
			maskL = maskL >> mLobShift64;
			maskH = maskH >> mLobShift64;
		}
		//osetreni bordelu pri prvnim bloku (kdyz je treba lichy)
		if (i == mBsrSteps64 - 1 && mBsrFirstZaBlockMask64 != mBsrDefaultMask64)
		{
			block &= mBsrFirstZaBlockMask64;
			blockL &= mBsrFirstZaBlockMask64;
			blockH &= mBsrFirstZaBlockMask64;
		}
		//bsr instrukce a bitove operace
		bool b = true;
		while (b)
		{
			b = false;

			if (BitScanReverse64asm(&posL, (block ^ blockL) & maskL))
			{
				if (!(block & (uint64_t)1 << posL))
				{
					return false;
				}
				//tady bude to nastaveni v masce na vsech pozicich co nalezi bitu
				maskL &= mBsrShiftMaskArray64[posL];
				b = true;
			}

			if (BitScanReverse64asm(&posH, (block ^ blockH) & maskH))
			{
				if (block & (uint64_t)1 << posH)
				{
					return false;
				}

				maskH &= mBsrShiftMaskArray64[posH];
				b = true;
			}
		}
	}
	return true;
}
bool cZAddrUtils::IsInRectangle_fast_64(char* za_t, char* za_ql, char* za_qh)
{
	memset(mBsrMaskArrayL64 + mBsrSteps64 - 1, 0xFF, Int64ByteLength * (mRob64 + 1));
	memset(mBsrMaskArrayH64 + mBsrSteps64 - 1, 0xFF, Int64ByteLength * (mRob64 + 1));

	for (int i = mBsrSteps64 - 1; i >= 0; i--)
	{
		uint64_t block1 = *((uint64_t*)(za_t)+i);
		uint64_t block2 = *((uint64_t*)(za_t)+i);
		uint64_t blockL = *((uint64_t*)(za_ql)+i);
		uint64_t blockH = *((uint64_t*)(za_qh)+i);

		uint64_t* maskPtrL = mBsrMaskArrayL64 + i;
		uint64_t* maskPtrH = mBsrMaskArrayH64 + i;

		if (mRobShift64 < 64)
		{
			*maskPtrL = *(maskPtrL + mRob64) << mRobShift64 | *(maskPtrL + mLob64) >> mLobShift64;
			*maskPtrH = *(maskPtrH + mRob64) << mRobShift64 | *(maskPtrH + mLob64) >> mLobShift64;
		}
		else
		{
			*maskPtrL = *(maskPtrL + mLob64) >> mLobShift64;
			*maskPtrH = *(maskPtrH + mLob64) >> mLobShift64;
		}

		if (i == mBsrSteps64 - 1 && mBsrFirstZaBlockMask64 != mBsrDefaultMask64)
		{
			block1 &= mBsrFirstZaBlockMask64;
			block2 &= mBsrFirstZaBlockMask64;
			blockL &= mBsrFirstZaBlockMask64;
			blockH &= mBsrFirstZaBlockMask64;
		}

		block1 &= *maskPtrL;
		blockL &= *maskPtrL;

		if (blockL & ~block1)
		{
			return false;
		}

		block2 &= *maskPtrH;
		blockH &= *maskPtrH;

		if (block2 & ~blockH)
		{
			return false;
		}

		*maskPtrL &= ~(~blockL & block1);
		*maskPtrH &= ~(~block2 & blockH);
	}
	return true;
}
bool cZAddrUtils::IsInRectangle_bsr_32(char* za_t, char* za_ql, char* za_qh)
{
    unsigned long posL, posH;
    uint32_t maskL = mBsrDefaultMask32;
    uint32_t maskH = mBsrDefaultMask32;
    for (int i = mBsrSteps32 - 1; i >= 0; i--)
    {
        uint32_t block = *((uint32_t*)(za_t)+i);
        uint32_t blockL = *((uint32_t*)(za_ql)+i);
        uint32_t blockH = *((uint32_t*)(za_qh)+i);

        if (mRobShift32 < 32)
        {
            maskL = maskL << mRobShift32 | maskL >> mLobShift32;
            maskH = maskH << mRobShift32 | maskH >> mLobShift32;
        }
        else
        {
            maskL = maskL >> mLobShift32;
            maskH = maskH >> mLobShift32;
        }

        if (i == mBsrSteps32 - 1 && mBsrFirstZaBlockMask32 != mBsrDefaultMask32)
        {
            block &= mBsrFirstZaBlockMask32;
            blockL &= mBsrFirstZaBlockMask32;
            blockH &= mBsrFirstZaBlockMask32;
        }

        bool b = true;
        while (b)
        {
            b = false;

            if (BitScanReverse64asm(&posL, (block ^ blockL) & maskL))
            {
                if (!(block & (uint32_t)1 << posL))
                {
                    return false;
                }

                maskL &= mBsrShiftMaskArray32[posL];
                b = true;
            }

            if (BitScanReverse64asm(&posH, (block ^ blockH) & maskH))
            {
                if (block & (uint32_t)1 << posH)
                {
                    return false;
                }

                maskH &= mBsrShiftMaskArray32[posH];
                b = true;
            }
        }
    }
    return true;
}

bool cZAddrUtils::IsInRectangle_fast_32(char* za_t, char* za_ql, char* za_qh)
{
    memset(mBsrMaskArrayL32 + mBsrSteps32 - 1, 0xFF, Int32ByteLength * (mRob32 + 1));
    memset(mBsrMaskArrayH32 + mBsrSteps32 - 1, 0xFF, Int32ByteLength * (mRob32 + 1));

    for (int i = mBsrSteps32 - 1; i >= 0; i--)
    {
        uint32_t block1 = *((uint32_t*)(za_t)+i);
        uint32_t block2 = *((uint32_t*)(za_t)+i);
        uint32_t blockL = *((uint32_t*)(za_ql)+i);
        uint32_t blockH = *((uint32_t*)(za_qh)+i);

        uint32_t* maskPtrL = mBsrMaskArrayL32 + i;
        uint32_t* maskPtrH = mBsrMaskArrayH32 + i;

        if (mRobShift32 < 32)
        {
            *maskPtrL = *(maskPtrL + mRob32) << mRobShift32 | *(maskPtrL + mLob32) >> mLobShift32;
            *maskPtrH = *(maskPtrH + mRob32) << mRobShift32 | *(maskPtrH + mLob32) >> mLobShift32;
        }
        else
        {
            *maskPtrL = *(maskPtrL + mLob32) >> mLobShift32;
            *maskPtrH = *(maskPtrH + mLob32) >> mLobShift32;
        }

        if (i == mBsrSteps32 - 1 && mBsrFirstZaBlockMask32 != mBsrDefaultMask32)
        {
            block1 &= mBsrFirstZaBlockMask32;
            block2 &= mBsrFirstZaBlockMask32;
            blockL &= mBsrFirstZaBlockMask32;
            blockH &= mBsrFirstZaBlockMask32;
        }

        block1 &= *maskPtrL;
		blockL &= *maskPtrL;

		if (blockL & ~block1)
		{
			return false;
		}

		block2 &= *maskPtrH;
		blockH &= *maskPtrH;

		if (block2 & ~blockH)
		{
			return false;
		}

		*maskPtrL &= ~(~blockL & block1);
		*maskPtrH &= ~(~block2 & blockH);
	}
	return true;
}
bool cZAddrUtils::IsInRectangle_bsr_8(char* za_t, char* za_ql, char* za_qh)
{
    unsigned long posL, posH;
    uint8_t maskL = mBsrDefaultMask8;
    uint8_t maskH = mBsrDefaultMask8;
    for (int i = mBsrSteps8 - 1; i >= 0; i--)
    {
        uint8_t block = *((uint8_t*)(za_t)+i);
        uint8_t blockL = *((uint8_t*)(za_ql)+i);
        uint8_t blockH = *((uint8_t*)(za_qh)+i);

        if (mRobShift8 < 8)
        {
            maskL = maskL << mRobShift8 | maskL >> mLobShift8;
            maskH = maskH << mRobShift8 | maskH >> mLobShift8;
        }
        else
        {
            maskL = maskL >> mLobShift8;
            maskH = maskH >> mLobShift8;
        }

        if (i == mBsrSteps8 - 1 && mBsrFirstZaBlockMask8 != mBsrDefaultMask8)
        {
            block &= mBsrFirstZaBlockMask8;
            blockL &= mBsrFirstZaBlockMask8;
            blockH &= mBsrFirstZaBlockMask8;
        }

        bool b = true;
        while (b)
        {
            b = false;

            if (BitScanReverse64asm(&posL, (block ^ blockL) & maskL))
            {
                if (!(block & (uint8_t)1 << posL))
                {
                    return false;
                }

                maskL &= mBsrShiftMaskArray8[posL];
                b = true;
            }

            if (BitScanReverse64asm(&posH, (block ^ blockH) & maskH))
            {
                if (block & (uint8_t)1 << posH)
                {
                    return false;
                }

                maskH &= mBsrShiftMaskArray8[posH];
                b = true;
            }
        }
    }
    return true;
}
bool cZAddrUtils::IsInRectangle_fast_8(char* za_t, char* za_ql, char* za_qh)
{
    memset(mBsrMaskArrayL8 + mBsrSteps8 - 1, 0xFF, Int8ByteLength * (mRob8 + 1));
    memset(mBsrMaskArrayH8 + mBsrSteps8 - 1, 0xFF, Int8ByteLength * (mRob8 + 1));

    for (int i = mBsrSteps8 - 1; i >= 0; i--)
    {
        uint8_t block1 = *((uint8_t*)(za_t)+i);
        uint8_t block2 = *((uint8_t*)(za_t)+i);
        uint8_t blockL = *((uint8_t*)(za_ql)+i);
        uint8_t blockH = *((uint8_t*)(za_qh)+i);

        uint8_t* maskPtrL = mBsrMaskArrayL8 + i;
        uint8_t* maskPtrH = mBsrMaskArrayH8 + i;

        if (mRobShift8 < 8)
        {
            *maskPtrL = *(maskPtrL + mRob8) << mRobShift8 | *(maskPtrL + mLob8) >> mLobShift8;
            *maskPtrH = *(maskPtrH + mRob8) << mRobShift8 | *(maskPtrH + mLob8) >> mLobShift8;
        }
        else
        {
            *maskPtrL = *(maskPtrL + mLob8) >> mLobShift8;
            *maskPtrH = *(maskPtrH + mLob8) >> mLobShift8;
        }

        if (i == mBsrSteps8 - 1 && mBsrFirstZaBlockMask8 != mBsrDefaultMask8)
        {
            block1 &= mBsrFirstZaBlockMask8;
            block2 &= mBsrFirstZaBlockMask8;
            blockL &= mBsrFirstZaBlockMask8;
            blockH &= mBsrFirstZaBlockMask8;
        }

        block1 &= *maskPtrL;
		blockL &= *maskPtrL;

		if (blockL & ~block1)
		{
			return false;
		}

		block2 &= *maskPtrH;
		blockH &= *maskPtrH;

		if (block2 & ~blockH)
		{
			return false;
		}

		*maskPtrL &= ~(~blockL & block1);
		*maskPtrH &= ~(~block2 & blockH);
	}

	return true;
}

bool cZAddrUtils::IsIntersected_bsr_64(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2)
{
    unsigned long pos1, pos2;

    uint64_t mask1 = mBsrDefaultMask64;
    uint64_t mask2 = mBsrDefaultMask64;

    for (int i = mBsrSteps64 - 1; i >= 0; i--)
    {
        uint64_t blockL1 = *((uint64_t*)(za_ql1)+i);
        uint64_t blockH1 = *((uint64_t*)(za_qh1)+i);
        uint64_t blockL2 = *((uint64_t*)(za_ql2)+i);
        uint64_t blockH2 = *((uint64_t*)(za_qh2)+i);

        if (mRobShift64 < 64)
        {
            mask1 = mask1 << mRobShift64 | mask1 >> mLobShift64;
            mask2 = mask2 << mRobShift64 | mask2 >> mLobShift64;
        }
        else
        {
            mask1 = mask1 >> mLobShift64;
            mask2 = mask2 >> mLobShift64;
        }

        if (i == mBsrSteps64 - 1 && mBsrFirstZaBlockMask64 != mBsrDefaultMask64)
        {
            blockL1 &= mBsrFirstZaBlockMask64;
            blockH1 &= mBsrFirstZaBlockMask64;
            blockL2 &= mBsrFirstZaBlockMask64;
            blockH2 &= mBsrFirstZaBlockMask64;
        }

        bool b = true;
        while (b)
        {
            b = false;

            if (BitScanReverse64asm(&pos1, (blockL1 ^ blockH2) & mask1))
			{
				if (!(blockH2 & (uint64_t)1 << pos1))
				{
					return false;
				}

				mask1 &= mBsrShiftMaskArray64[pos1];
				b = true;
			}

			if (BitScanReverse64asm(&pos2, (blockL2 ^ blockH1) & mask2))
			{
				if (blockL2 & (uint64_t)1 << pos2)
				{
					return false;
				}

				mask2 &= mBsrShiftMaskArray64[pos2];
				b = true;
			}
        }
    }
    return true;
}
bool cZAddrUtils::IsIntersected_fast_64(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2)
{
    memset(mBsrMaskArrayL64 + mBsrSteps64 - 1, 0xFF, Int64ByteLength * (mRob64 + 1));
	memset(mBsrMaskArrayH64 + mBsrSteps64 - 1, 0xFF, Int64ByteLength * (mRob64 + 1));

	for (int i = mBsrSteps64 - 1; i >= 0; i--)
	{
		uint64_t blockL1 = *((uint64_t*)(za_ql1)+i);
		uint64_t blockH1 = *((uint64_t*)(za_qh1)+i);
		uint64_t blockL2 = *((uint64_t*)(za_ql2)+i);
		uint64_t blockH2 = *((uint64_t*)(za_qh2)+i);

		uint64_t* maskPtr1 = mBsrMaskArrayL64 + i;
		uint64_t* maskPtr2 = mBsrMaskArrayH64 + i;

		if (mRobShift64 < 64)
		{
			*maskPtr1 = *(maskPtr1 + mRob64) << mRobShift64 | *(maskPtr1 + mLob64) >> mLobShift64;
			*maskPtr2 = *(maskPtr2 + mRob64) << mRobShift64 | *(maskPtr2 + mLob64) >> mLobShift64;
		}
		else
		{
			*maskPtr1 = *(maskPtr1 + mLob64) >> mLobShift64;
			*maskPtr2 = *(maskPtr2 + mLob64) >> mLobShift64;
		}

		if (i == mBsrSteps64 - 1 && mBsrFirstZaBlockMask64 != mBsrDefaultMask64)
		{
			blockL1 &= mBsrFirstZaBlockMask64;
			blockH1 &= mBsrFirstZaBlockMask64;
			blockL2 &= mBsrFirstZaBlockMask64;
			blockH2 &= mBsrFirstZaBlockMask64;
		}
   
		blockL1 &= *maskPtr1;
		blockH2 &= *maskPtr1;

		if (blockL1 & ~blockH2)
		{
			return false;
		}

		blockL2 &= *maskPtr2;
		blockH1 &= *maskPtr2;

		if (blockL2 & ~blockH1)
		{
			return false;
		}

		*maskPtr1 &= ~(~blockL1 & blockH2);
		*maskPtr2 &= ~(~blockL2 & blockH1);
	}

	return true;
}
void DetectFirstDiffBit(uint blockOrder, uint32_t a, uint32_t b, uint32_t &diffABBlockMask, unsigned long & bo)
{
	if (a != b)
	{
		uint32_t xor_a_b = a ^ b;
		cZAddrUtils::BitScanReverse64asm(&bo, xor_a_b);

		diffABBlockMask = ((uint64_t)1 << (bo))-1;
	}
}
void DetectFirstDiffBit64(uint blockOrder, uint64_t a, uint64_t b, uint64_t &diffABBlockMask, unsigned long & bo)
{
	if (a != b)
	{
		uint64_t xor_a_b = a ^ b;
		cZAddrUtils::BitScanReverse64asm(&bo, xor_a_b);

		diffABBlockMask = ((uint64_t)1 << (bo))-1;
	}
}
bool cZAddrUtils::IsIntersected_ZrQr_block_bsr_32(char* zi_a, char* zi_b, char* za_ql, char* za_qh){
    // Type-casted arrays.
	uint* zi_a_ui = (uint*)zi_a;
	uint* zi_b_ui = (uint*)zi_b;
	uint* za_ql_ui = (uint*)za_ql;
	uint* za_qh_ui = (uint*)za_qh;

	bool aFalse = false;
	bool bFalse = false;

	unsigned long pos;
	int diffStep = -1;

	uint diffABBlockMask;
	uint diffABBlockMask1;

	unsigned long diffPos;
	uint firstDiffMask;

	uint maskH = mBsrDefaultMask32;
	uint maskL = mBsrDefaultMask32;

	uint half_a_block;
	uint half_b_block;

	// Tento jednoduchy test sice pomaha, ale nejsem si jisty, jestli to neni spis charakterem dotazu.
	int firstDiffStep = -1;
	for (int step = mBsrSteps32 - 1; step >= 0; step--)
	{
		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block = za_ql_ui[step];
		uint qh_block = za_qh_ui[step];

		if (a_block != b_block || b_block != ql_block || ql_block != qh_block)
		{
			firstDiffStep = step;
			break;
		}
	}
	if (firstDiffStep == -1)
	{
		return true;
	}

	for (int step = firstDiffStep; step >= 0; step--)
	{
		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block = za_ql_ui[step];
		uint qh_block = za_qh_ui[step];
        if (step == mBsrSteps32 - 1 && mBsrFirstZaBlockMask32 != mBsrDefaultMask32)
        {
            a_block &= mBsrFirstZaBlockMask32;
            b_block &= mBsrFirstZaBlockMask32;
            ql_block &= mBsrFirstZaBlockMask32;
            qh_block &= mBsrFirstZaBlockMask32;
        }

		if (a_block != b_block)
		{
			diffStep = step;
			DetectFirstDiffBit(step, a_block, b_block, diffABBlockMask, diffPos);
			diffABBlockMask1 = (diffABBlockMask << 1) + 1; 
			uint diffBitMask = 1 << diffPos; 

			half_a_block = a_block | diffABBlockMask; //...11111111101111111
			half_b_block = half_a_block + 1;//...11111111110000000
			//zacatek is in rectangle, který pořád běží nad společným prefixem (znegovana maska diff1)
			while (BitScanReverse64asm(&pos, (ql_block ^ a_block) & maskL & ~diffABBlockMask1))
			{
				if (ql_block & (uint)1 << pos)
				{
					return false;
				}
				else
				{
					maskL &= mBsrShiftMaskArray32[pos];
				}
			}

			while (BitScanReverse64asm(&pos, (qh_block ^ a_block) & maskH & ~diffABBlockMask1))
			{
				if (a_block & (uint)1 << pos)
				{
					return false;
				}
				else
				{
					maskH &= mBsrShiftMaskArray32[pos];
				}
			}
			//konec is in rectangle
			if (~ql_block & maskL & diffBitMask)
			{
				maskL &= mBsrShiftMaskArray32[diffPos];
			}

			if (ql_block & maskL & diffBitMask)
			{
				aFalse = true;
			}

			if (qh_block & maskH & diffBitMask)
			{
				maskH &= mBsrShiftMaskArray32[diffPos];
			}

			if (~qh_block & maskH & diffBitMask)
			{
				bFalse = true;
			}

			break;
		}
		//is in rectangle
		while (BitScanReverse64asm(&pos, (ql_block ^ a_block) & maskL))
		{
			if ((uint)1 << pos & ql_block)
			{
				return false;
			}
			else
			{
				maskL &= mBsrShiftMaskArray32[pos];
			}
		}

		while (BitScanReverse64asm(&pos, (qh_block ^ a_block) & maskH))
		{
			if ((uint)1 << pos & a_block)
			{
				return false;
			}
			else
			{
				maskH &= mBsrShiftMaskArray32[pos];
			}
		}

		maskH = maskH << mRobShift32 | maskH >> mLobShift32;
		maskL = maskL << mRobShift32 | maskL >> mLobShift32;
		//konec is in rectangle
	}

	for (int step = diffStep; step >= 0; step--)
	{
		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block_a = za_ql_ui[step];
		uint qh_block_b = za_qh_ui[step];

        if (step == mBsrSteps32 - 1 && mBsrFirstZaBlockMask32 != mBsrDefaultMask32)
        {
            a_block &= mBsrFirstZaBlockMask32;
            b_block &= mBsrFirstZaBlockMask32;
            ql_block_a &= mBsrFirstZaBlockMask32;
            qh_block_b &= mBsrFirstZaBlockMask32;
        }

		uint qh_block_a;
		uint ql_block_b;

		if (step == diffStep)
		{
			qh_block_a = qh_block_b & maskH | half_a_block & ~maskH;
			ql_block_b = ql_block_a & maskL | half_b_block & ~maskL;
		}
		else
		{
			qh_block_a = qh_block_b | ~maskH;
			ql_block_b = ql_block_a & maskL;
		}

		if (!aFalse)
		{
			if (qh_block_a < a_block)
			{
				aFalse = true;
			}
			else if (qh_block_a > a_block)
			{
				return true;
			}
		}
		if (!bFalse)
		{
			if (ql_block_b > b_block)
			{
				bFalse = true;
			}
			else if (ql_block_b < b_block)
			{
				return true;
			}
		}

		if (aFalse && bFalse)
		{
			return false;
		}

		maskH = maskH << mRobShift32 | maskH >> mLobShift32;
		maskL = maskL << mRobShift32 | maskL >> mLobShift32;
	}

	return true;
}
bool cZAddrUtils::IsIntersected_ZrQr_block_fast_32(char* zi_a, char* zi_b, char* za_ql, char* za_qh)
{
	// Type-casted arrays.
	uint* zi_a_ui = (uint*)zi_a;
	uint* zi_b_ui = (uint*)zi_b;
	uint* za_ql_ui = (uint*)za_ql;
	uint* za_qh_ui = (uint*)za_qh;

	bool aFalse = false;
	bool bFalse = false;

	unsigned long pos;
	int diffStep = -1;

	uint diffABBlockMask;
	uint diffABBlockMask1;

	unsigned long diffPos;
	uint firstDiffMask;

	uint half_a_block;
	uint half_b_block;

	// Tento jednoduchy test sice pomaha, ale nejsem si jisty, jestli to neni spis charakterem dotazu.
    int firstDiffStep = -1;
	for (int step = mBsrSteps32 - 1; step >= 0; step--)
	{
		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block = za_ql_ui[step];
		uint qh_block = za_qh_ui[step];

		if (a_block != b_block || b_block != ql_block || ql_block != qh_block)
		{
			firstDiffStep = step;
			break;
		}
	}
	if (firstDiffStep == -1)
	{
		return true;
	}
    

	memset(mMaskArrayAL32 + firstDiffStep, 0xFF, Int32ByteLength * (mRob32 + 1));
	memset(mMaskArrayAH32 + firstDiffStep, 0xFF, Int32ByteLength * (mRob32 + 1));
	uint* maskL = mMaskArrayAL32 + firstDiffStep;
	uint* maskH = mMaskArrayAH32 + firstDiffStep;


	for (int step = firstDiffStep; step >= 0; step--)
	{
		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block = za_ql_ui[step];
		uint qh_block = za_qh_ui[step];

        if (step == mBsrSteps32 - 1 && mBsrFirstZaBlockMask32 != mBsrDefaultMask32)
        {
            a_block &= mBsrFirstZaBlockMask32;
            b_block &= mBsrFirstZaBlockMask32;
            ql_block &= mBsrFirstZaBlockMask32;
            qh_block &= mBsrFirstZaBlockMask32;
        }

		if (a_block != b_block)
		{
			diffStep = step;
			DetectFirstDiffBit(step, a_block, b_block, diffABBlockMask, diffPos);
			diffABBlockMask1 = (diffABBlockMask << 1) + 1;
			uint diffBitMask = 1 << diffPos;

			half_a_block = a_block | diffABBlockMask;
			half_b_block = half_a_block + 1;


			if (ql_block & ~a_block & *maskL & ~diffABBlockMask1)
			{
				return false;
			}

			if (~qh_block & a_block & *maskH & ~diffABBlockMask1)
			{
				return false;
			}

			*maskL &= ~(~ql_block & a_block) | diffABBlockMask1;
			*maskH &= ~(qh_block & ~a_block) | diffABBlockMask1;


			if (~ql_block & *maskL & diffBitMask)
			{
				*maskL &= ~diffBitMask;
			}

			if (ql_block & *maskL & diffBitMask)
			{
				aFalse = true;
			}

			if (qh_block & *maskH & diffBitMask)
			{
				*maskH &= ~diffBitMask;
			}

			if (~qh_block & *maskH & diffBitMask)
			{
				bFalse = true;
			}

			break;
		}

		if (ql_block & ~a_block & *maskL)
		{
			return false;
		}

		if (~qh_block & a_block & *maskH)
		{
			return false;
		}

		*maskL &= ~(~ql_block & a_block);
		*maskH &= ~(qh_block & ~a_block);

		if (step > 0)
		{
			maskL--;
			maskH--;

			if (mRobShift32 < 32)
			{
				*maskL = *(maskL + mRob32) << mRobShift32 | *(maskL + mLob32) >> mLobShift32;
				*maskH = *(maskH + mRob32) << mRobShift32 | *(maskH + mLob32) >> mLobShift32;
			}
			else
			{
				*maskL = *(maskL + mLob32) >> mLobShift32;
				*maskH = *(maskH + mLob32) >> mLobShift32;
			}
		}
	}

	for (int step = diffStep; step >= 0; step--)
	{
		maskL = mMaskArrayAL32 + step;
		maskH = mMaskArrayAH32 + step;

		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block_a = za_ql_ui[step];
		uint qh_block_b = za_qh_ui[step];

        if (step == mBsrSteps32 - 1 && mBsrFirstZaBlockMask32 != mBsrDefaultMask32)
        {
            a_block &= mBsrFirstZaBlockMask32;
            b_block &= mBsrFirstZaBlockMask32;
            ql_block_a &= mBsrFirstZaBlockMask32;
            qh_block_b &= mBsrFirstZaBlockMask32;
        }

		uint qh_block_a;
		uint ql_block_b;

		if (step == diffStep)
		{
			qh_block_a = qh_block_b & *maskH | half_a_block & ~*maskH;
			ql_block_b = ql_block_a & *maskL | half_b_block & ~*maskL;
		}
		else
		{
			qh_block_a = qh_block_b | ~*maskH;
			ql_block_b = ql_block_a & *maskL;
		}

		if (!aFalse)
		{
			if (qh_block_a < a_block)
			{
				aFalse = true;
			}
			else if (qh_block_a > a_block)
			{
				return true;
			}
		}

		if (!bFalse)
		{
			if (ql_block_b > b_block)
			{
				bFalse = true;
			}
			else if (ql_block_b < b_block)
			{
				return true;
			}
		}

		if (aFalse && bFalse)
		{
			return false;
		}

		if (step > 0)
		{
			maskL--;
			maskH--;

			if (mRobShift32 < 32)
			{
				*maskL = *(maskL + mRob32) << mRobShift32 | *(maskL + mLob32) >> mLobShift32;
				*maskH = *(maskH + mRob32) << mRobShift32 | *(maskH + mLob32) >> mLobShift32;
			}
			else
			{
				*maskL = *(maskL + mLob32) >> mLobShift32;
				*maskH = *(maskH + mLob32) >> mLobShift32;
			}
		}
	}

	return true;
}
//Following comparators must compare from MSB to LSB
bool cZAddrUtils::isZAddrEQ(char* za1, char* za2){
	u_int64_t* za1_ui = (u_int64_t*)za1;
	u_int64_t* za2_ui = (u_int64_t*)za2;
	int steps = mBsrSteps64-1;
	if(mBsrFirstZaBlockMask64 != mBsrDefaultMask64){
		if((za1_ui[steps] & mBsrFirstZaBlockMask64) != (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return false;
		steps--;
	}
	for (int i = steps; i >= 0; i--){//check the rest if there is any
		if(za1_ui[i] != za2_ui[i])
			return false;
	}
	return true;
}
bool cZAddrUtils::isZAddrLT(char* za1, char* za2){
	u_int64_t* za1_ui = (u_int64_t*)za1;
	u_int64_t* za2_ui = (u_int64_t*)za2;
	int steps = mBsrSteps64-1;
	if(mBsrFirstZaBlockMask64 != mBsrDefaultMask64){
		if((za1_ui[steps] & mBsrFirstZaBlockMask64) < (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return true;
		else if((za1_ui[steps] & mBsrFirstZaBlockMask64) > (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return false;
		steps--;
	}
	for (int i = steps; i >= 0; i--){//check the rest if there is any
		if(za1_ui[i] < za2_ui[i])
			return true;
		else if(za1_ui[i] > za2_ui[i])
			return false;
	}
	return false;
}
bool cZAddrUtils::isZAddrLEQT(char* za1, char* za2){
	u_int64_t* za1_ui = (u_int64_t*)za1;
	u_int64_t* za2_ui = (u_int64_t*)za2;
	int steps = mBsrSteps64-1;
	if(mBsrFirstZaBlockMask64 != mBsrDefaultMask64){
		if((za1_ui[steps] & mBsrFirstZaBlockMask64) < (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return true;
		else if((za1_ui[steps] & mBsrFirstZaBlockMask64) > (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return false;
		steps--;
	}
	for (int i = steps; i >= 0; i--){//check the rest if there is any
		if(za1_ui[i] < za2_ui[i])
			return true;
		else if(za1_ui[i] > za2_ui[i])
			return false;
	}
	return true;
}
bool cZAddrUtils::isZAddrGT(char* za1, char* za2){
	u_int64_t* za1_ui = (u_int64_t*)za1;
	u_int64_t* za2_ui = (u_int64_t*)za2;
	int steps = mBsrSteps64-1;
	if(mBsrFirstZaBlockMask64 != mBsrDefaultMask64){
		if((za1_ui[steps] & mBsrFirstZaBlockMask64) > (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return true;
		else if((za1_ui[steps] & mBsrFirstZaBlockMask64) < (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return false;
		steps--;
	}
	for (int i = steps; i >= 0; i--){//check the rest if there is any
		if(za1_ui[i] > za2_ui[i])
			return true;
		else if(za1_ui[i] < za2_ui[i])
			return false;
	}
	return false;
}
bool cZAddrUtils::isZAddrGEQT(char* za1, char* za2){
	u_int64_t* za1_ui = (u_int64_t*)za1;
	u_int64_t* za2_ui = (u_int64_t*)za2;
	int steps = mBsrSteps64-1;
	if(mBsrFirstZaBlockMask64 != mBsrDefaultMask64){
		if((za1_ui[steps] & mBsrFirstZaBlockMask64) > (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return true;
		else if((za1_ui[steps] & mBsrFirstZaBlockMask64) < (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return false;
		steps--;
	}
	for (int i = steps; i >= 0; i--){//check the rest if there is any
		if(za1_ui[i] > za2_ui[i])
			return true;
		else if(za1_ui[i] < za2_ui[i])
			return false;
	}
	return true;
}
//return -1 if LT, 0 if EQ, 1 if GT
int cZAddrUtils::cmpZAddress(char* za1, char* za2){
	u_int64_t* za1_ui = (u_int64_t*)za1;
	u_int64_t* za2_ui = (u_int64_t*)za2;
	int steps = mBsrSteps64-1;
	if(mBsrFirstZaBlockMask64 != mBsrDefaultMask64){
		if((za1_ui[steps] & mBsrFirstZaBlockMask64) > (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return 1;
		else if((za1_ui[steps] & mBsrFirstZaBlockMask64) < (za2_ui[steps] & mBsrFirstZaBlockMask64))
			return -1;
		steps--;
	}
	for (int i = steps; i >= 0; i--){//check the rest if there is any
		if(za1_ui[i] > za2_ui[i])
			return 1;
		else if(za1_ui[i] < za2_ui[i])
			return -1;
	}
	return 0;
}
//Class extension
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
    memset(data, 0, metadata->zAddressBytes);

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