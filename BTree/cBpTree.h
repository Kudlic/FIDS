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
#include "cBpTreeIterator.h"
#include <stdlib.h>
#include <math.h>
//DONE list:
//  - reimplement tree operations using stack
//  - implement split of inner node
//  - implement linked list of leaf nodes
//    - customise metadata, it might be tricky and may require splitting start of nData for leaves and for inner nodes
//    - implement pointer serialisation
//    - take care of proper updating when inserting between leaves
//  - implement point search
//  - implement range search using linked list
//  - implement range search using inner nodes

//TODO list:
//  - implement delete
//  - implement rangeQuery iterator, just move all variables into iterator class, find start and end nodes and also save them in iterator. Then just use some next or save methods to work with the result. We can avoid copying of data this way.

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
    cBpTree(int tupLen, int n1, int n2, int maxInnerNodeElements, int maxLeafNodeElements);
    ~cBpTree();
    double getBpTreeBytes();
    int getTupleCount();
    bool insert(cTuple<T>& tuple);
    bool remove(cTuple<T>& tuple);
    int pointSearch(cTuple<T>& tuple);
    int searchLinkedList(cTuple<T>& tupleLow, cTuple<T>& tupleHigh);
    int searchRange(cTuple<T>& tupleLow, cTuple<T>& tupleHigh, T*& resultData, int& allocatedCount, bool printData = false);
    int searchPoint(cTuple<T>& tuple, T*& resultData, int& allocatedCount, bool printData = false);
    int searchRangeNoAlloc(cTuple<T>& tupleLow, cTuple<T>& tupleHigh, T*& resultData, bool printData = false);
    cBpTreeIteratorRange<T>* searchRangeIterator(cTuple<T>& tupleLow, cTuple<T>& tupleHigh);
    void printTree();
    void printMetadata();

};

