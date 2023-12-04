#pragma once
#include <stdio.h>

template<typename T>
class cTuple {
public:
    cTuple(T* attributes, int n, bool isWrapper = 0) : attributes(attributes), n(n), isWrapper(isWrapper) {}
    ~cTuple() {
        if(!isWrapper){
            delete[] attributes;
        }
        else{
            attributes = nullptr;
        }
    }
    T* attributes;
    int n; // Number of attributes
    bool isWrapper;

    void printTuple() {
        printf("Tuple: ");
        for(int i = 0; i < n; i++){
            printf("%d ", attributes[i]);
        }
        printf("\n");
    }
    bool isEQ(cTuple<T>& tuple, int n = -1) {
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
    bool isLT(cTuple<T>& tuple, int n = -1) {
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
    bool isGT(cTuple<T>& tuple, int n = -1) {
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
    bool isLEQT(cTuple<T>& tuple, int n = -1) {
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
    bool isGEQT(cTuple<T>& tuple, int n = -1) {
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
    bool isTupleBetween(cTuple<T>& tuple1, cTuple<T>& tuple2, int n = -1) {
        // Implementation for isTupleBetween method
        if(n == -1){
            n = tuple1.n;
        }
        return tuple1.isLEQT(*this, n) && tuple2.isGEQT(*this, n);
    }

    float getTupleDistance(cTuple<T>& tuple, int n = -1) {
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
    float getTupleRangeDistance(cTuple<T>& tuple1, cTuple<T>& tuple2, int n = -1) {
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
};