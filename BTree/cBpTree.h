/*
Give me some base for cBpTree class implementation of B+ tree, which will include root node cNode(which can have child types cLeafNode or cRootNode, 
cNode itself should only include common nData char* pointer, where all data of nodes will be stored, including its count of elements on the start of nData). 
In its constructor there will be tupLen attribute, specifying size of cTuple (class encapsulating n*type array, include its specification). 
We will also need to specify number of indexed and nonindexed attributes. For simplicity, we can assume n1 will be number of indexed attributes, 
which will be first n tupple attributes. n2 will be rest of the tuple. We also need to specify max RootNode and LeafNode elements. 
All important information will be stored in object of cTreeMetadata class, 
which I also want you to create, please include even tree order and height. Tree itself should only contain the root node and methods for work with it. In implementation, I plan to always 
start the BpTree with cLeafNode. After it gets filled up, then I will create cRootNode based on it and do a split. 
cTuple should be typed, so I can specify its type in constructor. However, the nData array should be char*. nData will fit n * type + (2 * sizeof(int)) to store count of elements.
Likewise, cNode should be typed, so I can specify its type in constructor and determine size of nData.
I need to implement insert and search methods for B+ tree.
If possible, avoid vectors. 
Only arrays should be used.
*/
#include <iostream>
#include <cstring>

template<typename T> class cBpTree;
template<typename T> class cNode;
template<typename T> class cLeafNode;
template<typename T> class cRootNode;

template<typename TT>
class cTreeMetadata {
public:
    // Constructor
    cTreeMetadata(int n, int n1, int n2, int maxRootNodeElements, int maxLeafNodeElements) : 
        n(n), 
        n1(n1), 
        n2(n2), 
        maxRootNodeElements(maxRootNodeElements),
        maxLeafNodeElements(maxLeafNodeElements),
        order(0),
        tupleCount(0),
        leafNodeCount(0),
        rootNodeCount(0),
        nDataIsLeafNodeBShift(0),
        nDataCountBShift(4),
        nDataStartBShift(8) {
            attrSize = sizeof(TT);
            nDataSizeLeaf = (nDataStartBShift) + (attrSize * n * maxLeafNodeElements);
            nDataSizeRoot = (nDataStartBShift) + (sizeof(char*) * (n + 1)) + (sizeof(TT) * n * maxRootNodeElements);
            nDataElementLeafSize = attrSize * n;
            nDataElementRootSize = (n1 * attrSize);
        }
    void printMetadata() {
        std::cout << "n: " << n << std::endl;
        std::cout << "n1: " << n1 << std::endl;
        std::cout << "n2: " << n2 << std::endl;
        std::cout << "maxRootNodeElements: " << maxRootNodeElements << std::endl;
        std::cout << "maxLeafNodeElements: " << maxLeafNodeElements << std::endl;
        std::cout << "order: " << order << std::endl;
        std::cout << "tupleCount: " << tupleCount << std::endl;
        std::cout << "leafNodeCount: " << leafNodeCount << std::endl;
        std::cout << "rootNodeCount: " << rootNodeCount << std::endl;
        std::cout << "nDataIsLeafNodeBShift: " << nDataIsLeafNodeBShift << std::endl;
        std::cout << "nDataCountBShift: " << nDataCountBShift << std::endl;
        std::cout << "nDataStartBShift: " << nDataStartBShift << std::endl;
        std::cout << "nDataSizeLeaf: " << nDataSizeLeaf << std::endl;
        std::cout << "nDataSizeRoot: " << nDataSizeRoot << std::endl;
        std::cout << "nDataElementLeafSize: " << nDataElementLeafSize << std::endl;
        std::cout << "nDataElementRootSize: " << nDataElementRootSize << std::endl;
    }

private:
    //Constants
    int n; // Number of attributes
    int n1; // Number of indexed attributes
    int n2; // Number of non-indexed attributes
    int maxRootNodeElements; // Maximum number of elements in a root node
    int maxLeafNodeElements; // Maximum number of elements in a leaf node
    int attrSize; // Size of an attribute
    int nDataSizeLeaf; // Size of nData in a leaf node
    int nDataSizeRoot; // Size of nData in a root node