template<typename T>
cBpTree<T>::cBpTree(int tupLen, int n1, int n2, int maxInnerNodeElements, int maxLeafNodeElements){
    //start with leaf node, after it overflows, it will be split into inner node
    this->metadata = new cTreeMetadata<T>(tupLen, n1, n2, maxInnerNodeElements, maxLeafNodeElements);
    this->root = new cLeafNode<T>(this->metadata);
    this->metadata->order++;
}
template<typename T>
cBpTree<T>::~cBpTree() {
    delete this->root;
    delete this->metadata;
}
template<typename T>
double cBpTree<T>::getBpTreeBytes(){
    return this->metadata->getBpTreeAllocatedBytes();
}
template<typename T>
int cBpTree<T>::getTupleCount(){
    return this->metadata->tupleCount;
}
template<typename T>
bool cBpTree<T>::insert(cTuple<T>& tuple) {
    //Initiate a node stack
    cStack<cNode<T>*> stack = cStack<cNode<T>*>(this->metadata->order);
    stack.push(this->root, 0);
    bool needToSplit = false;
    //Iterate through the tree until leaf node is reached
    while (!stack.isEmpty() && !needToSplit) {
        int position = 0;
        cNode<T>* current = stack.peek(position);
        bool isLeaf = current->isLeafNode();
        int count = current->getCount();

        //If current node is inner node, find child node to insert tuple into
        if(!isLeaf){
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
            //If child is leaf node
            if(innerNode->getChild(0)->isLeafNode()){
                //find child node to insert tuple
                cLeafNode<T>* child = nullptr;
                int childIndex = -1;
                //Searching for range is simplified, if possible, always insert into earliest possible range, which will have range high widened
                //Distances are not used to find closest tuple
                for(int i = 0; i < count; i++) {
                    //Two tuples from range of inner node
                    cTuple<T> tupleLowObj = cTuple<T> ((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n1, true);
                    cTuple<T> tupleHighObj = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);

                    if(tuple.isLT(tupleLowObj)) {
                        //If tuple is lower than first tuple in inner node, insert into first child
                        if(i == 0){
                            childIndex = 0;
                            innerNode->widenLow(childIndex, &tuple);
                        }
                        //Else insert into previous child
                        else{
                            childIndex = i-1;
                            innerNode->widenHigh(childIndex, &tuple);
                        }
                        child = dynamic_cast<cLeafNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                    else if(tuple.isTupleBetween(tupleLowObj, tupleHighObj)) {
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
            //If child is not yet leaf
            else{
                //find child node to insert tuple
                cInnerNode<T>* child = nullptr;
                int childIndex = -1;
                for(int i = 0; i < count; i++) {
                    cTuple<T> tupleLowObj = cTuple<T> ((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n1, true);
                    cTuple<T> tupleHighObj = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);

                    if(tuple.isLT(tupleLowObj)) {
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
                    else if(tuple.isTupleBetween(tupleLowObj, tupleHighObj)) {
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
        //If current node is leaf node, insert tuple into it and jump to checking for splits
        else if(isLeaf){
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
    return true;
}

//First try to find the appropriate record using between on ranges, preemptively stop if not found
template<typename T>
bool cBpTree<T>::remove(cTuple<T>& tuple){
    //Initiate a node stack
    cStack<cNode<T>*> stack = cStack<cNode<T>*>(this->metadata->order);
    stack.push(this->root, 0);
    bool needToMerge = false;
    int position = -1;
    //Iterate through the tree until leaf node is reached
    while (!stack.isEmpty() && !needToMerge) {
        cNode<T>* current = stack.peek(position);
        bool isLeaf = current->isLeafNode();
        int count = current->getCount();
        if(!isLeaf){
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupHigh = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);
                if(tupHigh.isGEQT(tuple, innerNode->metadata->n1)){
                    current = innerNode->getChild(i);
                    break;
                }
            }
            if(i == count){
                return false;
            }
            stack.push(current, i);
            continue;
        }
        else if(isLeaf){
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tempTuple = cTuple<T>((T*)leafNode->getElementPtr(i), leafNode->metadata->n1, true);
                if(tuple.isEQ(tempTuple, leafNode->metadata->n1)){
                    leafNode->deleteTuple(i);
                    needToMerge = true;
                    break;
                }
            }
            if(i == count){
                return false;
            }
        }
    }
    //turn off needToMerge, when no changes need to be made, by changes we mean merging of nodes or fixing ranges in inner nodes
    while(!stack.isEmpty() && needToMerge){
        int position = 0;
        cNode<T>* current = stack.pop(position);
        bool isLeaf = current->isLeafNode();
        int count = current->getCount();

        cInnerNode<T>* parent = stack.isEmpty() ? nullptr : dynamic_cast<cInnerNode<T>*>(stack.peek());
        if(isLeaf){
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
            count = leafNode->getCount();
            //Step 1: First check if leafNode is root, in that case no borrowing or merging can occur
            if(parent == nullptr){
                needToMerge = false;
                break;
            }
            //Step 2: If not root and underflowed, try borrowing or merging
            else if (count < leafNode->metadata->halfLeafNode){
                //now I need to try borrowing from siblings, if not possible, merge
                int parentCount = parent->getCount();
                bool rightPresent =  parentCount-1 > position ? 1 : 0;
                bool borrowed = false;
                //borrow from right
                if(rightPresent){
                    cLeafNode<T>* sibling = dynamic_cast<cLeafNode<T>*>(parent->getChild(position + 1));
                    if(sibling->getCount() > sibling->metadata->halfLeafNode){
                        //save tuple from sibling, insert it into current, remove it from sibling
                        cTuple<T> borrowedTuple = cTuple<T>((T*)sibling->getElementPtr(0), sibling->metadata->n, true);
                        leafNode->insertTuple(&borrowedTuple);
                        sibling->deleteTuple(0);
                        //update ranges of sibling and current
                        parent->adjustRange(position + 1);
                        parent->adjustRange(position);
                        borrowed = true;
                    }
                }
                //borrow from left
                if(!borrowed && position > 0){
                    cLeafNode<T>* sibling = dynamic_cast<cLeafNode<T>*>(parent->getChild(position - 1));
                    if(sibling->getCount() > sibling->metadata->halfLeafNode){
                        //save tuple from sibling, insert it into current, remove it from sibling
                        cTuple<T> borrowedTuple = cTuple<T>((T*)sibling->getElementPtr(sibling->getCount()-1), sibling->metadata->n, true);
                        leafNode->insertTuple(&borrowedTuple);
                        sibling->deleteTuple(sibling->getCount()-1);
                        //update ranges of sibling and current
                        parent->adjustRange(position - 1);
                        parent->adjustRange(position);
                        borrowed = true;
                    }
                }
                if(!borrowed){
                    if(rightPresent){
                        //merge with right sibling
                        current->mergeRight(position, parent);   
                    }
                    else if(position > 0){
                        //merge with left sibling
                        cLeafNode<T>* sibling = dynamic_cast<cLeafNode<T>*>(parent->getChild(position - 1));
                        sibling->mergeRight(position-1, parent);   
                    }
                    else{
                        throw std::overflow_error("Node underflowed");
                    }
                }
                
            }
            //Step 3: If no merges are needed, check if ranges are correct
            else if(parent->needAdjustment(position)){
                parent->adjustRange(position);
            }
            //Step 4: If no merges are needed and ranges are correct, turn off needToMerge
            else{
                needToMerge = false;
                break;
            }
        }
        if(!isLeaf){
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
            count = innerNode->getCount();
            //check for merge needs
            //Step 1: First check if innerNode is root and underflowed to 1 range, in that case delete it and make leafNode root
            //TODO
            if(parent == nullptr){
                //If we are at root and it underflowe
                if(count == 1){
                    this->root = innerNode->passDown();
                    delete innerNode;
                }
                needToMerge = false;
                break;
            }
            //Step 2: If underflowed, try borrowing or merging
            else if(count < innerNode->metadata->halfInnerNode) {
                //TODO: do exactly the same thing as with leaf nodes, with inner nodes, 
                //      we still need to borrow and merge after it is possible, so it should be no different

                int parentCount = parent->getCount();
                bool rightPresent =  parentCount-1 > position ? 1 : 0;
                bool borrowed = false;
                //borrow from right
                if(rightPresent){
                    borrowed = innerNode->borrowFromRight(position, parent);
                }
                //borrow from left
                if(!borrowed && position > 0){
                    borrowed = innerNode->borrowFromLeft(position, parent);
                }
                if(!borrowed){
                    if(rightPresent){
                        //merge with right sibling
                        current->mergeRight(position, parent);   
                    }
                    else if (position > 0){
                        //merge with left sibling
                        cInnerNode<T>* sibling = dynamic_cast<cInnerNode<T>*>(parent->getChild(position - 1));
                        sibling->mergeRight(position-1, parent);   
                    }
                    else{
                        throw std::overflow_error("Node underflowed");
                    }
                }
                //Step 3: If no merges are needed, check if ranges are correct (If step 2 ran, adjustment is included in it)
                else if(parent->needAdjustment(position)){
                    parent->adjustRange(position);
                }
                //Step 4: If no merges are needed and ranges are correct, turn off needToMerge
                else{
                    needToMerge = false;
                    break;
                }
            }            
            
        }
    }
    return true;
}

template<typename T>
int cBpTree<T>::pointSearch(cTuple<T>& tuple) {
    cNode<T>* current = this->root;
    int count = -1;
    int foundTuples = 0;
    while (current != nullptr) {
        if(!current->isLeafNode()){
            count = current->getCount();
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupHigh = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);
                if(tupHigh.isGEQT(tuple, innerNode->metadata->n1)){
                    current = innerNode->getChild(i);
                    break;
                }
            }
            if(i == count){
                return 0;
            }
            continue;
        }
        break;
        
    }
    cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
    int foundCount = 0;
    bool readyToStop = false;
    while(leafNode != nullptr){
        count = leafNode->getCount();
        for(int i = 0; i < count; i++) {
            cTuple<T> tempTup = cTuple<T>((T*)leafNode->getElementPtr(i), leafNode->metadata->n1, true);
            if(tuple.isEQ(tempTup, leafNode->metadata->n1)){
                foundCount++;
                readyToStop = true;
            }
            else if (readyToStop == true){
                return foundCount;
            }
        }
        readyToStop = true;
        leafNode = leafNode->getNodeLink();
    }
    return foundCount;
    
}
template<typename T>
int cBpTree<T>::searchLinkedList(cTuple<T>& tupleLow, cTuple<T>& tupleHigh) {
    cNode<T>* current = this->root;
    int count = -1;
    int foundTuples = 0;
    while (current != nullptr) {
        if(!current->isLeafNode()){
            count = current->getCount();
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupLowTree = cTuple<T> ((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n1, true);
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);
                if(tupleLow.isTupleBetween(tupLowTree, tupHighTree)){
                    current = innerNode->getChild(i);
                    break;
                }
            }
            if(i == count){
                return foundTuples;
            }
        }
        else{
            break;
        }
    }
    cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
    bool lastRes = false;
    while (leafNode != nullptr) {
        count = leafNode->getCount();
        //cTuple<T> *tupLow = new cTuple<T> ((T*)leafNode->getElementPtr(0), leafNode->metadata->n1);
        //cTuple<T> *tupHigh = new cTuple<T> ((T*)leafNode->getElementPtr(leafNode->getCount()), leafNode->metadata->n1);
        leafNode->printNodes(false, 0);
        for(int i = 0; i < count; i++) {
            cTuple<T> tempTup = cTuple<T>((T*)leafNode->getElementPtr(i), leafNode->metadata->n1, true);

            if(tupleHigh.isGEQT(tempTup)){
                foundTuples++;
                lastRes = true;
            }
            else if (lastRes == true){
                return foundTuples;
            }
        }
        if(lastRes){
            leafNode = leafNode->getNodeLink();
            if(leafNode == nullptr){
                return foundTuples;
            }
            continue;
        }
        return foundTuples;
    }
}
template<typename T>
int cBpTree<T>::searchRange(cTuple<T>& tupleLow, cTuple<T>& tupleHigh, T*& resultData, int& allocatedCount, bool printData){
    int resultCount = 0;
    int indexedValues = this->metadata->n1;

    if(tupleLow.isGT(tupleHigh)){
        allocatedCount = -1;
        return -1;
    }

    cNode<T>* first = nullptr;
    cNode<T>* last = nullptr;
    cNode<T>* current = this->root;

    int order = this->metadata->order;
    int subtreesRangeOrder = this->metadata->maxInnerNodeElements-1;
    //find first place where range splits into different nodes
    while(current != nullptr){
        bool isLeaf = current->isLeafNode();
        int count = current->getCount();
        if(isLeaf){
            first = current;
            last = current;
            current = nullptr;
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);
                //Easy to find last interval it fits in, just HigherEQthan, because lower bound can be lower than first element
                if(tupHighTree.isGEQT(tupleLow, indexedValues)){
                    first = innerNode->getChild(i);
                    break;
                }
            }

            int j = count - 1;
            for(; j >= 0; j--) {
                cTuple<T> tupLowTree = cTuple<T> ((T*)innerNode->getTupleLowPtr(j), innerNode->metadata->n1, true);
                if(tupLowTree.isLEQT(tupleHigh, indexedValues)){
                    last = innerNode->getChild(j);
                    break;
                }
            }
            //if both first and last progress into same node, we are still in the same subtree and we can allocate only for subtree of certain height
            if(i == j){
                current = first;
                order -= 1;
            }
            //this happens if our range is between two nodes, therefore there is no subtree suitable and no values in range
            else if(i > j){
                allocatedCount = -1;
                return 0;
            }
            else{
                // tupleLow.printTuple();
                // tupleHigh.printTuple();
                // first->printNodes(true, 0, true);
                // last->printNodes(true, 0, true);
                subtreesRangeOrder = j-i+1;
                current = nullptr;
            }
        }

    }   
    //TODO: improve, so not whole subtree is allocated, but only part of it defined by i and j difference
    //(maxInnerNodeElements-1 ^ order-1 * maxLeafNodeElements-1) * n
    //first->printNodes(false, 0, true);
    //last->printNodes(false, 0, true);
    int maxResultTuples = pow((this->metadata->maxInnerNodeElements-1), (order-1)) * (this->metadata->maxLeafNodeElements-1);
    resultData = new T[ maxResultTuples * this->metadata->n];
    allocatedCount = maxResultTuples * this->metadata->n;

    //Now find first leaf node, search until Leaf node is reached
    while(first != nullptr){
        bool isLeaf = first->isLeafNode();
        int count = first->getCount();
        if(isLeaf){
            break;
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(first);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);
                //Easy to find last interval it fits in, just HigherEQthan, because lower bound can be lower than first element
                if(tupHighTree.isGEQT(tupleLow, indexedValues)){
                    first = innerNode->getChild(i);
                    break;
                }
            }
        }
    }
    //Now find last leaf node, search until Leaf node is reached
    while(last != nullptr){
        bool isLeaf = last->isLeafNode();
        int count = last->getCount();
        if(isLeaf){
            break;
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(last);
            int j = count - 1;
            for(; j >= 0; j--) {
                cTuple<T> tupLowTree = cTuple<T> ((T*)innerNode->getTupleLowPtr(j), innerNode->metadata->n1, true);
                if(tupLowTree.isLEQT(tupleHigh, indexedValues)){
                    last = innerNode->getChild(j);
                    break;
                }
            }

        }
    }
    //Now we have first and last leaf node, and maximum size of result, we can start searching
    cLeafNode<T>* currentLeaf = dynamic_cast<cLeafNode<T>*>(first);
    bool endReached = false;
    //we need to check which records to copy only in first and last leaf node, all between can be brute copied
    //tupleLow.printTuple();
    //tupleHigh.printTuple();
    while(!endReached){
        //index of first element to copy
        int count = currentLeaf->getCount();
        int startTupleIx = 0;
        //index of last element to copy
        int endTupleIx = count-1;
        //currentLeaf->printNodes(false, 0, true);

        if(currentLeaf == first){
            //find first element to copy
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupLowLeaf = cTuple<T> ((T*)currentLeaf->getElementPtr(i), currentLeaf->metadata->n1, true);
                if(tupLowLeaf.isGEQT(tupleLow, indexedValues)){
                    startTupleIx = i;
                    break;
                }
            }
        }
        if(currentLeaf == last){
            //find first element to not copy
            int j = endTupleIx;
            for(; j >= 0; j--) {
                cTuple<T> tupHighLeaf = cTuple<T> ((T*)currentLeaf->getElementPtr(j), currentLeaf->metadata->n1, true);
                if(tupHighLeaf.isLEQT(tupleHigh, indexedValues)){
                    endTupleIx = j;
                    break;
                }
            }
            endReached = true;
        }

        count = endTupleIx - startTupleIx + 1;
        //copy all elements between startTupleIx and endTupleIx
        int startPos = (resultCount * this->metadata->nDataElementLeafSize);
        int leafPos = currentLeaf->metadata->nDataStartBShift + (currentLeaf->metadata->nDataElementLeafSize * startTupleIx);
        memcpy(
            (char*)resultData + (resultCount * this->metadata->nDataElementLeafSize ), 
            currentLeaf->nData + currentLeaf->metadata->nDataStartBShift + (currentLeaf->metadata->nDataElementLeafSize * startTupleIx),
            currentLeaf->metadata->nDataElementLeafSize * (count)
        );
        resultCount += count;
        cLeafNode<T>* currentLeafBckp = currentLeaf;
        currentLeaf = currentLeaf->getNodeLink();
        if(currentLeaf == nullptr){
            //printf("End of linked list reached\n");
            //currentLeafBckp->printNodes(false, 0, true);
            endReached = true;
        }
        
    }
    if(printData){
        printf("Found %d tuples\n", resultCount);
        for(int i = 0; i < resultCount; i++){
            printf("[");
            for(int j = 0; j < this->metadata->n; j++)
                printf("%d ", resultData[j+i*this->metadata->n]);
            printf("],\n");
        }
    }
    return resultCount;
}
template<typename T>
int cBpTree<T>::searchPoint(cTuple<T>& tuple, T*& resultData, int& allocatedCount, bool printData){
    return searchRange(tuple, tuple, resultData, allocatedCount, printData);
}
template<typename T>
int cBpTree<T>::searchRangeNoAlloc(cTuple<T>& tupleLow, cTuple<T>& tupleHigh, T*& resultData, bool printData){
    int resultCount = 0;
    int indexedValues = this->metadata->n1;

    if(tupleLow.isGT(tupleHigh)){
        return -1;
    }

    cNode<T>* first = this->root;
    cNode<T>* last = this->root;        

    //Now find first leaf node, search until Leaf node is reached
    while(first != nullptr){
        bool isLeaf = first->isLeafNode();
        int count = first->getCount();
        if(isLeaf){
            break;
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(first);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);
                //Easy to find last interval it fits in, just HigherEQthan, because lower bound can be lower than first element
                if(tupHighTree.isGEQT(tupleLow, indexedValues)){
                    first = innerNode->getChild(i);
                    break;
                }
            }
            if(i == count){
                first = innerNode->getChild(i-1);
            }
        }
    }
    //Now find last leaf node, search until Leaf node is reached
    while(last != nullptr){
        bool isLeaf = last->isLeafNode();
        int count = last->getCount();
        if(isLeaf){
            break;
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(last);
            int j = count - 1;
            for(; j >= 0; j--) {
                cTuple<T> tupLowTree = cTuple<T> ((T*)innerNode->getTupleLowPtr(j), innerNode->metadata->n1, true);
                if(tupLowTree.isLEQT(tupleHigh, indexedValues)){
                    last = innerNode->getChild(j);
                    break;
                }
            }
            if(j == -1){
                last = innerNode->getChild(0);
            }
        }
    }
    //Check if first and last didnt pass each other
    if(dynamic_cast<cLeafNode<T>*>(last)->getNodeLink() == first){
        return 0;
    }
    //Now we have first and last leaf node, and maximum size of result, we can start searching
    cLeafNode<T>* currentLeaf = dynamic_cast<cLeafNode<T>*>(first);
    bool endReached = false;
    //we need to check which records to copy only in first and last leaf node, all between can be brute copied
    //tupleLow.printTuple();
    //tupleHigh.printTuple();
    while(!endReached){
        //index of first element to copy
        int count = currentLeaf->getCount();
        int startTupleIx = 0;
        //index of last element to copy
        int endTupleIx = count-1;
        //currentLeaf->printNodes(false, 0, true);

        if(currentLeaf == first){
            //find first element to copy
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupLowLeaf = cTuple<T> ((T*)currentLeaf->getElementPtr(i), currentLeaf->metadata->n1, true);
                if(tupLowLeaf.isGEQT(tupleLow, indexedValues)){
                    
                    break;
                }
            }
            startTupleIx = i;
        }
        if(currentLeaf == last){
            //find first element to not copy
            int j = endTupleIx;
            for(; j >= 0; j--) {
                cTuple<T> tupHighLeaf = cTuple<T> ((T*)currentLeaf->getElementPtr(j), currentLeaf->metadata->n1, true);
                if(tupHighLeaf.isLEQT(tupleHigh, indexedValues)){
                    break;
                }
            }
            endTupleIx = j;
            endReached = true;
        }

        count = endTupleIx - startTupleIx + 1;
        //copy all elements between startTupleIx and endTupleIx
        int startPos = (resultCount * this->metadata->nDataElementLeafSize);
        int leafPos = currentLeaf->metadata->nDataStartBShift + (currentLeaf->metadata->nDataElementLeafSize * startTupleIx);
        memcpy(
            (char*)resultData + (resultCount * this->metadata->nDataElementLeafSize ), 
            currentLeaf->nData + currentLeaf->metadata->nDataStartBShift + (currentLeaf->metadata->nDataElementLeafSize * startTupleIx),
            currentLeaf->metadata->nDataElementLeafSize * (count)
        );
        resultCount += count;
        currentLeaf = currentLeaf->getNodeLink();
        if(currentLeaf == nullptr){
            //printf("End of linked list reached\n");
            endReached = true;
        }
        
    }
    if(printData){
        printf("Found %d tuples\n", resultCount);
        for(int i = 0; i < resultCount; i++){
            printf("[");
            for(int j = 0; j < this->metadata->n; j++)
                printf("%d ", resultData[j+i*this->metadata->n]);
            printf("],\n");
        }
    }
    return resultCount;
}
template<typename T>
cBpTreeIteratorRange<T>* cBpTree<T>::searchRangeIterator(cTuple<T>& tupleLow, cTuple<T>& tupleHigh){
    int resultCount = 0;
    int indexedValues = this->metadata->n1;

    if(tupleLow.isGT(tupleHigh)){
        return nullptr;
    }

    cNode<T>* first = this->root;
    cNode<T>* last = this->root;        
    int indexFirst = -1;
    int indexLast = -1;

    //Now find first leaf node, search until Leaf node is reached
    while(first != nullptr && indexFirst == -1){
        bool isLeaf = first->isLeafNode();
        int count = first->getCount();
        if(isLeaf){
            //find index of first element to copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(first);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupleTemp = cTuple<T> ((T*)leafNode->getElementPtr(i), first->metadata->n1, true);
                if(tupleTemp.isGEQT(tupleLow, indexedValues)){
                    indexFirst = i;
                    break;
                }
            }
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(first);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n1, true);
                //Easy to find last interval it fits in, just HigherEQthan, because lower bound can be lower than first element
                if(tupHighTree.isGEQT(tupleLow, indexedValues)){
                    first = innerNode->getChild(i);
                    break;
                }
            }
            if(i == count){
                first = nullptr;
                indexFirst = 0;
            }
        }
    }
    //Now find last leaf node, search until Leaf node is reached
    while(last != nullptr && indexLast == -1){
        bool isLeaf = last->isLeafNode();
        int count = last->getCount();
        if(isLeaf){
            //find index of first element to not copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(last);
            int j = 0;
            for(; j < count; j++) {
                cTuple<T> tupleTemp = cTuple<T> ((T*)leafNode->getElementPtr(j), last->metadata->n1, true);
                if(tupleTemp.isGT(tupleHigh, indexedValues)){
                    indexLast = j;
                    break;
                }
            }
        }
        else{//if there is no tuple that is not supposed to be included, we will set inner node tu nullptr,
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(last);
            int j = 0;
            int count = last->getCount();
            //check if tuple is higher than last tuple in inner node
            for(; j < count; j++) {
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(j), innerNode->metadata->n1, true);
                if(tupHighTree.isGT(tupleHigh, indexedValues)){
                    last = innerNode->getChild(j);
                    break;
                }
            }
            if(j == count){
                last = nullptr;
                indexLast = 0;
                break;
            }
        }
    }
    return new cBpTreeIteratorRange<T>(dynamic_cast<cLeafNode<T>*>(first), dynamic_cast<cLeafNode<T>*>(last), indexFirst, indexLast, this->metadata);

}
template<typename T>
void cBpTree<T>::printTree(){
    this->root->printNodes(true, 0);
}
template<typename T>
void cBpTree<T>::printMetadata(){
    this->metadata->printMetadata();
}
