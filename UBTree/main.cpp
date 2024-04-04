#include <iostream>
#include "cBpTree.h"
#include <stdlib.h>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <random>
#include <numeric>


#define u_int8_t unsigned char
#define u_int16_t unsigned short
#define u_int32_t unsigned int
#define u_int64_t unsigned long long

int benchmarks = 0;
float GetThroughput(int opsCount, float period, int unit = 1000000)
{
    return ((float)opsCount / unit) / period;
}
float BytesToMB(int bytes){
    return (float)bytes / 1024 / 1024;
}
void benchmark_uint16(int dims, int recordsNum, int queryNum, int iterations, int innerNodeCap = 32, int leafNodeCap = 32, float querySelectivity = 0.04){
    cBpTree<uint16_t, uint16_t> tree = cBpTree<uint16_t, uint16_t>(dims, innerNodeCap, leafNodeCap);
    uint16_t* data = new uint16_t[recordsNum * dims];
    int maxVal = 64;
    //generate random n-dim data.
    //we need something workable, so we will halve the interval for each next dimension
    //for dims 0-7, we will have 256, 128, 64, 32, 16, 8, 4, 2; for dims 8-15 only 2. Each following is zero
    int maxValTmp = maxVal;
    for(int i = 0; i < dims; i++){
        for(int j = 0; j < recordsNum; j++){
            data[j * dims + i] = rand() % maxValTmp;
        }
        /*
        if(i < 8){
            maxValTmp = maxValTmp / 2;
        }
        else if(i < 16){
            maxValTmp = 2;
        }
        else{
            maxValTmp = 0;
        }
        */
    }

    cTuple<uint16_t> tupleContainer = cTuple<uint16_t>(data, dims, true);
    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < recordsNum; i++){
        tupleContainer.setTuple(data + i * dims, dims);
        tree.insert(tupleContainer);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span_insert = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
    printf("Insert: Time: %.2fs, Records:%d, Throughput: %.2f mil. op/s.\n\n", time_span_insert.count(), recordsNum, GetThroughput(recordsNum, time_span_insert.count()));
    tree.printMetadata();
    printf("BpTree MB: %.3f\n",BytesToMB(tree.getBpTreeBytes()));
    printf("Number of links: %d\n", tree.checkLinkLength());
    //Generation should be simple enough. By default we set 0-maxVal for each dim.
    //Specified number of dimensions with custom range is selected.
    uint16_t* lqrData = new uint16_t[queryNum * dims];
    uint16_t* hqrData = new uint16_t[queryNum * dims];
    //now we will set random ranges for each query
    srand(17);
    int randomDimNum = std::min(8, dims); //If possible, we want select on 8 dims
    for(int i = 0; i < queryNum; i++){
        int toChange = randomDimNum;
        int toLeave = dims - randomDimNum;
        float currentSelectivity = 1.0f;
        for(int j = 0; j < dims; j++){
            bool change = rand() % (toChange + toLeave) < toChange;
            if(change){ //if dim to be changed
                int interval = rand() % maxVal;
                if(toChange == 1){//If we are at the last, calculate last interval based on selectivities
                    interval = (querySelectivity * (maxVal-1)) / currentSelectivity;
                }
                else{
                    while(true){
                        if((static_cast<float>(interval)/(maxVal-1))*currentSelectivity < querySelectivity){
                            interval = interval + (rand() % (maxVal-interval));
                        }
                        else{
                            break;
                        }
                    }
                }
                lqrData[i * dims + j] = rand() % (maxVal-interval);
                hqrData[i * dims + j] = lqrData[i * dims + j] + interval;
                toChange--;
                currentSelectivity *= (static_cast<float>(interval)/(maxVal-1));
            }
            else{//if normal dim
                lqrData[i * dims + j] = 0;
                hqrData[i * dims + j] = (maxVal-1);
                toLeave--;
            }
        }
    }
    cTuple<uint16_t> lQr = cTuple<uint16_t>(lqrData, dims, true);
    cTuple<uint16_t> hQr = cTuple<uint16_t>(hqrData, dims, true);
    cBpTreeIterator<uint16_t> *iter = nullptr;
    int* volumes = new int[queryNum];
    auto t3 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queryNum; i++){
        lQr.setTuple(lqrData + i * dims, dims);
        hQr.setTuple(hqrData + i * dims, dims);
        iter = tree.searchRangeIterator(lQr, hQr);
        int volume = iter->skip(-1);
        volumes[i] = volume;
        std::cout << "(" << iter->rectangleCalls << ", " << iter->isIntersectedCalls << "); ";
        delete iter;
    }
    auto t4 = std::chrono::high_resolution_clock::now();
    std::cout << std::endl;
    std::chrono::duration<double> time_span_search = std::chrono::duration_cast<std::chrono::duration<double>>(t4 - t3);
    printf("Search: Time: %.2fs, Queries:%d, Throughput: %.2f op/s.\n", time_span_search.count(), queryNum, GetThroughput(queryNum, time_span_search.count(), 1));
    //print min, max, avg volume
    int minVolume = *std::min_element(volumes, volumes + queryNum);
    int maxVolume = *std::max_element(volumes, volumes + queryNum);
    int sumVolume = std::accumulate(volumes, volumes + queryNum, 0);
    printf("Volume: Min: %d, Max: %d, Avg: %.2f\n\n", minVolume, maxVolume, (float)sumVolume/queryNum);

    int* volumesStack = new int[queryNum];  
    auto t3s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < queryNum; i++) {
		lQr.setTuple(lqrData + i * dims, dims);
		hQr.setTuple(hqrData + i * dims, dims);
		iter = tree.searchRangeIteratorStack(lQr, hQr);
		int volume = iter->skip(-1);
		volumesStack[i] = volume;
        std::cout << "(" << iter->rectangleCalls << ", " << iter->isIntersectedCalls << "); ";
		delete iter;
	}
    auto t4s = std::chrono::high_resolution_clock::now();
    std::cout << std::endl;
    std::chrono::duration<double> time_span_search_stack = std::chrono::duration_cast<std::chrono::duration<double>>(t4s - t3s);
    printf("Search Stack: Time: %.2fs, Queries:%d, Throughput: %.2f op/s.\n", time_span_search_stack.count(), queryNum, GetThroughput(queryNum, time_span_search_stack.count(), 1));
    //print min, max, avg volume
    minVolume = *std::min_element(volumesStack, volumesStack + queryNum);
    maxVolume = *std::max_element(volumesStack, volumesStack + queryNum);
    sumVolume = std::accumulate(volumesStack, volumesStack + queryNum, 0);
    printf("Volume Stack: Min: %d, Max: %d, Avg: %.2f\n\n", minVolume, maxVolume, (float)sumVolume/queryNum);
    //benchmark point queries, take points from data
    int* pointVolumes = new int[queryNum];
    auto t5 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < queryNum; i++){
        lQr.setTuple(data + i * dims, dims);
        iter = tree.searchPointIterator(lQr);
        int volume = iter->skip(-1);
        pointVolumes[i] = volume;
        delete iter;
    }
    auto t6 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span_search_point = std::chrono::duration_cast<std::chrono::duration<double>>(t6 - t5);
    printf("Search Point: Time: %.2fs, Queries:%d, Throughput: %.2f op/s.\n", time_span_search_point.count(), queryNum, GetThroughput(queryNum, time_span_search_point.count(), 1));
    //print min, max, avg volume
    minVolume = *std::min_element(pointVolumes, pointVolumes + queryNum);
    maxVolume = *std::max_element(pointVolumes, pointVolumes + queryNum);
    sumVolume = std::accumulate(pointVolumes, pointVolumes + queryNum, 0);
    printf("Volume Point: Min: %d, Max: %d, Avg: %.2f\n\n", minVolume, maxVolume, (float)sumVolume/queryNum);

    delete []data;
    delete []lqrData;
    delete []hqrData;
    delete []volumes;
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

    cBpTreeIteratorRange<uint8_t> *iter = tree.searchRangeIterator(lQr, hQr);
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
    delete []data;
    delete []lqrData;
    delete []hqrData;
    delete iter;
    delete lqrZaddr;
    delete hqrZaddr;

}

