#pragma once
#include <iostream>
#include <cstring>
#include "cTreeMetadata.h"

template<typename T>
class cNode {
public:
    cNode(cTreeMetadata<T> *metadata);
    virtual ~cNode();
    bool isLeafNode();
    bool setLeafNode(bool isLeafNode);
    int getCount();
    int setCount(int count);
    int incrementCount(int incrementBy = 1);
    virtual cNode* split(int splitNodeIndex, cNode* parent);
    virtual cNode* mergeRight(int splitNodeIndex, cNode* parent);
    virtual void printNodes(bool printSubtree = false, int level = 0, bool includeLinks = false) {}

protected:
    char* nData; // Data of the node
    cTreeMetadata<T> * metadata; // Metadata of the tree

    template<typename TT> friend class cBpTree;
    template<typename TT> friend class cLeafNode;
    template<typename TT> friend class cInnerNode;
};
template<typename T>
cNode<T>::cNode(cTreeMetadata<T> *metadata) : 
    metadata(metadata) {
}

template<typename T>
cNode<T>::~cNode() {
    // Implementation for destructor
}

template<typename T>
bool cNode<T>::isLeafNode() {
    // Implementation for isLeafNode method
    int extractedInt = 0;
    std::memcpy(&extractedInt, nData + metadata->nDataIsLeafNodeBShift, sizeof(int));
    return extractedInt != 0;
}

template<typename T>
bool cNode<T>::setLeafNode(bool isLeafNode) {
    // Implementation for setLeafNode method
    int extractedInt = isLeafNode ? 1 : 0;
    std::memcpy(nData + metadata->nDataIsLeafNodeBShift, &extractedInt, sizeof(int));
    return isLeafNode;
}

template<typename T>
int cNode<T>::getCount() {
    // Implementation for getCount method
    int extractedInt = 0;
    std::memcpy(&extractedInt, nData + metadata->nDataCountBShift, sizeof(int));
    return extractedInt;
}

template<typename T>
int cNode<T>::setCount(int count) {
    // Implementation for setCount method
    std::memcpy(nData + metadata->nDataCountBShift, &count, sizeof(int));
    return count;
}

template<typename T>
int cNode<T>::incrementCount(int incrementBy) {
    // Implementation for incrementCount method
    int count = getCount();
    count = count + incrementBy;
    std::memcpy(nData + metadata->nDataCountBShift, &count, sizeof(int));
    return count;
}

template<typename T>
cNode<T>* cNode<T>::split(int splitNodeIndex, cNode<T>* parent) {
    // Implementation for split method
    return nullptr;
}
template<typename T>
cNode<T>* cNode<T>::mergeRight(int mergeNodeIndex, cNode<T>* parent) {
    // Implementation for merge method
    return nullptr;
}