    //Counters
    int order; // The order of the B+ tree
    int tupleCount; // Count of tuples in the B+ tree
    int leafNodeCount; // Count of leaf nodes in the B+ tree
    int rootNodeCount; // Count of root nodes in the B+ tree

    //nData Byte Shifts
    int nDataCountBShift; // Count of tuples of nData in nData
    int nDataIsLeafNodeBShift; // IsLeafNode flag in nData
    int nDataStartBShift; // Start of nData in nData

    int nDataElementLeafSize; // Size of an element in a leaf node, element is tuple of n attributes
    int nDataElementRootSize; // Size of an element in a root node, element is tuple of indexed n (n1) attributes

    template<typename T> friend class cBpTree;
    template<typename T> friend class cNode;
    template<typename T> friend class cLeafNode;
    template<typename T> friend class cRootNode;
};

template<typename T>
class cTuple {
public:
    cTuple(T* attributes, int n) : attributes(attributes), n(n) {}
    ~cTuple() {
        delete[] attributes;
    }
    T* attributes;
    int n; // Number of attributes

    void printTuple() {
        printf("Tuple: ");
        for(int i = 0; i < n; i++){
            printf("%d ", attributes[i]);
        }
        printf("\n");
    }
};

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
    int getCount() {
        // Implementation for getCount method
        int extractedInt = 0;
        std::memcpy(&extractedInt, nData + metadata->nDataCountBShift, sizeof(int));
        return extractedInt;
    }

protected:
    char* nData; // Data of the node
    cTreeMetadata<T> * metadata; // Metadata of the tree

    template<typename TT> friend class cBpTree;
};

template<typename T>
class cLeafNode : public cNode<T> {
    public:
    // Implementation for leaf node
    // Leaf nodes have mData structure as follows: int isLeaf, int count, T* data 
    cLeafNode(cTreeMetadata<T> * metadata) : cNode<T>(metadata) {
        this->nData = new char[metadata->nDataSizeLeaf];
        int zero = 0;
        int one = 1;
        std::memcpy(this->nData + metadata->nDataIsLeafNodeBShift, &one, sizeof(int));
        std::memcpy(this->nData + metadata->nDataCountBShift, &zero, sizeof(int));
        metadata->leafNodeCount++;
    }
    ~cLeafNode() {
        delete[] this->nData;
    }
    void printTuples(){
        //print tuples in leaf node, format: [tuple1, tuple2, ...], where tuple is [attr1, attr2, ...]
        int count = this->getCount();
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
};

template<typename T>
class cRootNode : public cNode<T> {
    public:
    // Implementation for root node
    // Leaf nodes have mData structure as follows: int isLeaf, int count, cNode* child[0], T data[0], cNode* child[1], T data[1], ... , T data[n-1], cNode* child[n]
    cRootNode(cTreeMetadata<T> * metadata): cNode<T>(metadata) {
        this->nData = new char[metadata->nDataSizeRoot];
        int zero = 0;
        std::memcpy(this->nData + metadata->nDataIsLeafNodeBShift, &zero, sizeof(int));
        std::memcpy(this->nData + metadata->nDataCountBShift, &zero, sizeof(int));
        this->metadata->rootNodeCount++;
    }
    ~cRootNode() {
        delete[] this->nData;
    }
    cNode<T>* getChildPtr(int index) {
        // Implementation for getChildPtr method
        return (cNode<T>*) this->nData + this->metadata->nDataStartBShift + (sizeof(cNode<T>*) * index) + (this->metadata->attrSize * index);
    }
    char* getElementPtr(int index) {
        // Implementation for getElementPtr method
        return this->nData + this->metadata->nDataStartBShift + (sizeof(cNode<T>*) * (index + 1)) + (this->metadata->attrSize * index);
    }
};


template<typename T>
class cBpTree {
private:
    cNode<T>* root;
    cTreeMetadata<T> * metadata;

public:
    cBpTree(int tupLen, int n1, int n2, int maxRootNodeElements, int maxLeafNodeElements){
        //start with leaf node, after it overflows, it will be split into root node
        this->metadata = new cTreeMetadata<T>(tupLen, n1, n2, maxRootNodeElements, maxLeafNodeElements);
        this->root = new cLeafNode<T>(this->metadata);
    }

    ~cBpTree() {
        delete this->root;
    }