void smallTestCase(){
    //TESTCASE 3 
    cBpTree<u_int8_t, u_int8_t> tree(2, 8, 8);

    // Insert some data
    srand(170400);

    u_int8_t* data = new u_int8_t[2]{0, 0};
    cTuple<u_int8_t> * tupleContainer = new cTuple<u_int8_t>(data, 2, true);

    int jMax, iMax;
    iMax = 0xF;
    jMax = 0xF;
    int recordsNum = iMax * jMax;

    auto t1 = std::chrono::high_resolution_clock::now();
    data[0] = 7;
    data[1] = 7;
    //insert 3*7 for testing of point query
    tree.insert(*tupleContainer);
    tree.insert(*tupleContainer);
    tree.insert(*tupleContainer);


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
    delete []data;

    u_int8_t* lqrData = new u_int8_t[2]{2, 0};
    u_int8_t* hqrData = new u_int8_t[2]{3, 2};
    char * reconstruction = new char[2*sizeof(int)];

    cTuple<u_int8_t> lQr = cTuple<u_int8_t>(lqrData, 2, true); //4 ,  0x4
    cTuple<u_int8_t> hQr = cTuple<u_int8_t>(hqrData, 2, true); //13 , 0xD
    cTuple<u_int8_t> container = cTuple<u_int8_t>((u_int8_t*)reconstruction, 2, true);

    cBpTreeIterator<u_int8_t> *iter = tree.searchRangeIterator(lQr, hQr);
    /*
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
    */
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

    iter = tree.searchRangeIteratorStack(lQr, hQr);
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

    lqrData[0] = 7;
    lqrData[1] = 7;
    iter = tree.searchPointIterator(lQr);

    std::cout << "Point query ZAddrs Found: ";
    while (iter->hasNext()){
        cTuple<u_int8_t> * tuple = iter->next();
        tuple->printAsZaddress(); std::cout << ", ";
    }
    std::cout << std::endl;
    iter->reset();
    std::cout << "Skipped: " <<iter->skip(-1) << std::endl;
    delete iter;

    std::cout << "Delete test 3,3 " << std::endl;
    lqrData[0] = 3;
    lqrData[1] = 3;
    lQr.setTuple(lqrData, 2);
    iter = tree.searchPointIterator(lQr);
    std::cout << "Point query ZAddrs Found: ";
    while (iter->hasNext()){
        cTuple<u_int8_t> * tuple = iter->next();
        tuple->printAsZaddress(); std::cout << ", ";
    }
    std::cout << std::endl;
    iter->reset();
    std::cout << "Skipped: " <<iter->skip(-1) << std::endl;
    delete iter;
    tree.remove(lQr);
    iter = tree.searchPointIterator(lQr);
    std::cout << "Point query ZAddrs Found: ";
    while (iter->hasNext()){
        cTuple<u_int8_t> * tuple = iter->next();
        tuple->printAsZaddress(); std::cout << ", ";
    }
    std::cout << std::endl;
    iter->reset();
    std::cout << "Skipped: " <<iter->skip(-1) << std::endl;
    delete iter;

    delete []lqrData;
    delete []hqrData;
    delete []reconstruction;
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
    printf("ZAddressX: %llx\n", ptrZAddressXLong[0]);
}
void testFastVSBsr(){
    cBpTree<short, int> tree(8, 5, 5);
    cTuple<short> tupleLow = cTuple<short>(new short[9]{0, 0, 0, 0, 0, 0, 0, 0, 0}, 9);
    cTuple<short> tupleHigh = cTuple<short>(new short[9]{3, 3, 3, 3, 3, 3, 3, 3, 3}, 9);
    cTuple<short> tupleTest = cTuple<short>(new short[9]{1, 1, 1, 1, 1, 1, 1, 1, 1}, 9);

    char* zAddrLow = nullptr;
    char* zAddrHigh = nullptr;
    char* zAddrTest = nullptr;
    tree.getZTools()->transformDataToZAddress((char*)tupleLow.getAttributes(), zAddrLow);
    tree.getZTools()->transformDataToZAddress((char*)tupleHigh.getAttributes(), zAddrHigh);
    tree.getZTools()->transformDataToZAddress((char*)tupleTest.getAttributes(), zAddrTest);

    bool bsrRes = tree.getZTools()->IsInRectangle_bsr_32(zAddrTest, zAddrLow, zAddrHigh);
    bool fastRes = tree.getZTools()->IsInRectangle_fast_8(zAddrTest, zAddrLow, zAddrHigh);
    printf("BSR: %d, Fast: %d\n", bsrRes, fastRes);
}
int main() {
    //zAddressTranslationTest();
    //smallTestCase();
    //mediumTest();
    benchmark_uint16(4, 1e6, 50, 1, 128, 64, 0.001);
    //testFastVSBsr();
    return 0;
}
