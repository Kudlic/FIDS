//Declaration for Record class
//Contains Constructor, Destructor, Member Functions and Data Members
//Atributes: ZCurve (char array), ZCurveSize (int), Data (int array), DataSize (int)

//TODO: Make ZCurve into standalone class, so it can be used in functions standalone

#include <iostream>
#include <bitset>
#include <cstddef>
#include "ZAddressTools.h"

#pragma once

template <typename T = uint32_t>
class Record {
    public:
        ZAddressTools *zAddressTools;
        char* zAddress;
        T* data;
        Record();
        Record(T* data, ZAddressTools *zAddressTools);
        ~Record();
        void print();
        void printZAddress(bool reverse = false);
        void printZAddressInt32(bool reverse = false);
        void printData();
        char* getZAddressString();
        char* getZAddressStringBase2();
};

template <typename T>
Record<T>::Record(){
    this->zAddressTools = nullptr;
    this->zAddress = nullptr;
    this->data = nullptr;
}
template <typename T>
Record<T>::Record(T* data, ZAddressTools *zAddressTools){
    this->zAddressTools = zAddressTools;
    this->zAddress = nullptr;
    this->data = data;
    zAddressTools->transformDataToZAddress((char*)data, zAddress);
}
template <typename T>
Record<T>::~Record(){
    //std::cout<<"~Record()"<<std::endl;
    delete[] zAddress;
    delete[] data;
}
template <typename T>
void Record<T>::print(){
    printData();
    printZAddress();
}
template <typename T>
void Record<T>::printZAddress(bool reverse){
    std::cout << "ZCurve: bytes: " << zAddressTools->zAddressBytes << std::endl;
    if (reverse){
        for (int i = zAddressTools->zAddressBytes-1; i >= 0 ; i--){
            std::cout << std::bitset<8>(zAddress[i])<<" ";
        }
    }
    else{
        for (int i = 0; i < zAddressTools->zAddressBytes ; i++){
            std::cout << std::bitset<8>(zAddress[i])<<" ";
        }
    }
    std::cout << std::endl;
}
template <typename T>
void Record<T>::printZAddressInt32(bool reverse){
    std::cout << "ZCurve: bytes: " << zAddressTools->zAddressBytes << std::endl;
    int32_t * zAddressInt = (int32_t*)zAddress;
    if (reverse){
        for (int i = zAddressTools->zAddressBytes/4; i > 0 ; i--){
            std::cout << std::bitset<32>(zAddressInt[i-1])<<" ";
        }
    }
    else{
        for (int i = 0; i < zAddressTools->zAddressBytes/4 ; i++){
            std::cout << std::bitset<32>(zAddressInt[i])<<" ";
        }
    }
    std::cout << std::endl;
}
template <typename T>
void Record<T>::printData(){
    std::cout << "Data: ";
    for (int i = 0; i < zAddressTools->dataSize; i++){
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
}
template <typename T>
char* Record<T>::getZAddressString(){
    char* zAddressString = new char[zAddressTools->zAddressBits+1];
    for (int i = 0; i < zAddressTools->zAddressBits; i++){
        zAddressString[i] = ZAddressTools::getZAddressBit(zAddress, i) + '0';
    }
    zAddressString[zAddressTools->zAddressBits] = '\0';
    return zAddressString;
}
template <typename T>
char* Record<T>::getZAddressStringBase2(){
    char* zAddressString = new char[zAddressTools->zAddressBits+3];
    zAddressString[0] = '0';
    zAddressString[1] = 'b';
    int j = 2;
    for (int i = zAddressTools->zAddressBits; i > 0; j++){
        i--;
        zAddressString[j] = ZAddressTools::getZAddressBit(zAddress, i) + '0';
    }
    zAddressString[j] = '\0';
    return zAddressString;
}