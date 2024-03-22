#include "ZAddressTools.h"
#include <bitset>
#include <climits>
#include <cstring>
#include <iostream>
//#include <intrin.h>


/*
TODO:
Upravit konstruktor aby počítal s jinou velikostí datového typu než int32 (t5eba bsr steps)
Upravit pro to samé i algoritmy


Dodelat fastovy algoritmus, sjednotit velikosti bloku, dodelat testing rozsahu, dodelat text a pokud vyjde udelat grafy z testovani

*/
ZAddressTools::ZAddressTools(){
    zAddressBytes = 0;
	zAddressBits = 0;
    dataSize = 0;
    dataTypeBytes = 4;
    dataTypeBits = 8*dataTypeBytes;
}
ZAddressTools::ZAddressTools(std::size_t dimensionByteSize, std::size_t dimensionCount){
    zAddressBytes = dimensionByteSize*dimensionCount;
	zAddressBits = 8*zAddressBytes;
    dataSize = dimensionCount;
    dataTypeBytes = dimensionByteSize;
    dataTypeBits = 8*dataTypeBytes;

	// --- BSR Algorithm ---

	// Ceiling of (d * Int32ByteLength) / Int64ByteLength.
	// Kdybychom měli 3 dimenze 32b, tak 3 * 4 = 12, což je 2 Int64. 
	// (3*4 + 8 - 1)/ 8 = 2	  12B se s 4 nulovými byty vleze do 2 Int64
	// (8*1 +8 -1)/8 = 1      8B se akorát vleze do 1 Int64
	// (9*1 +8 -1)/8 = 2      9B se s 7 nulovými byty vleze do 2 Int64
	mBsrSteps = (dataSize * Int32ByteLength + Int64ByteLength - 1) / Int64ByteLength;
	// Default mask of all ones.
	memset((char*)&mBsrDefaultMask, 0xFF, Int64ByteLength);

	// Prepare a bit mask for the first (most significant) compared block of UInt64.
	mBsrFirsTZaBlockMask = mBsrDefaultMask;
	uint zeroBytes = mBsrSteps * Int64ByteLength - dataSize * Int32ByteLength;
	memset((char*)&mBsrFirsTZaBlockMask + Int64ByteLength - zeroBytes, 0x00, zeroBytes);

	if (dataSize <= Int64BitLength)
	{
		mBsrRob = 1;
		mBsrLob = 1;
		mBsrRobShift = Int64BitLength % dataSize;
		mBsrLobShift = dataSize - mBsrRobShift;
	}
	else
	{
		mBsrRob = dataSize / Int64BitLength + 1;
		mBsrLob = dataSize / Int64BitLength;
		mBsrLobShift = dataSize % Int64BitLength;
		mBsrRobShift = Int64BitLength - mBsrLobShift;
	}

	mBsrMaskArrayL = new uint64_t[mBsrSteps + mBsrRob];
	mBsrMaskArrayH = new uint64_t[mBsrSteps + mBsrRob];

	//Predpripravene pole masek pro bity
	mBsrShiftMaskArray = new uint64_t[Int64BitLength];
	for (uint i = 0; i < Int64BitLength; i++)
	{
		unsigned int offset = i % dataSize;
		mBsrShiftMaskArray[i] = mBsrDefaultMask;

		uint64_t setBit = 1;
		for (uint j = 0; j < Int64BitLength; j++)
		{
			if (j % dataSize == offset)
			{
				mBsrShiftMaskArray[i] ^= setBit;
			}

			setBit <<= 1;
		}
	}
	//32 bsr
	mBsrSteps32 = (dataSize * Int32ByteLength + Int32ByteLength - 1) / Int32ByteLength;
	// Default mask of all ones.
	memset((char*)&mBsrDefaultMask32, 0xFF, Int32ByteLength);

	if (dataSize <= Int32BitLength)
	{
		mBsrRob32 = 1;
		mBsrLob32 = 1;
		mBsrRobShift32 = Int32BitLength % dataSize;
		mBsrLobShift32 = dataSize - mBsrRobShift32;
	}
	else
	{
		mBsrRob32 = dataSize / Int32BitLength + 1;
		mBsrLob32 = dataSize / Int32BitLength;
		mBsrLobShift32 = dataSize % Int32BitLength;
		mBsrRobShift32 = Int32BitLength - mBsrLobShift32;
	}

	mMaskArrayAL32 = new uint[mBsrSteps32 + mBsrRob32];
	mMaskArrayAH32 = new uint[mBsrSteps32 + mBsrRob32];
	mMaskArrayAL = new uint64_t[mBsrSteps + mBsrRob];
	mMaskArrayAH = new uint64_t[mBsrSteps + mBsrRob];
	mBsrShiftMaskArray32 = new uint32_t[Int32BitLength];
	for (uint i = 0; i < Int32BitLength; i++)
	{
		unsigned int offset = i % dataSize;
		mBsrShiftMaskArray32[i] = mBsrDefaultMask32;

		uint32_t setBit = 1;
		for (uint j = 0; j < Int32BitLength; j++)
		{
			if (j % dataSize == offset)
			{
				mBsrShiftMaskArray32[i] ^= setBit;
			}

			setBit <<= 1;
		}
	}
}
ZAddressTools::~ZAddressTools(){
	delete[] mBsrMaskArrayL;
	delete[] mBsrMaskArrayH;
	delete[] mBsrShiftMaskArray;
	delete[] mMaskArrayAL32;
	delete[] mMaskArrayAH32;
	delete[] mMaskArrayAL;
	delete[] mMaskArrayAH;
	delete[] mBsrShiftMaskArray32;
}
bool ZAddressTools::getZAddressBit(char* zAddress, std::size_t bitIndex){
	char mask = 1 << bitIndex % 8;
	uint charOrder = bitIndex / 8; // / Int8BitLength; deleno 8, bit order je absolutni pozice v z adress
	uint bit = zAddress[charOrder] & mask;
	return bit > 0;
}

