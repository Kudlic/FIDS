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
#include "cStack.h"
//DONE list:
//  - reimplement tree operations using stack

//TODO list:
//  - implement split of inner node
//  - implement linked list of leaf nodes
//    - customise metadata, it might be tricky and may require splitting start of nData for leaves and for inner nodes
//    - implement pointer serialisation
//    - take care of proper updating when inserting between leaves
//  - implement point search
//  - implement range search using linked list
//  - implement delete
template<typename T>
class cBpTree {
private:
    cNode<T>* root;
    cTreeMetadata<T> * metadata;

public:
    // Attriutes:
    // + tupLen - number of elements in tuple
    // + n1 - number of indexed attributes
    // + n2 - number of non-indexed attributes
    // + maxInnerNodeElements - maximum number of elements in inner node
    // + maxLeafNodeElements - maximum number of elements in leaf node
    cBpTree(int tupLen, int n1, int n2, int maxInnerNodeElements, int maxLeafNodeElements){
        //start with leaf node, after it overflows, it will be split into inner node
        this->metadata = new cTreeMetadata<T>(tupLen, n1, n2, maxInnerNodeElements, maxLeafNodeElements);
        this->root = new cLeafNode<T>(this->metadata);
        this->metadata->order++;
    }

    ~cBpTree() {
        delete this->root;
    }

    bool insert(cTuple<T>& tuple) {
        cStack<cNode<T>*> stack = cStack<cNode<T>*>(this->metadata->order);
        stack.push(this->root, 0);
        bool needToSplit = false;

        while (!stack.isEmpty() && !needToSplit) {
            int position = 0;
            cNode<T>* current = stack.peek(position);
            bool isLeaf = current->isLeafNode();
            int count = current->getCount();
            if(isLeaf){
                cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
                if(count >= leafNode->metadata->maxLeafNodeElements) {return false;}
                leafNode->insertTuple(&tuple);
                count = leafNode->getCount();
                if(count >= leafNode->metadata->maxLeafNodeElements){
                    needToSplit = true;
                    break;
                }
                return true;
            }
            else if(!isLeaf){
                cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
                if(innerNode->getChild(0)->isLeafNode()){
                    //find child node to insert tuple
                    cLeafNode<T>* child = nullptr;
                    int childIndex = -1;
                    for(int i = 0; i < count; i++) {
                        //TODO: ensure we are not wasting memory here
                        cTuple<T> *tupleLowObj = new cTuple<T> ((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n1);
                        cTuple<T> *tupleHighObj = new cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1);

                        if(tuple.isLT(*tupleLowObj)) {
                            if(i == 0){
                                childIndex = 0;
                                innerNode->widenLow(childIndex, &tuple);
                            }
                            else{
                                childIndex = i-1;
                                innerNode->widenHigh(childIndex, &tuple);
                            }
                            child = dynamic_cast<cLeafNode<T>*>(innerNode->getChild(childIndex));
                            break;
                        }
                        else if(tuple.isTupleBetween(*tupleLowObj, *tupleHighObj)) {
                            childIndex = i;
                            child = dynamic_cast<cLeafNode<T>*>(innerNode->getChild(childIndex));
                            break;
                        }
                    }   
                    //If child node was not found, insert tuple into last child, as all ranges must have been too low
                    if(child == nullptr) {
                        childIndex = count - 1;
                        innerNode->widenHigh(childIndex, &tuple);
                        child = dynamic_cast<cLeafNode<T>*>(innerNode->getChild(childIndex));
                    }
                    stack.push(child, childIndex);
                    continue;
                }
                else{
                    //find child node to insert tuple
                    cInnerNode<T>* child = nullptr;
                    int childIndex = -1;
                    for(int i = 0; i < count; i++) {
                        cTuple<T> *tupleLowObj = new cTuple<T> ((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n1);
                        cTuple<T> *tupleHighObj = new cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1);

                        if(tuple.isLT(*tupleLowObj)) {
                            if(i == 0){
                                childIndex = 0;
                                innerNode->widenLow(childIndex, &tuple);
                            }
                            else{
                                childIndex = i-1;
                                innerNode->widenHigh(childIndex, &tuple);
                            }
                            child = dynamic_cast<cInnerNode<T>*>(innerNode->getChild(childIndex));
                            break;
                        }
                        else if(tuple.isTupleBetween(*tupleLowObj, *tupleHighObj)) {
                            childIndex = i;
                            child = dynamic_cast<cInnerNode<T>*>(innerNode->getChild(childIndex));
                            break;
                        }
                    }
                    //If child node was not found, insert tuple into last child, as all ranges must have been too low
                    if(child == nullptr) {
                        childIndex = count - 1;
                        innerNode->widenHigh(childIndex, &tuple);
                        child = dynamic_cast<cInnerNode<T>*>(innerNode->getChild(childIndex));
                    }
                    stack.push(child, childIndex);
                    continue;
                }
            }
        }
        while(!stack.isEmpty() && needToSplit){
            int position = 0;
            cNode<T>* current = stack.pop(position);
            bool isLeaf = current->isLeafNode();
            int count = current->getCount();
            if(isLeaf){
                cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
                if(count < leafNode->metadata->maxLeafNodeElements) {return true;}
                cNode<T>* splitRet= leafNode->split(position, stack.isEmpty() ? nullptr : stack.peek());
                if(stack.isEmpty()){
                    this->root = splitRet;
                    return true;
                }
            }
            if(!isLeaf){
                cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
                if (count < innerNode->metadata->maxInnerNodeElements) {return true;}
                cNode<T>* splitRet= innerNode->split(position, stack.isEmpty() ? nullptr : stack.peek());
                if(stack.isEmpty()){
                    this->root = splitRet;
                    return true;
                }
            }
        }
    }

    void search(cTuple<T>& tuple) {
        // Implementation for search method
    }
    void search(cTuple<T>& tuple, cTuple<T>& tuple) {
        // Implementation for search method
    }
    void printBpTree(){
        this->root->printNodes(true, 0);
    }
    void printMetadata(){
        this->metadata->printMetadata();
    }
};
