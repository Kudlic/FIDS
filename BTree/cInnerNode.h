#pragma once
#include "cNode.h"
#include "cTreeMetadata.h"
#include "cTuple.h"
#include "cLeafNode.h"
#include <iostream>
#include <cstring>
#include <algorithm>

template<typename T> class cLeafNode;

template<typename T>
class cInnerNode : public cNode<T> {
public:
    // inner nodes have mData structure as follows: int isLeaf, int count, [range[i], cNode* child[i]], where range is (tupleLow, tupleHigh), values in child are >= tupleLow and <= tupleHigh
    cInnerNode(cTreeMetadata<T> * metadata);
    ~cInnerNode() override;
    cNode<T>* getChild(int index);
    char * getChildPtr(int index);
    char* getTupleLowPtr(int index);
    char* getTupleHighPtr(int index);
    char* getElementPtr(int index);
    bool widenHigh(int index, cTuple<T>* tuple);
    bool widenLow(int index, cTuple<T>* tuple);
    bool setHigh(int index, cTuple<T>* tupleHigh);
    bool setLow(int index, cTuple<T>* tupleLow);
    bool adjustRange(int index);
    bool addElement(cInnerNode<T>* child, int index = -1);
    bool addElement(cLeafNode<T>* child, int index = -1);
    bool addElement(cTuple<T>* tupleLow, cTuple<T>* tupleHigh, cNode<T>* child, int index = -1);
    cNode<T>* split(int splitNodeIndex, cNode<T>* parent) override;
    void printNodes(bool printSubtree = false, int level = 0, bool includeLinks = false) override;
};

template<typename T>
cInnerNode<T>::cInnerNode(cTreeMetadata<T> * metadata): cNode<T>(metadata) {
    this->nData = new char[metadata->nDataSizeInner];
    this->setCount(0);
    this->setLeafNode(false);
    this->metadata->innerNodeCount++;
}

template<typename T>
cInnerNode<T>::~cInnerNode() {
    for(int i=0; i<this->getCount(); i++){
        cNode<T>* child = this->getChild(i);
        delete child;
    }
    this->metadata->innerNodeCount--;

    delete[] this->nData;
}

template<typename T>
cNode<T>* cInnerNode<T>::getChild(int index) {
    cNode<T>* child = nullptr;
    memcpy(&child, this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementChildBShift, sizeof(cNode<T>*));
    return child;
}

template<typename T>
char* cInnerNode<T>::getChildPtr(int index) {
    return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementChildBShift;
}

template<typename T>
char* cInnerNode<T>::getTupleLowPtr(int index) {
    return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementRangeLowBShift;
}

template<typename T>
char* cInnerNode<T>::getTupleHighPtr(int index) {
    return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementRangeHighBShift;
}

template<typename T>
char* cInnerNode<T>::getElementPtr(int index) {
    return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize);
}

template<typename T>
bool cInnerNode<T>::widenHigh(int index, cTuple<T>* tuple) {
    if(index < 0 || index >= this->getCount())
        return false;
    //If tupleHigh is lower than tuple, widen it
    cTuple<T> tupleHigh = cTuple<T> ((T*)this->getTupleHighPtr(index), this->metadata->n1, true);
    if(tuple->isGT(tupleHigh)) {
        {
            memcpy(
                this->getTupleHighPtr(index), 
                tuple->attributes, 
                this->metadata->attrSize * this->metadata->n1
            );
        }
    }
    return true;
}

template<typename T>
bool cInnerNode<T>::widenLow(int index, cTuple<T>* tuple) {
    if(index < 0 || index >= this->getCount())
        return false;
    //If tupleLow is lower than tuple, widen it
    cTuple<T> tupleLow = cTuple<T> ((T*)this->getTupleLowPtr(index), this->metadata->n1, true);
    if(tuple->isLT(tupleLow)) {
        {
            memcpy(
                this->getTupleLowPtr(index), 
                tuple->attributes, 
                this->metadata->attrSize * this->metadata->n1
            );
        }
    }
    return true;
}

template<typename T>
bool cInnerNode<T>::setHigh(int index, cTuple<T>* tupleHigh) {
    if(index < 0 || index >= this->getCount())
        return false;
    memcpy(
        this->getTupleHighPtr(index), 
        tupleHigh->attributes, 
        this->metadata->attrSize * this->metadata->n1
    );
    return true;
}

template<typename T>
bool cInnerNode<T>::setLow(int index, cTuple<T>* tupleLow) {
    if(index < 0 || index >= this->getCount())
        return false;
    memcpy(
        this->getTupleLowPtr(index), 
        tupleLow->attributes, 
        this->metadata->attrSize * this->metadata->n1
    );
    return true;
}

template<typename T>
bool cInnerNode<T>::adjustRange(int index) {
    //TODO: implement for inner node type children
    if(index < 0 || index >= this->getCount())
        return false;
    if(this->getChild(index)->isLeafNode()){
        cLeafNode<T>* child = dynamic_cast<cLeafNode<T>*>( this->getChild(index));
        if(child == nullptr)
            return false;
        cTuple<T> tupleLow = cTuple<T> ((T*)child->getElementPtr(0), this->metadata->n1, true);
        cTuple<T> tupleHigh = cTuple<T> ((T*)child->getElementPtr(child->getCount()-1), this->metadata->n1, true);
        this->setLow(index, &tupleLow);
        this->setHigh(index, &tupleHigh);
        return true;
    }
    else{
        cInnerNode<T>* child = dynamic_cast<cInnerNode<T>*>( this->getChild(index));
        if(child == nullptr)
            return false;
        cTuple<T> tupleLow = cTuple<T> ((T*)child->getTupleLowPtr(0), this->metadata->n1, true);
        cTuple<T> tupleHigh = cTuple<T> ((T*)child->getTupleHighPtr(child->getCount()-1), this->metadata->n1, true);
        this->setLow(index, &tupleLow);
        this->setHigh(index, &tupleHigh);
        return true;
    }
    return false;
}