    bool insert(cTuple<T>& tuple) {
        // Implementation for insert method
        cNode<T>* node = root;
        while (node != nullptr) {
            bool isLeafNode = node->isLeafNode();

            if(isLeafNode) {
                cLeafNode<T>* leafNode = (cLeafNode<T>*)node;
                int count = leafNode->getCount();
                if(count < metadata->maxLeafNodeElements) {
                    //insert into leaf node
                    //node MUST be ordered, therefore we need to find the right place to insert and memcpy all elements to the right to clear space for new element
                    // space for improvement is to use binary search
                    int i = 0;
                    bool found = false;
                    for(; i < count && !found; i++) {
                        char* nodeElement = leafNode->nData + metadata->nDataStartBShift + (metadata->nDataElementLeafSize * i);
                        for(int j = 0; j < metadata->n1; j++) {
                            //printf("tuple.attributes[%d]: %d, nodeElement[%d]: %d\n", j, tuple.attributes[j], j, nodeElement[j*metadata->attrSize]);
                            if(tuple.attributes[j] < nodeElement[j*metadata->attrSize]) {
                                found = true;
                                break;
                            } else if(tuple.attributes[j] >= nodeElement[j*metadata->attrSize]) {
                                continue;
                            }
                        }
                    }
                    if(found){
                        i--;//because break on found still increments i
                        //leafNode->printTuples();
                        memcpy(
                            leafNode->nData + metadata->nDataStartBShift + (metadata->nDataElementLeafSize * (i + 1)), 
                            leafNode->nData + metadata->nDataStartBShift + (metadata->nDataElementLeafSize * i), 
                            metadata->nDataElementLeafSize * (count - i)
                        );
                        //leafNode->printTuples();
                        memcpy(
                            leafNode->nData + metadata->nDataStartBShift + (metadata->nDataElementLeafSize * i), 
                            tuple.attributes, 
                            metadata->nDataElementLeafSize
                        );
                        //leafNode->printTuples();
                    }
                    else{
                        //printf("Saving %d bytes to %p\n", metadata->nDataElementLeafSize, leafNode->nData + metadata->nDataStartBShift + (metadata->nDataElementLeafSize * count));
                        memcpy(
                            leafNode->nData + metadata->nDataStartBShift + (metadata->nDataElementLeafSize * count), 
                            tuple.attributes, 
                            metadata->nDataElementLeafSize
                        );
                    }
                    leafNode->nData[metadata->nDataCountBShift] = count + 1;
                    metadata->tupleCount++;
                    return true;
                } else {
                    //split leaf node into root node
                    return false;
                }
            } else {
                //If we traverse all elements, we can notice that when we are searching for the right interval, it is always the child on the right side of the element.
                //When we finally find an element, that is greater than tuple to be inserted, child from left side is evaluated as the suitor and saved. We search no further.
                //Case when it never is on the left side is takne into consideration as well
                cRootNode<T>* rootNode = (cRootNode<T>*)node;
                int count = rootNode->getCount();
                cNode<T>* childNode = nullptr;
                bool childNodeFound = false;
                for(int i = 0; i < count && !childNodeFound; i++) {
                    //find child node
                    cNode<T>* leftChildNode = rootNode->getChildPtr(i);
                    char* nodeElement = rootNode->getElementPtr(i);
                    cNode<T>* rightChildNode = rootNode->getChildPtr(i+1);
                    for(int j = 0; j < metadata->n1; j++) {
                        //compare tuple with node element
                        if(tuple.attributes[j] < nodeElement[j]) {
                            if(childNode != nullptr){
                                childNodeFound = true;
                            }
                            childNode = leftChildNode;
                            break;
                        } else if(tuple.attributes[j] >= nodeElement[j]) {
                            childNode = rightChildNode;
                            break;
                        }
                    }
                    if(childNode != nullptr) {
                        //insert into child node
                        node = childNode;
                        break;
                    }
                }
            }
        }
    }

    void search(cTuple<T>& tuple) {
        // Implementation for search method
    }
    void printRootNode(){
        cLeafNode<T>* leafNode = (cLeafNode<T>*)this->root;
        leafNode->printTuples();
    }
    void printMetadata(){
        this->metadata->printMetadata();
    }
};
