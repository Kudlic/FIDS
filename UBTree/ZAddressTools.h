#include <iostream>
#include <bitset>
#include <cstddef>

#define Int32BitLength 32
#define Int32ByteLength 4
#define Int64BitLength 64
#define Int64ByteLength 8

#pragma once

class ZAddressTools {
    public:
        
        // Petr

        //počet iterací pro BSR algoritmy
        uint mBsrSteps;
        uint64_t mBsrDefaultMask;  //plné 0xFF bytů
        uint64_t mBsrFirsTZaBlockMask;  //Maska pro MSB, který může mít prázdné byty v bloku, Nevychází-li přesně byty všech dimenzí na byty všech bloků, jsou na začátek doplněny nuly
        uint mBsrRob, mBsrLob; // Pro BSR neřešíme, jen pro fasty, o kolik bloků dříve je třeba se podívat pro rekonstrukci masky
        /*Počty bitů použité pro rotaci masky
        mBsrRobShift: o kolik bitů je potřeba posunout masku doleva (<<), počet dimenzí % velikost bloku
        mBsrLobShift: o kolik bitů je potřeba posunout masku doprava (>>), počet dimenzí - robShift
        Nad posunutými maskami proběhne operace OR, výsledek je uložen v mBsrShiftMaskArray jako maska pro další blok
        Použití pro BSR algoritmy
        Trochu jiná pravidla při recyklaci na fastAlgoritmu
        */
        uint mBsrRobShift, mBsrLobShift; 
        uint64_t* mBsrMaskArrayL;
        uint64_t* mBsrMaskArrayH;
        uint64_t* mBsrShiftMaskArray; // 

        // Potrebne pro novy algo

        uint mBsrDefaultMask32; //filled
        uint mBsrSteps32; //filled
        uint* mBsrShiftMaskArray32; //filled

        uint mBsrRob32, mBsrLob32;
        
        uint* mMaskArrayAL32;
        uint* mMaskArrayAH32;
        uint64_t* mMaskArrayAL;
        uint64_t* mMaskArrayAH; //Jen musi existovat aby se mohly plnit

        uint mBsrRobShift32, mBsrLobShift32;// POTREBA


    public:
        std::size_t zAddressBytes;      // Number of bytes in zAddress, equals to number of chars allocated
        std::size_t zAddressBits;       // Number of bits in zAddress, equals to number of chars allocated * 8

        std::size_t dataSize;           // Number of dimensions
        std::size_t dataTypeBytes;      // Number of bytes in one dimension
        std::size_t dataTypeBits;       // Number of bits in one dimension

        ZAddressTools();
        ZAddressTools(std::size_t dataTypeBytes, std::size_t dataSize);
        ~ZAddressTools();
        static bool getZAddressBit(char* zAddress, std::size_t bitIndex);
        void transformDataToZAddress(char* data, char*& za);//msB first
        bool IsInRectangle_bsr64(char* za_t, char* za_ql, char* za_qh);
        bool IsInRectangle_fast64(char* za_t, char* za_ql, char* za_qh);
        static bool BitScanReverse64(unsigned long * index, uint64_t mask);
        static bool BitScanReverse64asm(unsigned long * index, uint64_t mask);
        bool IsIntersected_bsr64(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2);
        bool IsIntersected_fast64(char* za_ql1, char* za_qh1, char* za_ql2, char* za_qh2);
        bool IsIntersected_ZrQr_block_bsr(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
        bool IsIntersected_ZrQr_block_fast(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
        bool IsIntersected_ZrQr_block64_bsr(char* zi_a, char* zi_b, char* za_ql, char* za_qh);
        bool IsIntersected_ZrQr_block64_fast(char* zi_a, char* zi_b, char* za_ql, char* za_qh);//IsIntersected_ZrQr_block64_rptdm_new


        void print();
        
};
