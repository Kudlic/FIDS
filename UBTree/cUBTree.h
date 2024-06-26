#include <iostream>
#include <cstring>
#include "cTreeMetadata.h"
#include "cZAddrUtils.h"
#include "cTuple.h"
#include "cNode.h"
#include "cLeafNode.h"
#include "cInnerNode.h"
#include "cStack.h"
#include "cUBTreeIteratorRange.h"
#include "cUBTreeIteratorRangeStack.h"
#include "cUBTreeIteratorPoint.h"
#include "cUBTreeIteratorRangeStackBin.h"
#include <stdlib.h>
#include <math.h>
#include <iomanip>
#include <memory>

//How will conversion to zAddresses work:
// 1. cTuple will become a container for zAddress of type B instead of tuple of type T, all methods will stay the same otherwise
// 2. cUBTree methods will accept T type tuples, but will covert them to zAddresses at start of methods
// 3. cUbTree will use functions from ZAddressTools. Based on configuration, it will initialise metadata and also store function pointers to best functions for selection
// 4. ZAddress implementation only counts with 32bit integers. I need to type it to support other types.
// 5. Gotta find out if overhang is accounted for in zAddressTools algos

template<typename T>//T for incoming tuple type; 
class cUBTree {
private:
    cNode<T>* root; //T type to signalise type converted from, data is always stored as char
    cTreeMetadata * metadata;
    cZAddrUtils* zTools;

public:
    cUBTree(int dimensions, int maxInnerNodeElements, int maxLeafNodeElements);
    ~cUBTree();
    double getBpTreeBytes();
    int getTupleCount();
    cTreeMetadata* getMetadata();
    cZAddrUtils* getZTools();
    bool insert(cTuple<T>& tuple);
    bool remove(cTuple<T>& tuple);

    bool insert(char* zAddr);
    bool remove(char* zAddr);

    //int searchLinkedList(cTuple<T>& tupleLow, cTuple<T>& tupleHigh);
    cUBTreeIteratorRange<T>* searchRangeIterator(cTuple<T>& tupleLow, cTuple<T>& tupleHigh);
    cUBTreeIteratorRangeStack<T>* searchRangeIteratorStack(cTuple<T>& tupleLow, cTuple<T>& tupleHigh);
    cUBTreeIteratorRangeStackBin<T>* searchRangeIteratorStackBin(cTuple<T>& tupleLow, cTuple<T>& tupleHigh);
    cUBTreeIteratorPoint<T>* searchPointIterator(cTuple<T>& tuple);
    void printTreeTuples();
    void printTreeHex();
    void printMetadata();
    int checkLinkLength();
};

template<typename T>
cUBTree<T>::cUBTree(int dimensions, int maxInnerNodeElements, int maxLeafNodeElements){
    //start with leaf node, after it overflows, it will be split into inner node
    this->metadata = new cTreeMetadata(dimensions, sizeof(T), maxInnerNodeElements, maxLeafNodeElements);
    this->zTools = new cZAddrUtilsTemplate<T>(this->metadata);
    this->root = new cLeafNode<T>(this->metadata, this->zTools);
    this->metadata->order++;
}
template<typename T>
cUBTree<T>::~cUBTree() {
    delete this->root;
    delete this->metadata;
    delete this->zTools;
}
template<typename T>
double cUBTree<T>::getBpTreeBytes(){
    return this->metadata->getBpTreeAllocatedBytes();
}
template<typename T>
int cUBTree<T>::getTupleCount(){
    return this->metadata->tupleCount;
}
template<typename T>
cTreeMetadata* cUBTree<T>::getMetadata(){
    return this->metadata;
}
template<typename T>
cZAddrUtils* cUBTree<T>::getZTools(){
    return this->zTools;
}
template<typename T>
bool cUBTree<T>::insert(cTuple<T>& tuple) {
    char* zAddress = new char[this->metadata->zAddressBytes];
    cTuple<char> zAddrTuple = cTuple<char>((char*)zAddress, this->metadata->zAddressBytes); //This is a form of smart pointer, it will deallocate zAddress when it goes out of scope
    this->zTools->transformDataToZAddress((char*)tuple.getAttributes(), zAddress);
    return this->insert(zAddress);
}

