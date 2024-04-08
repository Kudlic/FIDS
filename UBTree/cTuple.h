#pragma once
#include <stdio.h>
#include <iomanip>

template<typename T>
class cTuple {
public:
    cTuple(int n);
    cTuple(int n, bool isWrapper);
    cTuple(T* attributes, int n, bool isWrapper = 0);
    ~cTuple();
    void setTuple(T* attributes, int n);
    void setValue(int index, T value);
    void printTuple();
    void printTupleHex();
    void printAsZaddress();
    bool isEQ(cTuple<T>& tuple, int n = -1);
    bool isLT(cTuple<T>& tuple, int n = -1);
    bool isGT(cTuple<T>& tuple, int n = -1);
    bool isLEQT(cTuple<T>& tuple, int n = -1);
    bool isGEQT(cTuple<T>& tuple, int n = -1);
    bool isTupleBetween(cTuple<T>& tuple1, cTuple<T>& tuple2, int n = -1);
    float getTupleDistance(cTuple<T>& tuple, int n = -1);
    float getTupleRangeDistance(cTuple<T>& tuple1, cTuple<T>& tuple2, int n = -1);
    T* getAttributes() { return attributes; }
    T getValue(int index) { return attributes[index]; }
    int getN() { return n; }
    
    T* attributes;
    int n; // Number of attributes
    bool isWrapper;
};

template<typename T>
cTuple<T>::cTuple(int n) : n(n), isWrapper(0) {
    attributes = new T[n];
}
template<typename T>
cTuple<T>::cTuple(int n, bool isWrapper) : n(n), isWrapper(isWrapper) {
    if(!isWrapper){
        attributes = new T[n];
    }
    else{
        attributes = nullptr;
    }
}   

template<typename T>
cTuple<T>::cTuple(T* attributes, int n, bool isWrapper) : attributes(attributes), n(n), isWrapper(isWrapper) {}

template<typename T>
cTuple<T>::~cTuple() {
    if(!isWrapper){
        delete[] attributes;
    }
    else{
        attributes = nullptr;
    }
}

template<typename T>
void cTuple<T>::setTuple(T* attributes, int n) {
    // Implementation for setTuple method
    this->attributes = attributes;
    this->n = n;
}
template<typename T>
void cTuple<T>::setValue(int index, T value) {
	// Implementation for setValue method
	this->attributes[index] = value;
}

template<typename T>
void cTuple<T>::printTuple() {
    printf("(");
    for(int i = 0; i < n; i++){
        printf("%d ", attributes[i]);
    }
    printf(")");
}
template<typename T>
void cTuple<T>::printTupleHex() {
    printf("(");
    for(int i = 0; i < n; i++){
        printf("%x ", attributes[i]);
    }
    printf(")");
}
/*
template<typename T>
void cTuple<T>::printAsZaddress() {
    //possible cases:
    // 1. ends after first loop without trigger, print 1 zero
    // 2. ends after first loop with trigger, print the number
    // 3. triggers and goes to second loop, print number in trigger and process the rest normally
    int fill = sizeof(T)*2;
    bool zeroes = true;
    int j = this->n-1;
    std::cout << "0x";
    for(; j >= 0 ; j--){
        if((int*)this->attributes[j] != 0){
            zeroes = false;
            std::cout << std::uppercase << std::hex << attributes[j];
            j--;
            break;
        }
    }
    if(zeroes){
        std::cout << 0;
        std::cout << std::dec;
        return;
    }
    for(; j >= 0 ; j--){
            zeroes = false;
            std::cout << std::uppercase << std::setw(fill)<< std::setfill('0')<<std::hex << attributes[j];
        
    }
    std::cout << std::dec;
}*/
template<typename T>
void cTuple<T>::printAsZaddress() {
    int i = (this->n * sizeof(T)) -1;
    printf("0x");
    unsigned char* arr = (unsigned char*)this->attributes;
    // Skip leading zero bytes
    for (; i >= 0 && arr[i] == '\0'; i--);
    if (i == -1) {
        printf("0");
        return;
    }
    // Print remaining bytes
    for (; i >= 0; i--) {
        printf("%02X", (unsigned char)arr[i]);
    }
}
template<typename T>
bool cTuple<T>::isEQ(cTuple<T>& tuple, int n) {
    // Implementation for isTupleEqual method
    if(n == -1){
        n = tuple.n;
    }
    for(int i = 0; i < n; i++){
        if(attributes[i] != tuple.attributes[i]){
            return false;
        }
    }
    return true;
}

template<typename T>
bool cTuple<T>::isLT(cTuple<T>& tuple, int n) {
    if(n == -1){
        n = tuple.n;
    }
    for(int i = 0; i < n; i++){
        if(attributes[i] != tuple.attributes[i]){
            return attributes[i] < tuple.attributes[i];
        }
    }
    return false;
}

template<typename T>
bool cTuple<T>::isGT(cTuple<T>& tuple, int n) {
    if(n == -1){
        n = tuple.n;
    }
    for(int i = 0; i < n; i++){
        if(attributes[i] != tuple.attributes[i]){
            return attributes[i] > tuple.attributes[i];
        }
    }
    return false;
}

template<typename T>
bool cTuple<T>::isLEQT(cTuple<T>& tuple, int n) {
    if(n == -1){
        n = tuple.n;
    }
    for(int i = 0; i < n; i++){
        if(attributes[i] != tuple.attributes[i]){
            return attributes[i] < tuple.attributes[i];
        }
    }
    return true;
}

template<typename T>
bool cTuple<T>::isGEQT(cTuple<T>& tuple, int n) {
    if(n == -1){
        n = tuple.n;
    }
    for(int i = 0; i < n; i++){
        if(attributes[i] != tuple.attributes[i]){
            return attributes[i] > tuple.attributes[i];
        }
    }
    return true;
}

template<typename T>
bool cTuple<T>::isTupleBetween(cTuple<T>& tuple1, cTuple<T>& tuple2, int n) {
    // Implementation for isTupleBetween method
    if(n == -1){
        n = tuple1.n;
    }
    return tuple1.isLEQT(*this, n) && tuple2.isGEQT(*this, n);
}

template<typename T>
float cTuple<T>::getTupleDistance(cTuple<T>& tuple, int n) {
    // Implementation for getTupleDistance method
    if(n == -1){
        n = tuple.n;
    }
    float distance = 0;
    for(int i = 0; i < n; i++){
        distance += (attributes[i] - tuple.attributes[i]) * (attributes[i] - tuple.attributes[i]);
    }
    return distance;
}

template<typename T>
float cTuple<T>::getTupleRangeDistance(cTuple<T>& tuple1, cTuple<T>& tuple2, int n) {
    // Implementation for getTupleRangeDistance method
    // distance is 0 if tuple is between tuple1 and tuple2, otherwise it is the distance to the closest tuple
    if(n == -1){
        n = tuple1.n;
    }
    if(isTupleBetween(tuple1, tuple2, n)){
        return 0.0;
    }
    float distance1 = getTupleDistance(tuple1, n);
    float distance2 = getTupleDistance(tuple2, n);

    return distance1 < distance2 ? distance1 : distance2;
}
