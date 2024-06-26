#pragma once
#include <stdio.h>

template<typename T>
class cTuple {
public:
    cTuple(int n);
    cTuple(int n, bool isWrapper);
    cTuple(T* attributes, int n, bool isWrapper = 0);
    ~cTuple();
    void setTuple(T* attributes, int n);
    void printTuple();
    bool isEQ(cTuple<T>& tuple, int n = -1);
    bool isLT(cTuple<T>& tuple, int n = -1);
    bool isGT(cTuple<T>& tuple, int n = -1);
    bool isLEQT(cTuple<T>& tuple, int n = -1);
    bool isGEQT(cTuple<T>& tuple, int n = -1);
    bool isTupleBetween(cTuple<T>& tuple1, cTuple<T>& tuple2, int n = -1);
    T* getAttributes() { return attributes; }
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
void cTuple<T>::printTuple() {
    printf("Tuple: ");
    for(int i = 0; i < n; i++){
        printf("%d ", attributes[i]);
    }
    printf("\n");
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