template<typename T>
bool cUBTree<T>::insert(char* zAddress) {
    //TODO: dont forget to dealloc

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
                for(int i = 0; i < count; i++) {
                    //Two tuples from range of inner node
                    cTuple<T> tupleLowObj = cTuple<T> ((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n, true);
                    cTuple<T> tupleHighObj = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n, true);

                    if(zTools->isZAddrLT(zAddress, innerNode->getTupleLowPtr(i))) {
                        //If tuple is lower than first tuple in inner node, insert into first child
                        if(i == 0){
                            childIndex = 0;
                            innerNode->widenLowZAddr(childIndex, zAddress);
                        }
                        //Else insert into previous child
                        else{
                            childIndex = i-1;
                            innerNode->widenHighZAddr(childIndex, zAddress);
                        }
                        child = dynamic_cast<cLeafNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                    else if(zTools->isZAddrLEQT(zAddress, innerNode->getTupleHighPtr(i)) && zTools->isZAddrGEQT(zAddress, innerNode->getTupleLowPtr(i))) {
                        childIndex = i;
                        child = dynamic_cast<cLeafNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                }   
                //If child node was not found, insert tuple into last child, as all ranges must have been too low
                if(child == nullptr) {
                    childIndex = count - 1;
                    innerNode->widenHighZAddr(childIndex, zAddress);
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
                    cTuple<T> tupleLowObj = cTuple<T> ((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n, true);
                    cTuple<T> tupleHighObj = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n, true);

                    if(zTools->isZAddrLT(zAddress, innerNode->getTupleLowPtr(i))) {
                        if(i == 0){
                            childIndex = 0;
                            innerNode->widenLowZAddr(childIndex, zAddress);
                        }
                        else{
                            childIndex = i-1;
                            innerNode->widenHighZAddr(childIndex, zAddress);
                        }
                        child = dynamic_cast<cInnerNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                    else if(zTools->isZAddrLEQT(zAddress, innerNode->getTupleHighPtr(i)) && zTools->isZAddrGEQT(zAddress, innerNode->getTupleLowPtr(i))) {
                        childIndex = i;
                        child = dynamic_cast<cInnerNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                }
                //If child node was not found, insert tuple into last child, as all ranges must have been too low
                if(child == nullptr) {
                    childIndex = count - 1;
                    innerNode->widenHighZAddr(childIndex, zAddress);
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
            leafNode->insertZAddr(zAddress);
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
template<typename T>
bool cUBTree<T>::remove(cTuple<T>& tuple) {
    char* zAddress = new char[this->metadata->zAddressBytes];
    cTuple<char> zAddrTuple = cTuple<char>((char*)zAddress, this->metadata->zAddressBytes); //This is a form of smart pointer, it will deallocate zAddress when it goes out of scope
    this->zTools->transformDataToZAddress((char*)tuple.getAttributes(), zAddress);
    return this->remove(zAddress);
}

//First try to find the appropriate record using between on ranges, preemptively stop if not found
template<typename T>
bool cUBTree<T>::remove(char* zAddress){

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
                if(this->zTools->isZAddrGEQT(innerNode->getTupleHighPtr(i), zAddress)){
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
                if(this->zTools->isZAddrEQ(zAddress, leafNode->getElementPtr(i))){
                    leafNode->deleteZAddr(i);
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
                        leafNode->insertZAddr(sibling->getElementPtr(0));
                        sibling->deleteZAddr(0);
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
                        leafNode->insertZAddr(sibling->getElementPtr(sibling->getCount()-1));
                        sibling->deleteZAddr(sibling->getCount()-1);
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
                    this->metadata->order--;
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
    return true;
}
template<typename T>
cUBTreeIteratorRange<T>* cUBTree<T>::searchRangeIterator(cTuple<T>& tupleLow, cTuple<T>& tupleHigh){
    if(tupleLow.isGT(tupleHigh)){
        return nullptr;
    }

    char * zAddressLow = new char[this->metadata->zAddressBytes];
    this->zTools->transformDataToZAddress((char*)tupleLow.getAttributes(), zAddressLow);
    cTuple<T> * zAddrTupleLow = new cTuple<T>((T*)zAddressLow, this->metadata->n);

    char * zAddressHigh = new char[this->metadata->zAddressBytes];
    this->zTools->transformDataToZAddress((char*)tupleHigh.getAttributes(), zAddressHigh);
    cTuple<T> * zAddrTupleHigh = new cTuple<T>((T*)zAddressHigh, this->metadata->n);

    return new cUBTreeIteratorRange<T>(this->root, zAddrTupleLow, zAddrTupleHigh, this->metadata, this->getZTools());
}
template<typename T>
cUBTreeIteratorRangeStack<T>* cUBTree<T>::searchRangeIteratorStack(cTuple<T>& tupleLow, cTuple<T>& tupleHigh) {
    if (tupleLow.isGT(tupleHigh)) {
        return nullptr;
    }

    char* zAddressLow = new char[this->metadata->zAddressBytes];
    this->zTools->transformDataToZAddress((char*)tupleLow.getAttributes(), zAddressLow);
    cTuple<T>* zAddrTupleLow = new cTuple<T>((T*)zAddressLow, this->metadata->n);

    char* zAddressHigh = new char[this->metadata->zAddressBytes];
    this->zTools->transformDataToZAddress((char*)tupleHigh.getAttributes(), zAddressHigh);
    cTuple<T>* zAddrTupleHigh = new cTuple<T>((T*)zAddressHigh, this->metadata->n);

    return new cUBTreeIteratorRangeStack<T>(this->root, zAddrTupleLow, zAddrTupleHigh, this->metadata, this->getZTools());
}

template<typename T>
cUBTreeIteratorRangeStackBin<T>* cUBTree<T>::searchRangeIteratorStackBin(cTuple<T>& tupleLow, cTuple<T>& tupleHigh) {
    if (tupleLow.isGT(tupleHigh)) {
        return nullptr;
    }

    char* zAddressLow = new char[this->metadata->zAddressBytes];
    this->zTools->transformDataToZAddress((char*)tupleLow.getAttributes(), zAddressLow);
    cTuple<T>* zAddrTupleLow = new cTuple<T>((T*)zAddressLow, this->metadata->n);

    char* zAddressHigh = new char[this->metadata->zAddressBytes];
    this->zTools->transformDataToZAddress((char*)tupleHigh.getAttributes(), zAddressHigh);
    cTuple<T>* zAddrTupleHigh = new cTuple<T>((T*)zAddressHigh, this->metadata->n);

    return new cUBTreeIteratorRangeStackBin<T>(this->root, zAddrTupleLow, zAddrTupleHigh, this->metadata, this->getZTools());
}

template<typename T>
cUBTreeIteratorPoint<T>* cUBTree<T>::searchPointIterator(cTuple<T>& tuple){
    char * zAddress = new char[this->metadata->zAddressBytes];
    this->zTools->transformDataToZAddress((char*)tuple.getAttributes(), zAddress);
    cTuple<T> * zAddrTuple = new cTuple<T>((T*)zAddress, this->metadata->n);

    return new cUBTreeIteratorPoint<T>(this->root, zAddrTuple, this->metadata, this->getZTools());
}

template<typename T>
void cUBTree<T>::printTreeTuples(){
    //remake using stack
    //this->root->printNodes(true, 0);

    T* lowTupleUnpacked = (T*)new char[this->metadata->zAddressBytes];
    T* highTupleUnpacked = (T*)new char[this->metadata->zAddressBytes];
    T* tupleUnpacked = (T*)new char[this->metadata->zAddressBytes];
    cStack<cNode<T>*> stack = cStack<cNode<T>*>(this->metadata->order);
    stack.push(this->root, 0);
    int direction = 1; //1 going deeper, -1 going back
    while (!stack.isEmpty()) {
        int position = 0; //position of child to process next
        int order = stack.getOrder();
        cNode<T>* current = stack.pop(position);
        if(!current->isLeafNode()){
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);

            if(position==0 && direction == 1){
                for(int i = 0; i < order; i++){
                    std::cout << "_ ";
                }
                std::cout << "InnerNode: [";
                for(int i = 0; i < innerNode->getCount(); i++){
                    T* lowTuple = (T*)innerNode->getTupleLowPtr(i);
                    T* highTuple = (T*)innerNode->getTupleHighPtr(i);
                    memset(lowTupleUnpacked, 0, this->metadata->zAddressBytes);
                    memset(highTupleUnpacked, 0, this->metadata->zAddressBytes);
                    this->zTools->transformZAddressToData((char*)lowTuple, (char*&)lowTupleUnpacked);
                    this->zTools->transformZAddressToData((char*)highTuple, (char*&)highTupleUnpacked);
                    std::cout << "( ";
                    for(int j = 0; j < this->metadata->n; j++){
                        std::cout << lowTupleUnpacked[j] << " ";
                    }
                    std::cout << ")-( ";
                    for(int j = 0; j < this->metadata->n; j++){
                        std::cout << highTupleUnpacked[j] << " ";
                    }
                    std::cout << ")";
                    if(i != innerNode->getCount()-1){
                        std::cout << ", ";
                    }
                }
                std::cout << "]" << std::endl;
            }
            if(direction == 1){
                stack.push(innerNode, position);
                stack.push(innerNode->getChild(position), 0);
            }
            else if(direction == -1){//returning, either traverse next range or go back further
                if(position+1 < innerNode->getCount()){
                    stack.push(innerNode, position+1);
                    direction = 1;
                }
            }
        }
        else{
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
            for(int i = 0; i < order; i++){
                std::cout << "_ ";
            }
            std::cout << "LeafNode: [";
            for(int i = 0; i < leafNode->getCount(); i++){
                T* tuple = (T*)leafNode->getElementPtr(i);
                memset(tupleUnpacked, 0, this->metadata->zAddressBytes);
                this->zTools->transformZAddressToData((char*)tuple, (char*&)tupleUnpacked);
                std::cout << "( ";
                for(int j = 0; j < this->metadata->n; j++){
                    std::cout << tupleUnpacked[j] << " ";
                }
                std::cout << ")";
                if(i != leafNode->getCount()-1){
                    std::cout << ", ";
                }
            }
            std::cout << "]" << std::endl;
            direction = -1;
        }
    }
    delete[] lowTupleUnpacked;
    delete[] highTupleUnpacked;
    delete[] tupleUnpacked;
}
template<typename T>
void cUBTree<T>::printTreeHex(){
    //remake using stack
    //this->root->printNodes(true, 0);

    cStack<cNode<T>*> stack = cStack<cNode<T>*>(this->metadata->order);
    stack.push(this->root, 0);
    int direction = 1; //1 going deeper, -1 going back
    while (!stack.isEmpty()){
        int position = 0; //position of child to process next
        int order = stack.getOrder();
        cNode<T>* current = stack.pop(position);
        if(!current->isLeafNode()){
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);

            if(position==0 && direction == 1){
                for(int i = 0; i < order; i++){
                    std::cout << "_ ";
                }
                std::cout << "InnerNode: [";
                for(int i = 0; i < innerNode->getCount(); i++){
                    cTuple<T> lowTuple = cTuple<T>((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n, true);
                    cTuple<T> highTuple = cTuple<T>((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n, true);
                    std::cout << "(";
                    lowTuple.printAsZaddress();
                    std::cout << ")-(";
                    highTuple.printAsZaddress();
                    
                    std::cout << ")";
                    if(i != innerNode->getCount()-1){
                        std::cout << ", ";
                    }
                }
                std::cout << "]" << std::endl;
            }
            if(direction == 1){
                stack.push(innerNode, position);
                stack.push(innerNode->getChild(position), 0);
            }
            else if(direction == -1){//returning, either traverse next range or go back further
                if(position+1 < innerNode->getCount()){
                    stack.push(innerNode, position+1);
                    direction = 1;
                }
            }
        }
        else{
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
            for(int i = 0; i < order; i++){
                std::cout << "_ ";
            }
            std::cout << "LeafNode: [";
            for(int i = 0; i < leafNode->getCount(); i++){
                cTuple<T> tuple = cTuple<T>((T*)leafNode->getElementPtr(i), leafNode->metadata->n, true);
                std::cout << "(";
                tuple.printAsZaddress();
                std::cout << ")";
                if(i != leafNode->getCount()-1){
                    std::cout << ", ";
                }
            }
            std::cout << "]" << std::endl;
            direction = -1;
        }
    }
}

template<typename T>
void cUBTree<T>::printMetadata(){
    this->metadata->printMetadata();
}

template<typename T>
int cUBTree<T>::checkLinkLength() {
	cNode<T>* current = this->root;
	int linkLength = 0;
    while (!current->isLeafNode()) {
		cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
		current = innerNode->getChild(0);
	}
    while (current != nullptr) {
		cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(current);
		linkLength += 1;
		current = leafNode->getNodeLink();
	}
	return linkLength;
}   