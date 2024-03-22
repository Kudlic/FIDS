#pragma once

// UBTree customisation - always will store chars
template<typename TT>
class cTreeMetadata {
public:
    // Constructor
    cTreeMetadata(int n, int maxInnerNodeElements, int maxLeafNodeElements);

    void printMetadata();
    double getBpTreeAllocatedBytes();

//switch to private later
public:
    //Constants
    int n; // Dimensions of ZAddress
    int maxInnerNodeElements; // Maximum number of elements in a inner node
    int maxLeafNodeElements; // Maximum number of elements in a leaf node
    int dimSize; // Size of an dimension in bytes
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
    int dataTypeBytes;
    int zAddressBytes;

    template<typename T, typename B> friend class cBpTree;
    template<typename T> friend class cNode;
    template<typename T> friend class cLeafNode;
    template<typename T> friend class cInnerNode;
    template<typename T> friend class cBpTreeIterator;
};


template<typename TT>
cTreeMetadata<TT>::cTreeMetadata(int n, int maxInnerNodeElements, int maxLeafNodeElements) : 
    n(n),
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
        dimSize = sizeof(TT);
        nDataSizeLeaf = (nDataStartBShift) + (dimSize * n * maxLeafNodeElements);
        nDataSizeInner = (nDataStartBShift) + ((sizeof(char*) + (n * dimSize * 2)) * maxInnerNodeElements);
        nDataElementLeafSize = dimSize * n;
        nDataElementInnerSize = (n * dimSize * 2) + sizeof(char*);
        nDataElementInnerTupleSize = n * dimSize;
        halfInnerNode = (maxInnerNodeElements-1)/2;
        halfLeafNode = (maxLeafNodeElements-1)/2;

        nDataInnerNodeElementChildBShift = (n * dimSize * 2);
        nDataInnerNodeElementRangeLowBShift = 0;
        nDataInnerNodeElementRangeHighBShift = (n * dimSize);

        //UBTree specific
        dataTypeBytes = sizeof(TT);
        zAddressBytes = n * dataTypeBytes;

}

template<typename TT>
void cTreeMetadata<TT>::printMetadata() {
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

template<typename TT>
double cTreeMetadata<TT>::getBpTreeAllocatedBytes(){
    return ((nDataSizeLeaf * leafNodeCount)+(nDataSizeInner * innerNodeCount));
}