void ZAddressTools::transformDataToZAddress(char* data, char*& za){
    if(za == nullptr)
        za = new char[zAddressBytes];
    uint* p_t_uint32 = (uint*)data;
	uint bitOrder = 0;
	uint* p_za_uint32 = (uint*)za;
	memset(za, 0, zAddressBytes);

	for (uint i = 0; i < Int32BitLength; i++)
	{
		uint mask = 1 << i;
		for (uint j = 0; j < dataSize; j++, bitOrder++)
		{
			uint bit = p_t_uint32[j] & mask;
			if (bit > 0)
			{
				uint int32Order = bitOrder >> 5; // / Int32BitLength; deleno 32, bit order je absolutni pozice v z adress
				uint shift = bitOrder % Int32BitLength; //bit order v ramci bloku
				p_za_uint32[int32Order] |= (1 << shift);
			}
		}
	}
}

void ZAddressTools::print(){
    std::cout << "ZAddressTools: " << std::endl;
    std::cout << "__zAddressBytes: " << zAddressBytes << std::endl;
    std::cout << "__dataSize: " << dataSize << std::endl;
    std::cout << "__dataTypeBytes: " << dataTypeBytes << std::endl;
    std::cout << "__dataTypeBits: " << dataTypeBits << std::endl;
	std::cout << "__mBsrSteps: " << mBsrSteps << std::endl;
	std::cout << "__mBsrDefaultMask: " << std::bitset<64>(mBsrDefaultMask) << std::endl;
	std::cout << "__mBsrFirsTZaBlockMask: " << std::bitset<64>(mBsrFirsTZaBlockMask) << std::endl;
	std::cout << "__mBsrRob: " << mBsrRob << std::endl;
	std::cout << "__mBsrLob: " << mBsrLob << std::endl;
	std::cout << "__mBsrRobShift: " << mBsrRobShift << std::endl;
	std::cout << "__mBsrLobShift: " << mBsrLobShift << std::endl;
	
}

