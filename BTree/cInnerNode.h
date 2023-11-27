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
    cInnerNode(cTreeMetadata<T> * metadata): cNode<T>(metadata) {
        this->nData = new char[metadata->nDataSizeInner];
        this->setCount(0);
        this->setLeafNode(false);
        this->metadata->innerNodeCount++;
    }
    ~cInnerNode() {
        this->metadata->innerNodeCount--;
        delete[] this->nData;
    }
    cNode<T>* getChild(int index) {
        cNode<T>* child = nullptr;
        memcpy(&child, this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementChildBShift, sizeof(cNode<T>*));
        return child;
    }
    char * getChildPtr(int index) {
        return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementChildBShift;
    }
    char* getTupleLowPtr(int index) {
        return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementRangeLowBShift;
    }
    char* getTupleHighPtr(int index) {
        return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataInnerNodeElementRangeHighBShift;
    }
    char* getElementPtr(int index) {
        return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize);
    }
    bool widenHigh(int index, cTuple<T>* tuple) {
        if(index < 0 || index >= this->getCount())
            return false;
        //If tupleHigh is lower than tuple, widen it
        cTuple<T> *tupleHigh = new cTuple<T> ((T*)this->getTupleHighPtr(index), this->metadata->n1);
        if(tuple->isGT(*tupleHigh)) {
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
    bool widenLow(int index, cTuple<T>* tuple) {
        if(index < 0 || index >= this->getCount())
            return false;
        //If tupleLow is lower than tuple, widen it
        cTuple<T> *tupleLow = new cTuple<T> ((T*)this->getTupleLowPtr(index), this->metadata->n1);
        if(tuple->isLT(*tupleLow)) {
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
    bool setHigh(int index, cTuple<T>* tupleHigh) {
        if(index < 0 || index >= this->getCount())
            return false;
        memcpy(
            this->getTupleHighPtr(index), 
            tupleHigh->attributes, 
            this->metadata->attrSize * this->metadata->n1
        );
        return true;
    }
    bool setLow(int index, cTuple<T>* tupleLow) {
        if(index < 0 || index >= this->getCount())
            return false;
        memcpy(
            this->getTupleLowPtr(index), 
            tupleLow->attributes, 
            this->metadata->attrSize * this->metadata->n1
        );
        return true;
    }
    bool adjustRange(int index) {
        //Adjust range should check for first elements that are not equal to its parent and adjust range accordingly
        //TODO: implement for inner node type children
        if(index < 0 || index >= this->getCount())
            return false;
        cLeafNode<T>* child = dynamic_cast<cLeafNode<T>*>( this->getChild(index));
        if(child == nullptr)
            return false;
        cTuple<T> *tupleLow = new cTuple<T> ((T*)child->getElementPtr(0), this->metadata->n1);
        cTuple<T> *tupleHigh = new cTuple<T> ((T*)child->getElementPtr(child->getCount()-1), this->metadata->n1);
        this->setLow(index, tupleLow);
        this->setHigh(index, tupleHigh);
        return true;
    }

    bool addElement(cTuple<T>* tupleLow, cTuple<T>* tupleHigh, cNode<T>* child, int index = -1) {
        // Implementation for addElement method, finds correct possition
        if(index < 0){
            index = this->getCount();
        }
        else{
            if(index > this->getCount())
                return false;
            //Make space for new element
            memcpy(
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
        // TODO: check for overflow
        this->incrementCount();
        if(this->getCount() >= this->metadata->maxInnerNodeElements)
            printf("Overflow, InnerNode split should be there!\n");
        return true;
    }
    //TODO: implement insert for inner node type children
    int insertTuple(cTuple<T>* tuple) override {
        int count = this->getCount();
        if(count >= this->metadata->maxInnerNodeElements)
            return -1;
        // Implementation for insertTuple method
        // if node is empty, perform node Initialisation
        if(count == 0) {
            cLeafNode<T>* leafNode = new cLeafNode<T>(this->metadata);
            leafNode->parent = this;
            leafNode->insertTuple(tuple);
            int retVal = addElement(tuple, tuple, leafNode);
            return retVal;
        }

        //If there are elements present, find the right place to insert
        //Because we are working with tuples, we cannot use distance, instead we will use tuple LT comparison and first smaller element will be the right place to insert and increase range of
        else{
            cLeafNode<T>* child = nullptr;
            int childIndex = -1;
            for(int i = 0; i < count; i++) {
                //TODO: ensure we are not wasting memory here
                char * tupleLow = this->getTupleLowPtr(i);
                char * tupleHigh = this->getTupleHighPtr(i);
                cTuple<T> *tupleLowObj = new cTuple<T> ((T*)tupleLow, this->metadata->n1);
                cTuple<T> *tupleHighObj = new cTuple<T> ((T*)tupleHigh, this->metadata->n1);

                if(tuple->isLT(*tupleLowObj)) {
                    if(i == 0){
                        childIndex = 0;
                        this->widenLow(childIndex, tuple);
                    }
                    else{
                        childIndex = i-1;
                        this->widenHigh(childIndex, tuple);
                    }
                    child = dynamic_cast<cLeafNode<T>*>(this->getChild(childIndex));
                    break;
                }
                else if(tuple->isTupleBetween(*tupleLowObj, *tupleHighObj)) {
                    childIndex = i;
                    child = dynamic_cast<cLeafNode<T>*>(this->getChild(childIndex));
                    break;
                }
            }   
            if(child == nullptr) {
                childIndex = count - 1;
                this->widenHigh(childIndex, tuple);
                child = dynamic_cast<cLeafNode<T>*>(this->getChild(childIndex));
            }
            child->insertTuple(tuple);
            int countAftIns = child->getCount();
            //Check for overflow, if overflow, split child node and insert second node into parent
            if (countAftIns >= this->metadata->maxLeafNodeElements) {
                //TODO: make a adjustRange method, which will adjust range of parent node based on its children
                //split child node
                cLeafNode<T>* second = child->split();
                //insert second node into parent
                cTuple<T> *tupleLow = new cTuple<T> ((T*)second->getElementPtr(0), this->metadata->n1);
                cTuple<T> *tupleHigh = new cTuple<T> ((T*)second->getElementPtr(second->getCount()-1), this->metadata->n1);
                this->addElement(tupleLow, tupleHigh, second, childIndex + 1);
                this->adjustRange(childIndex);
            }
            return childIndex;
        }
    }
    void printNodes(bool printSubtree = false, int level = 0) override{
        //print tuples in leaf node, format: [tuple1, tuple2, ...], where tuple is [attr1, attr2, ...]
        int count = this->getCount();
        if(printSubtree){
            for(int i = 0; i < level; i++){
                printf("  ");
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
                    child->printNodes(printSubtree, level + 1);
            }
        }
    }
};