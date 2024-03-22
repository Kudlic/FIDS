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
    cLeafNode(cTreeMetadata * metadata);
    ~cLeafNode() override;
    char* getElementPtr(int index);
    cLeafNode<T>* getNodeLink();
    char* setNodeLink(cLeafNode<T>* nodeLink);
    //void printNodes(bool printSubtree = false, int level = 0, bool includeLinks = false) override;
    int insertZAddr(char* data, cZAddrUtils* zTools);
    int deleteZAddr(char* data, cZAddrUtils* zTools);
    cNode<T>* split(int splitNodeIndex, cNode<T>* parent) override;
    cNode<T>* mergeRight(int posInParent, cNode<T>* parent) override;
};

template<typename T>
cLeafNode<T>::cLeafNode(cTreeMetadata * metadata) : cNode<T>(metadata) {
    this->nData = new char[metadata->nDataSizeLeaf];
    this->setCount(0);
    this->setLeafNode(true);
    this->setNodeLink(nullptr);
    metadata->leafNodeCount++;
}

template<typename T>
cLeafNode<T>::~cLeafNode() {
    this->metadata->leafNodeCount--;
    delete[] this->nData;
}

template<typename T>
char* cLeafNode<T>::getElementPtr(int index) {
    return this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementLeafSize);
}

template<typename T>
cLeafNode<T>* cLeafNode<T>::getNodeLink() {
    cLeafNode<T>* child = nullptr;
    memcpy(&child, this->nData + this->metadata->nDataNodeLinkBShift, sizeof(char*));  
    return child;
}

template<typename T>
char* cLeafNode<T>::setNodeLink(cLeafNode<T>* nodeLink) {
    std::memcpy(this->nData + this->metadata->nDataNodeLinkBShift, &nodeLink, sizeof(char*));
    return (char*)nodeLink;
}

template<typename T>
int cLeafNode<T>::insertZAddr(char* data, cZAddrUtils* zTools) {
    // Implementation for insertTuple method
    int count = this->getCount();
    if(count >= this->metadata->maxLeafNodeElements) { return -1; }
    //insert into leaf node
    //node MUST be ordered, therefore we need to find the right place to insert and memcpy all elements to the right to clear space for new element
    // space for improvement is to use binary search
    int i = 0;
    bool found = false;
    for(; i < count; i++) {
        if(zTools->isZAddrLT(data, this->getElementPtr(i))){
            //i--;//go back, because we want to insert before this element
            found = true;
            break;
        }
    }
    //If position to insert was found, make space at index i
    if(found){
        memmove(
            this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * (i + 1)), 
            this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * i), 
            this->metadata->nDataElementLeafSize * (count - i)
        );
    }
    //insert tuple at index i
    memcpy(
        this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * i), 
        data, 
        this->metadata->nDataElementLeafSize
    );
    this->incrementCount();
    this->metadata->tupleCount++;
    return i;
}

template<typename T>
int cLeafNode<T>::deleteZAddr(char* data, cZAddrUtils* zTools) {
    // Implementation for deleteTuple method
    int count = this->getCount();
    if(count == 0) { return -1; }
    int i = 0;
    bool found = false;
    for(; i < count; i++) {
        if(zTools->isZAddrEQ(data, this->getElementPtr(i))){
            found = true;
            break;
        }
    }
    //If position to delete was found, fill space at index i
    if(found){
        memmove(
            this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * i), 
            this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * (i + 1)), 
            this->metadata->nDataElementLeafSize * (count - i - 1)
        );
        this->incrementCount(-1);
        this->metadata->tupleCount--;
        return i;
    }
    return -1;
}

template<typename T>
cNode<T>* cLeafNode<T>::split(int splitNodeIndex, cNode<T>* parent) {
    // Splits the LeafNode into two LeafNodes,
    // If there is no parent node, creates a new parent node and initializes first range and sets split child pointer to it
    // Sets parent of second node to parent of first node, inserts second node and its range into parent node
    // Returns parent node
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
    
    second->setNodeLink(this->getNodeLink());
    this->setNodeLink(second);
    return parentNode;
}

template<typename T>
cNode<T>* cLeafNode<T>::mergeRight(int posInParent, cNode<T>* parent){
    // Merges right sibling,
    // If there is no parent node, no merge occurs (only possible when leafNode is root node)
    // If there is a parent node, merges right sibling into this node, removes right sibling from parent node and deletes right sibling amd adjusts range of this node
    cInnerNode<T>* parentNode = dynamic_cast<cInnerNode<T>*>(parent);

    if(parent == nullptr){
        return nullptr;
    }
    int parentCount = parentNode->getCount();
    if(parentCount == posInParent+1){
        throw std::invalid_argument("Merge node index is last element of parent node, merge is not possible");
    }
    cLeafNode<T>* sibling = dynamic_cast<cLeafNode<T>*>(parentNode->getChild(posInParent + 1));
    int count = this->getCount();
    int countSibling = sibling->getCount();
    if(count + countSibling > this->metadata->maxLeafNodeElements){
        throw std::invalid_argument("Merge is not possible, nodes have too many elements");
    }
    //copy elements from sibling to this node
    memcpy(
        this->nData + this->metadata->nDataStartBShift + (this->metadata->nDataElementLeafSize * count), 
        sibling->nData + this->metadata->nDataStartBShift, 
        this->metadata->nDataElementLeafSize * countSibling
    );
    //set count of this node
    this->incrementCount(countSibling);

    //remove sibling from parent node
    parentNode->deleteElement(posInParent + 1);

    //set node link of this node
    this->setNodeLink(sibling->getNodeLink());
    parentNode->adjustRange(posInParent);
    //delete sibling node
    delete sibling;
    return parent;
}
