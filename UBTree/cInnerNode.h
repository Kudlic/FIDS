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
    cInnerNode(cTreeMetadata * metadata);
    ~cInnerNode() override;
    //method to be called, when we need to pass root to child without destroying it
    //returns child of the node and clears it so it does not get destroyed
    cNode<T>* passDown();
    cNode<T>* getChild(int index);
    char * getChildPtr(int index);
    char* getTupleLowPtr(int index);
    char* getTupleHighPtr(int index);
    char* getElementPtr(int index);
    
    //Customised for zAddr to avoid bothering with tuple types
    bool widenHighZAddr(int index, char* zAddr, cZAddrUtils *zTools);
    bool widenLowZAddr(int index, char* zAddr, cZAddrUtils *zTools);
    bool setHighZAddr(int index, char* zAddr);
    bool setLowZAddr(int index, char* zAddr);
    //Should work for zAddresses and tuples, works with stored data, makes T tuples and compare EQ, which works for both zAddr and tuples
    bool needAdjustment(int index);
    //Goes and sets range to match child's range, no comparison
    bool adjustRange(int index);

    bool addElement(cInnerNode<T>* child, int index = -1);
    bool addElement(cLeafNode<T>* child, int index = -1);
    bool addElement(cTuple<T>* tupleLow, cTuple<T>* tupleHigh, cNode<T>* child, int index = -1);
    bool deleteElement(int index);
    cNode<T>* split(int splitNodeIndex, cNode<T>* parent) override;
    cNode<T>* mergeRight(int posInParent, cNode<T>* parent) override;
    bool borrowFromLeft(int posInParent, cInnerNode<T>* parent);
    bool borrowFromRight(int posInParent, cInnerNode<T>* parent);
    //void printNodes(bool printSubtree = false, int level = 0, bool includeLinks = false) override;
};

