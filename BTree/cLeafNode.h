#pragma once
#include "cNode.h"
#include "cTreeMetadata.h"
#include "cTuple.h"
#include "cInnerNode.h"
#include <iostream>
#include <cstring>

template<typename T>
class cLeafNode : public cNode<T> {
    private:
        template<typename TT> friend class cInnerNode;
    public:
    // Implementation for leaf node
    // Leaf nodes have mData structure as follows: int isLeaf, int count, T* data 
    cLeafNode(cTreeMetadata<T> * metadata) : cNode<T>(metadata) {
        this->nData = new char[metadata->nDataSizeLeaf];
        this->setCount(0);
        this->setLeafNode(true);
        metadata->leafNodeCount++;
    }
    ~cLeafNode() {
        this->metadata->leafNodeCount--;
        delete[] this->nData;
    }
    char* getElementPtr(int index) {
        // Implementation for getElementPtr method
        return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementLeafSize);
    }
    void printNodes(bool printSubtree = false, int level = 0) override{
        //print tuples in leaf node, format: [tuple1, tuple2, ...], where tuple is [attr1, attr2, ...]
        int count = this->getCount();
        if(printSubtree){
            for(int i = 0; i < level; i++){
                printf("  ");
            }
            printf("LeafNode: ");
        }
        printf("[");
        for(int i = 0; i < count; i++){
            printf("[");
            for(int j = 0; j < this->metadata->n; j++){
                printf("%d", *reinterpret_cast<T*>(this->nData+ this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * i) + j*this->metadata->attrSize));
                if(j < this->metadata->n - 1){
                    printf(", ");
                }
            }
            printf("]");
            if(i < count - 1){
                printf(", ");
            }
        }
        printf("]\n");
    }
    int insertTuple(cTuple<T>* tuple){
        // Implementation for insertTuple method
        int count = this->getCount();
        if(count >= this->metadata->maxLeafNodeElements) { return -1; }
        //insert into leaf node
        //node MUST be ordered, therefore we need to find the right place to insert and memcpy all elements to the right to clear space for new element
        // space for improvement is to use binary search
        int i = 0;
        bool found = false;
        for(; i < count; i++) {
            cTuple<T> * tempTuple = new cTuple<T>((T*)this->getElementPtr(i), this->metadata->n1);
            if(tuple->isLT(*tempTuple)){
                //i--;//go back, because we want to insert before this element
                found = true;
                break;
            }
        }
        //If position to insert was found, make space at index i
        if(found){
            memcpy(
                this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * (i + 1)), 
                this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * i), 
                this->metadata->nDataElementLeafSize * (count - i)
            );
        }
        //insert tuple at index i
        memcpy(
            this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * i), 
            tuple->attributes, 
            this->metadata->nDataElementLeafSize
        );
        this->incrementCount();
        this->metadata->tupleCount++;
        return i;
    }
    cLeafNode* split() override {
        int count = this->getCount();
        if(count < this->metadata->maxLeafNodeElements)
            return nullptr;
        cLeafNode* second = new cLeafNode<T>(this->metadata);
        //copy second half of elements to second node
        memcpy(
            second->nData + this->metadata->nDataStartBShift, 
            this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * this->metadata->halfLeafNode), 
            this->metadata->nDataElementLeafSize * (this->metadata->maxLeafNodeElements - this->metadata->halfLeafNode)
        );
        int countSecond = this->metadata->maxLeafNodeElements - this->metadata->halfLeafNode;
        //set count of second node
        second->setCount(countSecond);
        //set count of first node
        this->setCount(this->metadata->halfLeafNode);

        //take care of parent node, init if not initialized and set parent of second node, This can happen only once
        if(this->parent == nullptr){
            //create new parent node
            cNode<T>* parent = new cInnerNode<T>(this->metadata);
            
            this->parent = parent;
            //set first element of parent node, start at nDataStartBShift, save first n1 elements of first tuple from this->nData, then save first n1 elements of last tuple from this->nData and append pointerto this node
            memcpy(
                parent->nData + this->metadata->nDataStartBShift, 
                this->nData + this->metadata->nDataStartBShift, 
                this->metadata->nDataElementLeafSize
            );
            
        }
        second->parent = this->parent;
        return second;
    }
};