template<typename T>
bool cInnerNode<T>::addElement(cInnerNode<T>* child, int index) {
    cTuple<T> tupleLow = cTuple<T> ((T*)child->getTupleLowPtr(0), this->metadata->n1, true);
    cTuple<T> tupleHigh = cTuple<T> ((T*)child->getTupleHighPtr(child->getCount()-1), this->metadata->n1, true);
    return addElement(&tupleLow, &tupleHigh, child, index);
}

template<typename T>
bool cInnerNode<T>::addElement(cLeafNode<T>* child, int index) {
    cTuple<T> tupleLow = cTuple<T> ((T*)child->getElementPtr(0), this->metadata->n1, true);
    cTuple<T> tupleHigh = cTuple<T> ((T*)child->getElementPtr(child->getCount()-1), this->metadata->n1, true);
    return addElement(&tupleLow, &tupleHigh, child, index);
}

template<typename T>
bool cInnerNode<T>::addElement(cTuple<T>* tupleLow, cTuple<T>* tupleHigh, cNode<T>* child, int index) {
    if(index < 0){
        index = this->getCount();
    }
    else{
        if(index > this->getCount())
            return false;
        //Make space for new element
        memmove(
            this->nData + this->metadata->nDataStartBShift + ((index + 1) * this->metadata->nDataElementInnerSize), 
            this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize), 
            this->metadata->nDataElementInnerSize * (this->getCount() - index)
        );
    }

    if(tupleLow->n < this->metadata->n1 || tupleHigh->n < this->metadata->n1)
        return false;
    //If there are elements present, find the right place to insert
    memcpy(
        this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize)+ this->metadata->nDataInnerNodeElementRangeLowBShift, 
        tupleLow->attributes, 
        this->metadata->nDataElementInnerTupleSize
    );
    memcpy(
        this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementRangeHighBShift, 
        tupleHigh->attributes, 
        this->metadata->nDataElementInnerTupleSize
    );
    memcpy(
        this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + (this->metadata->nDataInnerNodeElementChildBShift), 
        &child, 
        sizeof(cNode<T>*)
    );
    this->incrementCount();
    return true;
}

template<typename T>
cNode<T>* cInnerNode<T>::split(int splitNodeIndex, cNode<T>* parent) {
    // Splits the InnerNode into two InnerNodes,
    // If there is no parent node, creates a new parent node and initializes first range and sets split child pointer to it
    // Sets parent of second node to parent of first node, inserts second node and its range into parent node
    // Returns parent node
    int count = this->getCount();
    if(count < this->metadata->maxInnerNodeElements)
        return nullptr;
    cInnerNode<T>* second = new cInnerNode<T>(this->metadata);
    //copy second half of elements to second node
    memcpy(
        second->nData + this->metadata->nDataStartBShift, 
        this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementInnerSize * this->metadata->halfInnerNode), 
        this->metadata->nDataElementInnerSize * (this->metadata->maxInnerNodeElements - this->metadata->halfInnerNode)
    );
    int countSecond = this->metadata->maxInnerNodeElements - this->metadata->halfInnerNode;
    //set count of second node
    second->setCount(countSecond);
    //set count of first node
    this->setCount(this->metadata->halfInnerNode);

    cInnerNode<T>* parentNode = dynamic_cast<cInnerNode<T>*>(parent);
    //take care of parent node, init if not initialized and set parent of second node, This can happen only once
    if(parentNode == nullptr){
        //create new parent node
        parentNode = new cInnerNode<T>(this->metadata);
        this->metadata->order++;
        //set first element of parent node, also describable as first InnerNode element and parent of split leafNode
        parentNode->addElement(this);
    }
    else{
        parentNode->adjustRange(splitNodeIndex);
    }
    parentNode->addElement(second, splitNodeIndex+1);
    return parentNode;
}

template<typename T>
void cInnerNode<T>::printNodes(bool printSubtree, int level, bool includeLinks) {
    //print tuples in leaf node, format: [tuple1, tuple2, ...], where tuple is [attr1, attr2, ...]
    int count = this->getCount();
    if(printSubtree){
        for(int i = 0; i < level; i++){
            printf("_  ");
        }
        printf("InnerNode: ");
    }
    printf("[");
    for(int i = 0; i < count; i++){
        printf("([");
        char* tupleLow = this->getTupleLowPtr(i);
        char* tupleHigh = this->getTupleHighPtr(i);
        for(int j = 0; j < this->metadata->n1; j++){
            printf("%d", *reinterpret_cast<T*>(tupleLow + j*this->metadata->attrSize));
            if(j < this->metadata->n1 - 1){
                printf(", ");
            }
        }
        printf("],[");
        for(int j = 0; j < this->metadata->n1; j++){
            printf("%d", *reinterpret_cast<T*>(tupleHigh + j*this->metadata->attrSize));
            if(j < this->metadata->n1 - 1){
                printf(", ");
            }
        }
        printf("])");
        if(i < count - 1){
            printf(", ");
        }
    }
    printf("]\n");
    if(printSubtree){
        for(int i = 0; i < count; i++){
            cNode<T>* child = this->getChild(i);
            if(child != nullptr)
                child->printNodes(printSubtree, level + 1, includeLinks);
        }
    }
}
        