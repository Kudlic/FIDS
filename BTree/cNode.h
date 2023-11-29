#pragma once
#include <iostream>
#include <cstring>
#include "cTreeMetadata.h"

template<typename T>
class cNode {
public:
    cNode(cTreeMetadata<T> *metadata) : 
        metadata(metadata) {
        }
    bool isLeafNode() {
        // Implementation for isLeafNode method
        int extractedInt = 0;
        std::memcpy(&extractedInt, nData + metadata->nDataIsLeafNodeBShift, sizeof(int));
        return extractedInt != 0;
    }
    bool setLeafNode(bool isLeafNode) {
        // Implementation for setLeafNode method
        int extractedInt = isLeafNode ? 1 : 0;
        std::memcpy(nData + metadata->nDataIsLeafNodeBShift, &extractedInt, sizeof(int));
        return isLeafNode;
    }
    int getCount() {
        // Implementation for getCount method
        int extractedInt = 0;
        std::memcpy(&extractedInt, nData + metadata->nDataCountBShift, sizeof(int));
        return extractedInt;
    }
    int setCount(int count) {
        // Implementation for setCount method
        std::memcpy(nData + metadata->nDataCountBShift, &count, sizeof(int));
        return count;
    }
    int incrementCount(int incrementBy = 1) {
        // Implementation for incrementCount method
        int count = getCount();
        count = count + incrementBy;
        std::memcpy(nData + metadata->nDataCountBShift, &count, sizeof(int));
        return count;
    }
    virtual cNode* split(int splitNodeIndex, cNode* parent) {return nullptr;}
    virtual void printNodes(bool printSubtree = false, int level = 0) {}

protected:
    char* nData; // Data of the node
    cTreeMetadata<T> * metadata; // Metadata of the tree

    template<typename TT> friend class cBpTree;
    template<typename TT> friend class cLeafNode;
    template<typename TT> friend class cInnerNode;
};