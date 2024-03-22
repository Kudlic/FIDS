#include <iostream>
#include "cBpTree.h"
#include <stdlib.h>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <random>

int benchmarks = 0;
float GetThroughput(int opsCount, float period, int unit = 1000000)
{
    return ((float)opsCount / unit) / period;
}
float BytesToMB(int bytes){
    return (float)bytes / 1024 / 1024;
}
void mediumTest(){
    int dims = 8;
    cBpTree<uint8_t, uint8_t> tree = cBpTree<uint8_t, uint8_t>(dims, 32, 32);
    uint8_t* data = new uint8_t[dims];
    cTuple<uint8_t> * tupleContainer = new cTuple<uint8_t>(data, dims, true);
    //now generate every possible 8 byte tuple where values of first 4 dims range from 0-8 and insert them
    int recordsNum = pow(8, 4);
    int counter = 0;

    auto t1 = std::chrono::high_resolution_clock::now();
    for(counter = 0; counter < recordsNum; counter++){
        for(int j = 0; j < dims; j++){
            data[j] = counter/((int)pow(8, j)) % 8;
        }
        tree.insert(*tupleContainer);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Insert BpTable done. Time: %.2fs, Throughput: %.2f mil. op/s.\n\n", time_span.count(), GetThroughput(recordsNum, time_span.count()));
    tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));
    tree.printTreeHex();

    //Now try several searches, calculate their volume and verify with search iterator
    uint8_t* lqrData = new uint8_t[dims]{5, 2, 2, 5, 0, 0, 0, 0}; 
    uint8_t* hqrData = new uint8_t[dims]{6, 3, 6, 5, 7, 7, 7, 7}; 
    //make zAddrs and print them
    char * lqrZaddr = nullptr;
    char * hqrZaddr = nullptr;

    tree.getZTools()->transformDataToZAddress((char*)lqrData, lqrZaddr);
    tree.getZTools()->transformDataToZAddress((char*)hqrData, hqrZaddr);

    cTuple<uint8_t> lQr = cTuple<uint8_t>(lqrData, dims, true);
    cTuple<uint8_t> hQr = cTuple<uint8_t>(hqrData, dims, true);
    lQr.printAsZaddress(); std::cout << " - "; hQr.printAsZaddress(); std::cout << std::endl;

    cBpTreeIterator<uint8_t> *iter = tree.searchRangeIterator(lQr, hQr);
    int volumeExpected = 1;
    for(int i = 0; i < dims; i++){
        volumeExpected *= (hqrData[i] - lqrData[i] + 1);
    }
    int volumeIter = 0;
    char* dataPr = (char*)new uint8_t[dims];
    while(iter->hasNext()){
        cTuple<uint8_t> * tuple = iter->next();
        //conver back to tuple and print
        tree.getZTools()->transformZAddressToData((char*)tuple->getAttributes(), dataPr);
        cTuple<uint8_t> tuplePr = cTuple<uint8_t>((uint8_t*)dataPr, dims, true);
        tuple->printAsZaddress(); std::cout << ", ";
        tuplePr.printTuple();

        volumeIter++;
    }
    printf("Volume expected: %d, Volume iter: %d\n", volumeExpected, volumeIter);

    delete tupleContainer;
    delete data;
    delete lqrData;
    delete hqrData;
    delete iter;
    delete lqrZaddr;
    delete hqrZaddr;

}

