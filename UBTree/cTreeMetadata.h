#pragma once
#ifndef CTREEMETADATA_H
#define CTREEMETADATA_H
// UBTree customisation - always will store chars
class cTreeMetadata {
public:
    // Constructor
    cTreeMetadata(int n, int attributeSize, int blockSize, int maxInnerNodeElements, int maxLeafNodeElements);

    void printMetadata();
    double getBpTreeAllocatedBytes();

//switch to private later
public:
    //Constants
    int n; // Dimensions of ZAddress
    int attributeSize; // Size of an attribute in bytes
    int maxInnerNodeElements; // Maximum number of elements in a inner node
    int maxLeafNodeElements; // Maximum number of elements in a leaf node
    int nDataSizeLeaf; // Size of nData in a leaf node
    int nDataSizeInner; // Size of nData in a inner node
    int halfInnerNode; // Index of middle element of inner node, last element of first half after split
    int halfLeafNode; // Index of middle element of leaf node, last element of first half after split

    //Counters
    int order; // The order of the B+ tree
    int tupleCount; // Count of tuples in the B+ tree
    int innerElemCount; //Count of elements in inner nodes of B+ tree. Element is a pair of range and reference
    int leafNodeCount; // Count of leaf nodes in the B+ tree
    int innerNodeCount; // Count of inner nodes in the B+ tree

    //nData Byte Shifts
    int nDataCountBShift; // Count of tuples of nData in nData
    int nDataIsLeafNodeBShift; // IsLeafNode flag in nData
    int nDataNodeLinkBShift; // Link to leaf node in nData
    int nDataStartBShift; // Start of nData in nData
    int nDataInnerNodeElementRangeLowBShift; // Start of low range in inner node nData element
    int nDataInnerNodeElementRangeHighBShift; // Start of high range in inner node nData element
    int nDataInnerNodeElementChildBShift; // Start of child node in inner node nData element

    int nDataElementLeafSize; // Size of an element in a leaf node, element is tuple of n attributes
    int nDataElementInnerSize; // Size of an element in a inner node, element is lowTuple, highTuple and reference to child node
    int nDataElementInnerTupleSize; // Size of a tuple in an element in a inner node

    //UBTree specific
    int zAddressBytes;
    int zBlockSize;

    template<typename T, typename B> friend class cBpTree;
    template<typename T> friend class cNode;
    template<typename T> friend class cLeafNode;
    template<typename T> friend class cInnerNode;
    template<typename T> friend class cBpTreeIterator;
    template<typename T> friend class cZAddrUtilsTemplate;
                         friend class cZAddrUtils;
};

cTreeMetadata::cTreeMetadata(int n, int blockSize, int attributeSize, int maxInnerNodeElements, int maxLeafNodeElements) : 
    n(n),
    attributeSize(attributeSize),
    maxInnerNodeElements(maxInnerNodeElements),
    maxLeafNodeElements(maxLeafNodeElements),

    //trackers
    order(0),
    tupleCount(0),
    leafNodeCount(0),
    innerNodeCount(0),
    nDataIsLeafNodeBShift(0),
    //set position shifts
    nDataCountBShift(4),
    nDataNodeLinkBShift(8) {
        nDataStartBShift = nDataNodeLinkBShift+sizeof(char*);
        nDataSizeLeaf = (nDataStartBShift) + (attributeSize * n * maxLeafNodeElements);
        nDataSizeInner = (nDataStartBShift) + ((sizeof(char*) + (n * attributeSize * 2)) * maxInnerNodeElements);
        nDataElementLeafSize = attributeSize * n;
        nDataElementInnerSize = (n * attributeSize * 2) + sizeof(char*);
        nDataElementInnerTupleSize = n * attributeSize;
        halfInnerNode = (maxInnerNodeElements-1)/2;
        halfLeafNode = (maxLeafNodeElements-1)/2;

        nDataInnerNodeElementChildBShift = (n * attributeSize * 2);
        nDataInnerNodeElementRangeLowBShift = 0;
        nDataInnerNodeElementRangeHighBShift = (n * attributeSize);

        //UBTree specific
        zAddressBytes = n * attributeSize;
        zBlockSize = blockSize;
}

void cTreeMetadata::printMetadata() {
    std::cout << "n: " << n << std::endl;
    std::cout << "maxInnerNodeElements: " << maxInnerNodeElements << std::endl;
    std::cout << "maxLeafNodeElements: " << maxLeafNodeElements << std::endl;
    std::cout << "order: " << order << std::endl;
    std::cout << "tupleCount: " << tupleCount << std::endl;
    std::cout << "leafNodeCount: " << leafNodeCount << std::endl;
    std::cout << "innerNodeCount: " << innerNodeCount << std::endl;
    std::cout << "nDataSizeLeaf: " << nDataSizeLeaf << std::endl;
    std::cout << "nDataSizeInner: " << nDataSizeInner << std::endl;
}

double cTreeMetadata::getBpTreeAllocatedBytes(){
    return ((nDataSizeLeaf * leafNodeCount)+(nDataSizeInner * innerNodeCount));
}
#endif