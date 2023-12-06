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
    ~cInnerNode() override{
        for(int i=0; i<this->getCount(); i++){
            cNode<T>* child = this->getChild(i);
            delete child;
        }
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
    //Less strict setHigh. Sets tuple only if it is higher than current tupleHigh
    bool widenHigh(int index, cTuple<T>* tuple) {
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
    //Less strict setLow. Sets tuple only if it is lower than current tupleLow
    bool widenLow(int index, cTuple<T>* tuple) {
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
    //Strict setHigh. Sets tupleHigh to tuple.
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
    //Strict setLow. Sets tupleLow to tuple.
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
    //Unconditionally adjusts range at index based on children on index
    bool adjustRange(int index) {
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

    bool addElement(cInnerNode<T>* child, int index = -1) {
        cTuple<T> tupleLow = cTuple<T> ((T*)child->getTupleLowPtr(0), this->metadata->n1, true);
        cTuple<T> tupleHigh = cTuple<T> ((T*)child->getTupleHighPtr(child->getCount()-1), this->metadata->n1, true);
        return addElement(&tupleLow, &tupleHigh, child, index);
    }
    bool addElement(cLeafNode<T>* child, int index = -1) {
        cTuple<T> tupleLow = cTuple<T> ((T*)child->getElementPtr(0), this->metadata->n1, true);
        cTuple<T> tupleHigh = cTuple<T> ((T*)child->getElementPtr(child->getCount()-1), this->metadata->n1, true);
        return addElement(&tupleLow, &tupleHigh, child, index);
    }
    //Adds element specified by tuple range and child pointer to the node at index. 
    //If index is not specified, adds element to the end of the node.
    bool addElement(cTuple<T>* tupleLow, cTuple<T>* tupleHigh, cNode<T>* child, int index = -1) {
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
    cNode<T>* split(int splitNodeIndex, cNode<T>* parent) override {
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
    void printNodes(bool printSubtree = false, int level = 0, bool includeLinks = false) override{
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
};