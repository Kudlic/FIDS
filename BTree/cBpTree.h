/*
Give me some base for cBpTree class implementation of B+ tree, which will include inner node cNode(which can have child types cLeafNode or cInnerNode, 
cNode itself should only include common nData char* pointer, where all data of nodes will be stored, including its count of elements on the start of nData). 
In its constructor there will be tupLen attribute, specifying size of cTuple (class encapsulating n*type array, include its specification). 
We will also need to specify number of indexed and nonindexed attributes. For simplicity, we can assume n1 will be number of indexed attributes, 
which will be first n tupple attributes. n2 will be rest of the tuple. We also need to specify max InnerNode and LeafNode elements. 
All important information will be stored in object of cTreeMetadata class, 
which I also want you to create, please include even tree order and height. Tree itself should only contain the inner node and methods for work with it. In implementation, I plan to always 
start the BpTree with cLeafNode. After it gets filled up, then I will create cInnerNode based on it and do a split. 
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
template<typename T> class cInnerNode;

template<typename TT>
class cTreeMetadata {
public:
    // Constructor
    cTreeMetadata(int n, int n1, int n2, int maxInnerNodeElements, int maxLeafNodeElements) : 
        n(n), 
        n1(n1), 
        n2(n2), 
        maxInnerNodeElements(maxInnerNodeElements),
        maxLeafNodeElements(maxLeafNodeElements),
        order(0),
        tupleCount(0),
        leafNodeCount(0),
        innerNodeCount(0),
        nDataIsLeafNodeBShift(0),
        nDataCountBShift(4),
        nDataStartBShift(8) {
            attrSize = sizeof(TT);
            nDataSizeLeaf = (nDataStartBShift) + (attrSize * n * maxLeafNodeElements);
            nDataSizeInner = (nDataStartBShift) + ((sizeof(char*) + (n1 * attrSize * 2)) * maxInnerNodeElements);
            nDataElementLeafSize = attrSize * n;
            nDataElementInnerSize = (n1 * attrSize * 2) + sizeof(char*);
            nDataElementInnerTupleSize = n1 * attrSize;
            halfInnerNode = maxInnerNodeElements/2;
            halfLeafNode = maxLeafNodeElements/2;
        }
    void printMetadata() {
        std::cout << "n: " << n << std::endl;
        std::cout << "n1: " << n1 << std::endl;
        std::cout << "n2: " << n2 << std::endl;
        std::cout << "maxInnerNodeElements: " << maxInnerNodeElements << std::endl;
        std::cout << "maxLeafNodeElements: " << maxLeafNodeElements << std::endl;
        std::cout << "order: " << order << std::endl;
        std::cout << "tupleCount: " << tupleCount << std::endl;
        std::cout << "leafNodeCount: " << leafNodeCount << std::endl;
        std::cout << "innerNodeCount: " << innerNodeCount << std::endl;
        std::cout << "nDataIsLeafNodeBShift: " << nDataIsLeafNodeBShift << std::endl;
        std::cout << "nDataCountBShift: " << nDataCountBShift << std::endl;
        std::cout << "nDataStartBShift: " << nDataStartBShift << std::endl;
        std::cout << "nDataSizeLeaf: " << nDataSizeLeaf << std::endl;
        std::cout << "nDataSizeInner: " << nDataSizeInner << std::endl;
        std::cout << "nDataElementLeafSize: " << nDataElementLeafSize << std::endl;
        std::cout << "nDataElementInnerSize: " << nDataElementInnerSize << std::endl;
    }

private:
    //Constants
    int n; // Number of attributes
    int n1; // Number of indexed attributes
    int n2; // Number of non-indexed attributes
    int maxInnerNodeElements; // Maximum number of elements in a inner node
    int maxLeafNodeElements; // Maximum number of elements in a leaf node
    int attrSize; // Size of an attribute
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
    int nDataStartBShift; // Start of nData in nData

    int nDataElementLeafSize; // Size of an element in a leaf node, element is tuple of n attributes
    int nDataElementInnerSize; // Size of an element in a inner node, element is lowTuple, highTuple and reference to child node
    int nDataElementInnerTupleSize; // Size of a tuple in an element in a inner node

    template<typename T> friend class cBpTree;
    template<typename T> friend class cNode;
    template<typename T> friend class cLeafNode;
    template<typename T> friend class cInnerNode;
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

    virtual bool split() = 0;

protected:
    char* nData; // Data of the node
    cTreeMetadata<T> * metadata; // Metadata of the tree
    cInnerNode * parent;

    template<typename TT> friend class cBpTree;
};

template<typename T>
class cLeafNode : public cNode<T> {
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
    bool split() override {
        int count = this->getCount();
        if(count < this->metadata->maxLeafNodeElements)
            return false;
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
            cInnerNode* parent = new cInnerNode<T>(this->metadata);
            this->parent = parent;
            //set first element of parent node, start at nDataStartBShift, save first n1 elements of first tuple from this->nData, then save first n1 elements of last tuple from this->nData and append pointerto this node
            memcpy(
                parent->nData + this->metadata->nDataStartBShift, 
                this->nData + this->metadata->nDataStartBShift, 
                this->metadata->nDataElementLeafSize
            );
        }
        second->parent = this->parent;
    }
};

template<typename T>
class cInnerNode : public cNode<T> {
    public:
    // Implementation for inner node
    // inner nodes have mData structure as follows: int isLeaf, int count, [cNode* child[i], range[i]], where range is (tupleLow, tupleHigh), values in child are >= tupleLow and <= tupleHigh
    cInnerNode(cTreeMetadata<T> * metadata): cNode<T>(metadata) {
        this->nData = new char[metadata->nDataSizeInner];
        this->setCount(0);
        this->setLeafNode(false);
        this->metadata->innerNodeCount++;
        this->metadata->
    }
    ~cInnerNode() {
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
    bool addElement(cTuple<T>* tupleLow, cTuple<T>* tupleHigh, cNode<T>* child) {
        // Implementation for addElement method, always inserts at end, therefore good only when we know order wont get changed
        int index = this->getCount();
        if(tupleLow->n < this->metadata->n1 || tupleHigh->n < this->metadata->n1)
            return false;
        //If there are elements present, find the right place to insert
        memcpy(
            this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize), 
            tupleLow->attributes, 
            this->metadata->nDataElementInnerTupleSize
        );
        memcpy(
            this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + this->metadata->nDataElementInnerTupleSize, 
            tupleHigh->attributes, 
            this->metadata->nDataElementInnerTupleSize
        );
        memcpy(
            this->nData + this->metadata->nDataStartBShift + (index * this->metadata->nDataElementInnerSize) + (this->metadata->nDataElementInnerTupleSize * 2), 
            &child, 
            sizeof(cNode<T>*)
        );
        return true;
    }
    bool splitElement(int index, cTuple<T>* firstTupleHigh, cTuple<T>* secondTupleLow) {
        if(index < 0 || index >= this->getCount() || this->getCount() >= this->metadata->maxInnerNodeElements)
            return false;
        // Implementation for splitElement method
        //split element at index into two elements, first element will be at index, second element will be at index + 1, all elements from index + 1 will be moved to the right
        //first element will have original tupleLow and tupleHigh = firstTupleHigh, second element will have tupleLow = secondTupleLow and original tupleHigh
        //after splitting element, split child node of element at index with address

    }
};


template<typename T>
class cBpTree {
private:
    cNode<T>* root;
    cTreeMetadata<T> * metadata;

public:
    cBpTree(int tupLen, int n1, int n2, int maxInnerNodeElements, int maxLeafNodeElements){
        //start with leaf node, after it overflows, it will be split into inner node
        this->metadata = new cTreeMetadata<T>(tupLen, n1, n2, maxInnerNodeElements, maxLeafNodeElements);
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
                    //split leaf node into inner node
                    return false;
                }
            } else {
                //If we traverse all elements, we can notice that when we are searching for the right interval, it is always the child on the right side of the element.
                //When we finally find an element, that is greater than tuple to be inserted, child from left side is evaluated as the suitor and saved. We search no further.
                //Case when it never is on the left side is taken into consideration as well
                cInnerNode<T>* innerNode = (cInnerNode<T>*)node;
                int count = innerNode->getCount();
                cNode<T>* childNode = nullptr;
                bool childNodeFound = false;
                for(int i = 0; i < count && !childNodeFound; i++) {
                    //find child node
                    cNode<T>* leftChildNode = innerNode->getChildPtr(i);
                    char* nodeElement = innerNode->getElementPtr(i);
                    cNode<T>* rightChildNode = innerNode->getChildPtr(i+1);
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
        return false;
    }

    void search(cTuple<T>& tuple) {
        // Implementation for search method
    }
    void printInnerNode(){
        cLeafNode<T>* leafNode = (cLeafNode<T>*)this->root;
        leafNode->printTuples();
    }
    void printMetadata(){
        this->metadata->printMetadata();
    }
};