bool ZAddressTools::IsInRectangle_bsr64(char* za_t, char* za_ql, char* za_qh)
{
	//Pripravit framework, naimplementovat zakladni euklid, zakodovani do z adresy, generovani tuplu co vyhovuji, nevyhovuji

	//zkusit kouknout na makra a templaty

	//za_t je pole bloku bodu, za_ql a za_qh jsou pole bloku rohu obdelniku
	//k ulozeni pozice navracene BSR
	unsigned long posL, posH;

	//nastaveni masek horni a dolni zAdresy
	uint64_t maskL = mBsrDefaultMask;
	uint64_t maskH = mBsrDefaultMask;
	//bsrSteps line 335
	for (int i = mBsrSteps - 1; i >= 0; i--)
	{
		//nacteni bloku z adresy
		uint64_t block = *((uint64_t*)(za_t)+i);
		uint64_t blockL = *((uint64_t*)(za_ql)+i);
		uint64_t blockH = *((uint64_t*)(za_qh)+i);

	    //posunuti masky
		if (mBsrRobShift < Int64BitLength)
		{
			maskL = maskL << mBsrRobShift | maskL >> mBsrLobShift;
			maskH = maskH << mBsrRobShift | maskH >> mBsrLobShift;
		}
		else
		{
			maskL = maskL >> mBsrLobShift;
			maskH = maskH >> mBsrLobShift;
		}
		//osetreni bordelu pri prvnim bloku (kdyz je treba lichy)
		if (i == mBsrSteps - 1 && mBsrFirsTZaBlockMask != mBsrDefaultMask)
		{
			block &= mBsrFirsTZaBlockMask;
			blockL &= mBsrFirsTZaBlockMask;
			blockH &= mBsrFirsTZaBlockMask;
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
				maskL &= mBsrShiftMaskArray[posL];
				b = true;
			}

			if (BitScanReverse64asm(&posH, (block ^ blockH) & maskH))
			{
				if (block & (uint64_t)1 << posH)
				{
					return false;
				}

				maskH &= mBsrShiftMaskArray[posH];
				b = true;
			}
		}
	}

	return true;
}
bool ZAddressTools::IsInRectangle_fast64(char* za_t, char* za_ql, char* za_qh)
{
	memset(mBsrMaskArrayL + mBsrSteps - 1, 0xFF, Int64ByteLength * (mBsrRob + 1));
	memset(mBsrMaskArrayH + mBsrSteps - 1, 0xFF, Int64ByteLength * (mBsrRob + 1));

	for (int i = mBsrSteps - 1; i >= 0; i--)
	{
		uint64_t block1 = *((uint64_t*)(za_t)+i);
		uint64_t block2 = *((uint64_t*)(za_t)+i);
		uint64_t blockL = *((uint64_t*)(za_ql)+i);
		uint64_t blockH = *((uint64_t*)(za_qh)+i);

		uint64_t* maskPtrL = mBsrMaskArrayL + i;
		uint64_t* maskPtrH = mBsrMaskArrayH + i;

		if (mBsrRobShift < Int64BitLength)
		{
			*maskPtrL = *(maskPtrL + mBsrRob) << mBsrRobShift | *(maskPtrL + mBsrLob) >> mBsrLobShift;
			*maskPtrH = *(maskPtrH + mBsrRob) << mBsrRobShift | *(maskPtrH + mBsrLob) >> mBsrLobShift;
		}
		else
		{
			*maskPtrL = *(maskPtrL + mBsrLob) >> mBsrLobShift;
			*maskPtrH = *(maskPtrH + mBsrLob) >> mBsrLobShift;
		}

		if (i == mBsrSteps - 1 && mBsrFirsTZaBlockMask != mBsrDefaultMask)
		{
			block1 &= mBsrFirsTZaBlockMask;
			block2 &= mBsrFirsTZaBlockMask;
			blockL &= mBsrFirsTZaBlockMask;
			blockH &= mBsrFirsTZaBlockMask;
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
bool ZAddressTools::IsIntersected_bsr64(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2)
{
	unsigned long pos1, pos2;

	uint64_t mask1 = mBsrDefaultMask;
	uint64_t mask2 = mBsrDefaultMask;

	for (int i = mBsrSteps - 1; i >= 0; i--)
	{
		uint64_t blockL1 = *((uint64_t*)(za_ql1)+i);
		uint64_t blockH1 = *((uint64_t*)(za_qh1)+i);
		uint64_t blockL2 = *((uint64_t*)(za_ql2)+i);
		uint64_t blockH2 = *((uint64_t*)(za_qh2)+i);

		if (mBsrRobShift < Int64BitLength)
		{
			mask1 = mask1 << mBsrRobShift | mask1 >> mBsrLobShift;
			mask2 = mask2 << mBsrRobShift | mask2 >> mBsrLobShift;
		}
		else
		{
			mask1 = mask1 >> mBsrLobShift;
			mask2 = mask2 >> mBsrLobShift;
		}

		if (i == mBsrSteps - 1 && mBsrFirsTZaBlockMask != mBsrDefaultMask)
		{
			blockL1 &= mBsrFirsTZaBlockMask;
			blockH1 &= mBsrFirsTZaBlockMask;
			blockL2 &= mBsrFirsTZaBlockMask;
			blockH2 &= mBsrFirsTZaBlockMask;
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

				mask1 &= mBsrShiftMaskArray[pos1];
				b = true;
			}

			if (BitScanReverse64asm(&pos2, (blockL2 ^ blockH1) & mask2))
			{
				if (blockL2 & (uint64_t)1 << pos2)
				{
					return false;
				}

				mask2 &= mBsrShiftMaskArray[pos2];
				b = true;
			}
		}
	}

	return true;
}

bool ZAddressTools::IsIntersected_fast64(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2)
{
	memset(mBsrMaskArrayL + mBsrSteps - 1, 0xFF, Int64ByteLength * (mBsrRob + 1));
	memset(mBsrMaskArrayH + mBsrSteps - 1, 0xFF, Int64ByteLength * (mBsrRob + 1));

	for (int i = mBsrSteps - 1; i >= 0; i--)
	{
		uint64_t blockL1 = *((uint64_t*)(za_ql1)+i);
		uint64_t blockH1 = *((uint64_t*)(za_qh1)+i);
		uint64_t blockL2 = *((uint64_t*)(za_ql2)+i);
		uint64_t blockH2 = *((uint64_t*)(za_qh2)+i);

		uint64_t* maskPtr1 = mBsrMaskArrayL + i;
		uint64_t* maskPtr2 = mBsrMaskArrayH + i;

		if (mBsrRobShift < Int64BitLength)
		{
			*maskPtr1 = *(maskPtr1 + mBsrRob) << mBsrRobShift | *(maskPtr1 + mBsrLob) >> mBsrLobShift;
			*maskPtr2 = *(maskPtr2 + mBsrRob) << mBsrRobShift | *(maskPtr2 + mBsrLob) >> mBsrLobShift;
		}
		else
		{
			*maskPtr1 = *(maskPtr1 + mBsrLob) >> mBsrLobShift;
			*maskPtr2 = *(maskPtr2 + mBsrLob) >> mBsrLobShift;
		}

		if (i == mBsrSteps - 1 && mBsrFirsTZaBlockMask != mBsrDefaultMask)
		{
			blockL1 &= mBsrFirsTZaBlockMask;
			blockH1 &= mBsrFirsTZaBlockMask;
			blockL2 &= mBsrFirsTZaBlockMask;
			blockH2 &= mBsrFirsTZaBlockMask;
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
		ZAddressTools::BitScanReverse64asm(&bo, xor_a_b);

		// uint diffABbitOrder = (blockOrder << cZAddressBlock::MULTIDIV_SHIFT) + bo; // 5 for int32, 6 for int64
		// compute mask in this way: 11111 ... 0000 ...
		uint32_t t = (uint32_t)1 << ((sizeof(uint32_t)*8) - 1);
		diffABBlockMask = (uint32_t)(t >> ((sizeof(uint32_t)*8) - (bo + 1)));
		diffABBlockMask = ~diffABBlockMask;
	}
}
void DetectFirstDiffBit64(uint blockOrder, uint64_t a, uint64_t b, uint64_t &diffABBlockMask, unsigned long & bo)
{
	if (a != b)
	{
		uint64_t xor_a_b = a ^ b;
		ZAddressTools::BitScanReverse64asm(&bo, xor_a_b);

		// uint diffABbitOrder = (blockOrder << cZAddressBlock::MULTIDIV_SHIFT) + bo; // 5 for int32, 6 for int64
		// compute mask in this way: 11111 ... 0000 ...
		diffABBlockMask = ((uint64_t)1 << (bo))-1;

		/*
		uint64_t t = (uint64_t)1 << (Int64BitLength - 1);
		diffABBlockMask = (uint64_t)(t >> (Int64BitLength - (bo + 1)));
		diffABBlockMask = ~diffABBlockMask;
		*/
		//vysledek BitOrder pozice bitu
		//         diffABBlockMask
	}
}

bool ZAddressTools::IsIntersected_ZrQr_block_bsr(char* zi_a, char* zi_b, char* za_ql, char* za_qh)
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

	uint maskH = mBsrDefaultMask32;
	uint maskL = mBsrDefaultMask32;

	uint half_a_block;
	uint half_b_block;

	// Tento jednoduchy test sice pomaha, ale nejsem si jisty, jestli to neni spis charakterem dotazu.
	/*
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
	*/

	for (int step = mBsrSteps32-1; step >= 0; step--)
	{
		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block = za_ql_ui[step];
		uint qh_block = za_qh_ui[step];

		if (a_block != b_block)
		{
			diffStep = step;
			DetectFirstDiffBit(step, a_block, b_block, diffABBlockMask, diffPos);
			diffABBlockMask1 = (diffABBlockMask << 1) + 1;
			uint diffBitMask = 1 << diffPos;

			half_a_block = a_block | diffABBlockMask; //...11111111101111111
			half_b_block = half_a_block + 1;//...11111111110000000
			//zacatek is in rectangle
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

		maskH = maskH << mBsrRobShift32 | maskH >> mBsrLobShift32;
		maskL = maskL << mBsrRobShift32 | maskL >> mBsrLobShift32;
		//konec is in rectangle
	}

	for (int step = diffStep; step >= 0; step--)
	{
		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block_a = za_ql_ui[step];
		uint qh_block_b = za_qh_ui[step];

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

		maskH = maskH << mBsrRobShift32 | maskH >> mBsrLobShift32;
		maskL = maskL << mBsrRobShift32 | maskL >> mBsrLobShift32;
	}

	return true;
}

bool ZAddressTools::IsIntersected_ZrQr_block_fast(char* zi_a, char* zi_b, char* za_ql, char* za_qh)
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

	memset(mMaskArrayAL32 + firstDiffStep, 0xFF, Int32ByteLength * (mBsrRob32 + 1));
	memset(mMaskArrayAH32 + firstDiffStep, 0xFF, Int32ByteLength * (mBsrRob32 + 1));
	uint* maskL = mMaskArrayAL32 + firstDiffStep;
	uint* maskH = mMaskArrayAH32 + firstDiffStep;


	for (int step = firstDiffStep; step >= 0; step--)
	{
		uint a_block = zi_a_ui[step];
		uint b_block = zi_b_ui[step];
		uint ql_block = za_ql_ui[step];
		uint qh_block = za_qh_ui[step];

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

			if (mBsrRobShift32 < Int32BitLength)
			{
				*maskL = *(maskL + mBsrRob32) << mBsrRobShift32 | *(maskL + mBsrLob32) >> mBsrLobShift32;
				*maskH = *(maskH + mBsrRob32) << mBsrRobShift32 | *(maskH + mBsrLob32) >> mBsrLobShift32;
			}
			else
			{
				*maskL = *(maskL + mBsrLob32) >> mBsrLobShift32;
				*maskH = *(maskH + mBsrLob32) >> mBsrLobShift32;
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

			if (mBsrRobShift32 < Int32BitLength)
			{
				*maskL = *(maskL + mBsrRob32) << mBsrRobShift32 | *(maskL + mBsrLob32) >> mBsrLobShift32;
				*maskH = *(maskH + mBsrRob32) << mBsrRobShift32 | *(maskH + mBsrLob32) >> mBsrLobShift32;
			}
			else
			{
				*maskL = *(maskL + mBsrLob32) >> mBsrLobShift32;
				*maskH = *(maskH + mBsrLob32) >> mBsrLobShift32;
			}
		}
	}

	return true;
}

bool ZAddressTools::IsIntersected_ZrQr_block64_bsr(char* zi_a, char* zi_b, char* za_ql, char* za_qh)
{
	// Type-casted arrays.
	uint64_t* zi_a_ui = (uint64_t*)zi_a;
	uint64_t* zi_b_ui = (uint64_t*)zi_b;
	uint64_t* za_ql_ui = (uint64_t*)za_ql;
	uint64_t* za_qh_ui = (uint64_t*)za_qh;

	bool aFalse = false;
	bool bFalse = false;

	unsigned long pos;
	int diffStep = -1;

	uint64_t diffABBlockMask;
	uint64_t diffABBlockMask1;

	unsigned long diffPos;
	uint64_t firstDiffMask;

	uint64_t maskH = mBsrDefaultMask;
	uint64_t maskL = mBsrDefaultMask;

	uint64_t half_a_block;
	uint64_t half_b_block;

	// Tento jednoduchy test sice pomaha, ale nejsem si jisty, jestli to neni spis charakterem dotazu.
	int firstDiffStep = -1;
	for (int step = mBsrSteps - 1; step >= 0; step--)
	{
		uint64_t a_block = zi_a_ui[step];
		uint64_t b_block = zi_b_ui[step];
		uint64_t ql_block = za_ql_ui[step];
		uint64_t qh_block = za_qh_ui[step];

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
		uint64_t a_block = zi_a_ui[step];
		uint64_t b_block = zi_b_ui[step];
		uint64_t ql_block = za_ql_ui[step];
		uint64_t qh_block = za_qh_ui[step];

		if (a_block != b_block)
		{
			diffStep = step;
			DetectFirstDiffBit64(step, a_block, b_block, diffABBlockMask, diffPos);
			diffABBlockMask1 = (diffABBlockMask << 1) + 1;
			uint64_t diffBitMask = (uint64_t)1 << diffPos;

			half_a_block = a_block | diffABBlockMask;
			half_b_block = half_a_block + 1;
			//zacatek is in rectangle
			//tady je jeste a block shodny s b blockem
			while (BitScanReverse64asm(&pos, (ql_block ^ a_block) & maskL & ~diffABBlockMask1))
			{
				if (ql_block & (uint64_t)1 << pos)
				{
					return false;
				}
				else
				{
					maskL &= mBsrShiftMaskArray[pos];
				}
			}

			while (BitScanReverse64asm(&pos, (qh_block ^ a_block) & maskH & ~diffABBlockMask1))
			{
				if (a_block & (uint64_t)1 << pos)
				{
					return false;
				}
				else
				{
					maskH &= mBsrShiftMaskArray[pos];
				}
			}
			//konec is in rectangle
			if (~ql_block & maskL & diffBitMask)
			{
				maskL &= mBsrShiftMaskArray[diffPos];
			}

			if (ql_block & maskL & diffBitMask)
			{
				aFalse = true;
			}

			if (qh_block & maskH & diffBitMask)
			{
				maskH &= mBsrShiftMaskArray[diffPos];
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
			if ((uint64_t)1 << pos & ql_block)
			{
				return false;
			}
			else
			{
				maskL &= mBsrShiftMaskArray[pos];
			}
		}

		while (BitScanReverse64asm(&pos, (qh_block ^ a_block) & maskH))
		{
			if ((uint64_t)1 << pos & a_block)
			{
				return false;
			}
			else
			{
				maskH &= mBsrShiftMaskArray[pos];
			}
		}

		maskH = maskH << mBsrRobShift | maskH >> mBsrLobShift;
		maskL = maskL << mBsrRobShift | maskL >> mBsrLobShift;
		//konec is in rectangle
	}

	for (int step = diffStep; step >= 0; step--)
	{
		uint64_t a_block = zi_a_ui[step];
		uint64_t b_block = zi_b_ui[step];
		uint64_t ql_block_a = za_ql_ui[step];
		uint64_t qh_block_b = za_qh_ui[step];

		uint64_t qh_block_a;
		uint64_t ql_block_b;

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

		maskH = maskH << mBsrRobShift | maskH >> mBsrLobShift;
		maskL = maskL << mBsrRobShift | maskL >> mBsrLobShift;
	}

	return true;
}

bool ZAddressTools::IsIntersected_ZrQr_block64_fast(char* zi_a, char* zi_b, char* za_ql, char* za_qh)
{
	// Type-casted arrays.
	uint64_t* zi_a_ui = (uint64_t*)zi_a;
	uint64_t* zi_b_ui = (uint64_t*)zi_b;
	uint64_t* za_ql_ui = (uint64_t*)za_ql;
	uint64_t* za_qh_ui = (uint64_t*)za_qh;

	bool aFalse = false;
	bool bFalse = false;

	unsigned long pos;
	int diffStep = -1;

	uint64_t diffABBlockMask;
	uint64_t diffABBlockMask1;

	unsigned long diffPos;
	uint64_t firstDiffMask;

	uint64_t half_a_block;
	uint64_t half_b_block;

	// Tento jednoduchy test sice pomaha, ale nejsem si jisty, jestli to neni spis charakterem dotazu.
	int firstDiffStep = -1;
	for (int step = mBsrSteps - 1; step >= 0; step--)
	{
		uint64_t a_block = zi_a_ui[step];
		uint64_t b_block = zi_b_ui[step];
		uint64_t ql_block = za_ql_ui[step];
		uint64_t qh_block = za_qh_ui[step];

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

	memset(mMaskArrayAL + firstDiffStep, 0xFF, Int64ByteLength * (mBsrRob + 1));
	memset(mMaskArrayAH + firstDiffStep, 0xFF, Int64ByteLength * (mBsrRob + 1));
	uint64_t* maskL = mMaskArrayAL + firstDiffStep;
	uint64_t* maskH = mMaskArrayAH + firstDiffStep;


	for (int step = firstDiffStep; step >= 0; step--)
	{
		uint64_t a_block = zi_a_ui[step];
		uint64_t b_block = zi_b_ui[step];
		uint64_t ql_block = za_ql_ui[step];
		uint64_t qh_block = za_qh_ui[step];

		if (a_block != b_block)
		{
			diffStep = step;
			DetectFirstDiffBit64(step, a_block, b_block, diffABBlockMask, diffPos);
			diffABBlockMask1 = (diffABBlockMask << 1) + 1;
			uint64_t diffBitMask = (uint64_t)1 << diffPos;

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

			if (mBsrRobShift < Int32BitLength)
			{
				*maskL = *(maskL + mBsrRob) << mBsrRobShift | *(maskL + mBsrLob) >> mBsrLobShift;
				*maskH = *(maskH + mBsrRob) << mBsrRobShift | *(maskH + mBsrLob) >> mBsrLobShift;
			}
			else
			{
				*maskL = *(maskL + mBsrLob) >> mBsrLobShift;
				*maskH = *(maskH + mBsrLob) >> mBsrLobShift;
			}
		}
	}

	for (int step = diffStep; step >= 0; step--)
	{
		maskL = mMaskArrayAL + step;
		maskH = mMaskArrayAH + step;

		uint64_t a_block = zi_a_ui[step];
		uint64_t b_block = zi_b_ui[step];
		uint64_t ql_block_a = za_ql_ui[step];
		uint64_t qh_block_b = za_qh_ui[step];

		uint64_t qh_block_a;
		uint64_t ql_block_b;

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

			if (mBsrRobShift < Int64BitLength)
			{
				*maskL = *(maskL + mBsrRob) << mBsrRobShift | *(maskL + mBsrLob) >> mBsrLobShift;
				*maskH = *(maskH + mBsrRob) << mBsrRobShift | *(maskH + mBsrLob) >> mBsrLobShift;
			}
			else
			{
				*maskL = *(maskL + mBsrLob) >> mBsrLobShift;
				*maskH = *(maskH + mBsrLob) >> mBsrLobShift;
			}
		}
	}

	return true;
}

bool ZAddressTools::BitScanReverse64(unsigned long * index, uint64_t mask)
{
	if (mask == 0)
		return false;
	*index = 63- __builtin_clzll(mask);
	return true;
}
bool ZAddressTools::BitScanReverse64asm(unsigned long * index, uint64_t mask)
{
	if (mask == 0)
		return false;
    __asm__ ("bsr %1, %0" : "=r" (*index) : "r" (mask)); // inline assembly to perform bsr
	return true;
}