void smallTestCase(){
    //TESTCASE 3 
    cBpTree<u_int8_t, u_int8_t> tree(2, 16, 32);

    // Insert some data
    srand(170400);

    u_int8_t* data = new u_int8_t[2]{0, 0};
    cTuple<u_int8_t> * tupleContainer = new cTuple<u_int8_t>(data, 2, true);

    int jMax, iMax;
    iMax = 0x3F;
    jMax = 0x3F;
    int recordsNum = iMax * jMax;

    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < iMax; i++) {
        for(int j = 0; j < jMax; j++) {
            data[0] = i;
            data[1] = j;
            
            if(!tree.insert(*tupleContainer)){
                printf("Insertion failed!\n");
                break;
            }
            
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));

    printf("Insert BpTable done. Time: %.2fs, Throughput: %.2f mil. op/s.\n\n", time_span.count(), GetThroughput(recordsNum, time_span.count()));
    
    //tree.printTreeTuples();
    tree.printTreeHex();
    //memory clean up
    delete tupleContainer; 
    delete data;

    u_int8_t* lqrData = new u_int8_t[2]{2, 0};
    u_int8_t* hqrData = new u_int8_t[2]{3, 2};
    char * reconstruction = new char[2*sizeof(int)];

    cTuple<u_int8_t> lQr = cTuple<u_int8_t>(lqrData, 2, true); //4 ,  0x4
    cTuple<u_int8_t> hQr = cTuple<u_int8_t>(hqrData, 2, true); //13 , 0xD
    cTuple<u_int8_t> container = cTuple<u_int8_t>((u_int8_t*)reconstruction, 2, true);

    cBpTreeIterator<u_int8_t> *iter = tree.searchRangeIterator(lQr, hQr);

    std::cout << "ZAddrs Found: ";
    while(iter->hasNext()){
        cTuple<u_int8_t> * tuple = iter->next();
        tree.getZTools()->transformZAddressToData((char*)tuple->getAttributes(), reconstruction);
        container.setTuple((u_int8_t*)reconstruction, 2);
        tuple->printAsZaddress(); std::cout << ", ";
    }
    std::cout << std::endl;
    iter->reset();
    std::cout << "Skipped: " << iter->skip(-1) << std::endl;
    delete iter;

    //set data to 7,4 and 10, 8 and search again
    lqrData[0] = 7;
    lqrData[1] = 4;
    hqrData[0] = 10;
    hqrData[1] = 8;
    iter = tree.searchRangeIterator(lQr, hQr);
    std::cout << "ZAddrs Found: ";
    while(iter->hasNext()){
        cTuple<u_int8_t> * tuple = iter->next();
        tree.getZTools()->transformZAddressToData((char*)tuple->getAttributes(), reconstruction);
        container.setTuple((u_int8_t*)reconstruction, 2);
        tuple->printAsZaddress(); std::cout << ", ";
    }
    std::cout << std::endl;
    iter->reset();
    std::cout << "Skipped: " <<iter->skip(-1) << std::endl;
    delete iter;

    delete lqrData;
    delete hqrData;
    delete reconstruction;
}

void zAddressTranslationTest(){
    cBpTree<int, int> tree(2, 5, 5);
    cTuple<int> tuple1 = cTuple<int>(new int[2]{0x7, 0x1}, 2);

    char * zAddress = nullptr;
    tree.getZTools()->transformDataToZAddress((char*)tuple1.getAttributes(), zAddress);
    cTuple<int> tuple2 = cTuple<int>((int*)zAddress, tree.getMetadata()->n, true);

    char *unZAddressed = nullptr; // Change the declaration to reference type
    tree.getZTools()->transformZAddressToData((char*)tuple2.getAttributes(), unZAddressed); // Use the address of operator to pass the reference

    cTuple<int> tuple3 = cTuple<int>((int*)unZAddressed, 2, true);

    tuple1.printTupleHex();
    tuple2.printTupleHex();
    tuple3.printTupleHex();

    unsigned int* zAddressX = new unsigned int [2]{0x89abcdef, 0x01234567}; //takto funguje spravne, na vyssi pozici je vyznamnejsi byte
    unsigned char* ptrZAddressX = (unsigned char*)zAddressX;
    unsigned short* ptrZAddressXShort = (unsigned short*)zAddressX;
    u_int64_t * ptrZAddressXLong = (u_int64_t*)zAddressX;
    printf("ZAddressX: %x %x %x %x %x %x %x %x\n", ptrZAddressX[7], ptrZAddressX[6], ptrZAddressX[5], ptrZAddressX[4], ptrZAddressX[3], ptrZAddressX[2], ptrZAddressX[1], ptrZAddressX[0]);
    printf("ZAddressX: %x %x %x %x\n", ptrZAddressXShort[3], ptrZAddressXShort[2], ptrZAddressXShort[1], ptrZAddressXShort[0]);
    printf("ZAddressX: %lx\n", ptrZAddressXLong[0]);
}
int main() {
    //zAddressTranslationTest();
    //smallTestCase();
    mediumTest();
    return 0;
}