template<typename T>
cInnerNode<T>::cInnerNode(cTreeMetadata * metadata): cNode<T>(metadata) {
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
cNode<T>* cInnerNode<T>::passDown(){
    if(this->getCount() == 0) return nullptr;
    else if(this->getCount() == 1){
        cNode<T>* child = this->getChild(0);
        this->setCount(0);
        return child;
    }
    else throw std::invalid_argument("Cannot pass down root with more than one child");
    
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
bool cInnerNode<T>::widenHighZAddr(int index, char* zAddr, cZAddrUtils *zTools) {
    if(index < 0 || index >= this->getCount())
        return false;
    //If tupleHigh is lower than tuple, widen it
    char* zAddrHigh = this->getTupleHighPtr(index);
    if(zTools->isZAddrGT(zAddr, zAddrHigh)){
        memcpy( 
            this->getTupleHighPtr(index), 
            zAddr, 
            this->metadata->zAddressBytes
        );
        return true;
    }
    return false;
}

template<typename T>
bool cInnerNode<T>::widenLowZAddr(int index, char* zAddr, cZAddrUtils *zTools) {
    if(index < 0 || index >= this->getCount())
        return false;
    //If tupleLow is Higher than tuple, widen it
    char* zAddrLow = this->getTupleLowPtr(index);
    if(zTools->isZAddrLT(zAddr, zAddrLow)){
        memcpy(
            this->getTupleLowPtr(index), 
            zAddr, 
            this->metadata->zAddressBytes
        );
        return true;
    }
    return false;
}

template<typename T>
bool cInnerNode<T>::setHighZAddr(int index, char* zAddr) {
    if(index < 0 || index >= this->getCount())
        return false;
    memcpy(
        this->getTupleHighPtr(index), 
        zAddr, 
        this->metadata->zAddressBytes
    );
    return true;
}

template<typename T>
bool cInnerNode<T>::setLowZAddr(int index, char* zAddr) {
    if(index < 0 || index >= this->getCount())
        return false;
    memcpy(
        this->getTupleLowPtr(index), 
        zAddr, 
        this->metadata->zAddressBytes
    );
    return true;
}
template<typename T>
bool cInnerNode<T>::needAdjustment(int index) {
    if(index < 0 || index >= this->getCount())
        return false;
    if(this->getChild(index)->isLeafNode()){
        cLeafNode<T>* child = dynamic_cast<cLeafNode<T>*>( this->getChild(index));
        if(child == nullptr)
            return false;
        cTuple<T> tupleLow = cTuple<T> ((T*)child->getElementPtr(0), this->metadata->n, true);
        cTuple<T> tupleHigh = cTuple<T> ((T*)child->getElementPtr(child->getCount()-1), this->metadata->n, true);
        cTuple<T> tupleLowNode = cTuple<T> ((T*)this->getTupleLowPtr(index), this->metadata->n, true);
        cTuple<T> tupleHighNode = cTuple<T> ((T*)this->getTupleHighPtr(index), this->metadata->n, true);
        return !(tupleLow.isEQ(tupleLowNode) && tupleHigh.isEQ(tupleHighNode));
    }
    else{
        cInnerNode<T>* child = dynamic_cast<cInnerNode<T>*>( this->getChild(index));
        if(child == nullptr)
            return false;
        cTuple<T> tupleLow = cTuple<T> ((T*)child->getTupleLowPtr(0), this->metadata->n, true);
        cTuple<T> tupleHigh = cTuple<T> ((T*)child->getTupleHighPtr(child->getCount()-1), this->metadata->n, true);
        cTuple<T> tupleLowNode = cTuple<T> ((T*)this->getTupleLowPtr(index), this->metadata->n, true);
        cTuple<T> tupleHighNode = cTuple<T> ((T*)this->getTupleHighPtr(index), this->metadata->n, true);
        return !(tupleLow.isEQ(tupleLowNode) && tupleHigh.isEQ(tupleHighNode));
    }
    return false;
}

template<typename T>
bool cInnerNode<T>::adjustRange(int index) {
    if(index < 0 || index >= this->getCount())
        return false;
    if(this->getChild(index)->isLeafNode()){
        cLeafNode<T>* child = dynamic_cast<cLeafNode<T>*>( this->getChild(index));
        if(child == nullptr)
            return false;
        this->setLowZAddr(index, child->getElementPtr(0));
        this->setHighZAddr(index, child->getElementPtr(child->getCount()-1));
        return true;
    }
    else{
        cInnerNode<T>* child = dynamic_cast<cInnerNode<T>*>( this->getChild(index));
        if(child == nullptr)
            return false;
        this->setLowZAddr(index, child->getTupleLowPtr(0));
        this->setHighZAddr(index, child->getTupleHighPtr(child->getCount()-1));
        return true;
    }
    return false;
}

template<typename T>
bool cInnerNode<T>::addElement(cInnerNode<T>* child, int index) {
    cTuple<T> tupleLow = cTuple<T> ((T*)child->getTupleLowPtr(0), this->metadata->n, true);
    cTuple<T> tupleHigh = cTuple<T> ((T*)child->getTupleHighPtr(child->getCount()-1), this->metadata->n, true);
    return addElement(&tupleLow, &tupleHigh, child, index);
}

template<typename T>
bool cInnerNode<T>::addElement(cLeafNode<T>* child, int index) {
    cTuple<T> tupleLow = cTuple<T> ((T*)child->getElementPtr(0), this->metadata->n, true);
    cTuple<T> tupleHigh = cTuple<T> ((T*)child->getElementPtr(child->getCount()-1), this->metadata->n, true);
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

    if(tupleLow->n < this->metadata->n || tupleHigh->n < this->metadata->n)
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
bool cInnerNode<T>::deleteElement(int index) {
    if(index < 0 || index >= this->getCount())
        return false;
    //If there are elements present, find the right place to insert
    memset(
        this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize), 
        0, 
        this->metadata->nDataElementInnerSize
    );
    memmove(
        this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize), 
        this->nData + this->metadata->nDataStartBShift + ((index + 1) * this->metadata->nDataElementInnerSize), 
        this->metadata->nDataElementInnerSize * (this->getCount() - index - 1)
    );
    this->incrementCount(-1);
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
cNode<T>* cInnerNode<T>::mergeRight(int posInParent, cNode<T>* parent){
    // Merges right sibling into this node, removes right sibling from parent node and deletes right sibling amd adjusts ranges
    // Basically, copy all elements (range and pointer to child) from right sibling to this node and unify them in parent node
    cInnerNode<T>* parentNode = dynamic_cast<cInnerNode<T>*>(parent);

    int parentCount = parentNode->getCount();
    if(parentCount == posInParent+1){
        throw std::invalid_argument("Merge node index is last element of parent node, merge is not possible");
    }
    cInnerNode<T>* sibling = dynamic_cast<cInnerNode<T>*>(parentNode->getChild(posInParent + 1));
    int count = this->getCount();
    int countSibling = sibling->getCount();
    if(count + countSibling > this->metadata->maxInnerNodeElements){
        throw std::invalid_argument("Merge is not possible, nodes have too many elements");
    }
    //copy elements from sibling to this node
    memcpy(
        this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementInnerSize * count), 
        sibling->nData + this->metadata->nDataStartBShift, 
        this->metadata->nDataElementInnerSize * countSibling
    );
    //set count of this node
    this->incrementCount(countSibling);

    //remove sibling from parent node
    parentNode->deleteElement(posInParent + 1);

    parentNode->adjustRange(posInParent);

    //prevention of child deletion
    sibling->setCount(0);
    //delete sibling node
    delete sibling;
    return parent;
}
template<typename T>
bool cInnerNode<T>::borrowFromLeft(int posInParent, cInnerNode<T>* parent){
    if(parent == nullptr || posInParent < 1) //cannot borrow from left if there is no parent or if this is the first element in parent
        return false;
    cInnerNode<T>* sibling = dynamic_cast<cInnerNode<T>*>(parent->getChild(posInParent - 1));
    if(sibling->getCount() > sibling->metadata->halfInnerNode){
        //make space for new element
        memmove(
            this->nData + this->metadata->nDataStartBShift + this->metadata->nDataElementInnerSize, 
            this->nData + this->metadata->nDataStartBShift, 
            this->metadata->nDataElementInnerSize * this->getCount()
        );
        //copy first element from sibling to this node
        memcpy(
            this->nData + this->metadata->nDataStartBShift, 
            sibling->nData + this->metadata->nDataStartBShift + (sibling->getCount() - 1) * this->metadata->nDataElementInnerSize, 
            this->metadata->nDataElementInnerSize
        );
        memset(
            sibling->nData + this->metadata->nDataStartBShift + (sibling->getCount() - 1) * this->metadata->nDataElementInnerSize, 
            0, 
            this->metadata->nDataElementInnerSize
        );
        sibling->incrementCount(-1);
        this->incrementCount();
        parent->adjustRange(posInParent - 1);
        parent->adjustRange(posInParent);
        return true;
    }
    return false;
}
template<typename T>
bool cInnerNode<T>::borrowFromRight(int posInParent, cInnerNode<T>* parent){
    if(parent == nullptr || posInParent >= parent->getCount()-1) //cannot borrow from right if there is no parent or if this is the last element in parent
        return false;
    cInnerNode<T>* sibling = dynamic_cast<cInnerNode<T>*>(parent->getChild(posInParent + 1));
    if(sibling->getCount() > sibling->metadata->halfInnerNode){
        //copy first element from sibling to this node
        memcpy(
            this->nData + this->metadata->nDataStartBShift + this->metadata->nDataElementInnerSize * this->getCount(), 
            sibling->nData + this->metadata->nDataStartBShift, 
            this->metadata->nDataElementInnerSize
        );
        //fill up space in sibling
        memmove(
            sibling->nData + this->metadata->nDataStartBShift, 
            sibling->nData + this->metadata->nDataStartBShift + this->metadata->nDataElementInnerSize, 
            this->metadata->nDataElementInnerSize * (sibling->getCount() - 1)
        );
        memset(
            sibling->nData + this->metadata->nDataStartBShift + (sibling->getCount() - 1) * this->metadata->nDataElementInnerSize, 
            0, 
            this->metadata->nDataElementInnerSize
        );
        sibling->incrementCount(-1);
        this->incrementCount();
        parent->adjustRange(posInParent);
        parent->adjustRange(posInParent + 1);
        return true;
    }
    return false;
}