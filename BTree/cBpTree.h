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
#include "cTreeMetadata.h"
#include "cTuple.h"
#include "cNode.h"
#include "cLeafNode.h"
#include "cInnerNode.h"

template<typename T>
class cBpTree {
private:
    cNode<T>* root;
    cTreeMetadata<T> * metadata;

public:
    // Attriutes:
    // tupLen - number of elements in tuple
    // n1 - number of indexed attributes
    // n2 - number of non-indexed attributes
    // maxInnerNodeElements - maximum number of elements in inner node
    // maxLeafNodeElements - maximum number of elements in leaf node
    cBpTree(int tupLen, int n1, int n2, int maxInnerNodeElements, int maxLeafNodeElements){
        //start with leaf node, after it overflows, it will be split into inner node
        this->metadata = new cTreeMetadata<T>(tupLen, n1, n2, maxInnerNodeElements, maxLeafNodeElements);
        this->root = new cInnerNode<T>(this->metadata);
    }

    ~cBpTree() {
        delete this->root;
    }

    bool insert(cTuple<T>& tuple) {
        return this->root->insertTuple(&tuple) < 0 ? false : true;
    }

    void search(cTuple<T>& tuple) {
        // Implementation for search method
    }
    void printBpTree(){
        this->root->printNodes(true, 0);
    }
    void printMetadata(){
        this->metadata->printMetadata();
    }
};
