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

//How will conversion to zAddresses work:
// 1. cTuple will become a container for zAddress of type B instead of tuple of type T, all methods will stay the same otherwise
// 2. cUBTree methods will accept T type tuples, but will covert them to zAddresses at start of methods
// 3. cUbTree will use functions from ZAddressTools. Based on configuration, it will initialise metadata and also store function pointers to best functions for selection
// 4. ZAddress implementation only counts with 32bit integers. I need to type it to support other types.
// 5. Gotta find out if overhang is accounted for in zAddressTools algos

template<typename T, typename B, typename C>//T for incoming tuple type, B for block type; 
class cBpTree {
private:
    cNode<T>* root; //T type to signalise type converted from, data is always stored as char
    cTreeMetadata<T> * metadata;
    //cZaddressTools<T, B> * zTools;

public:
    cBpTree(int dimensions, int maxInnerNodeElements, int maxLeafNodeElements);
    ~cBpTree();
    double getBpTreeBytes();
    int getTupleCount();
    cTreeMetadata<T> * getMetadata();
    bool insert(cTuple<T>& tuple);
    bool remove(cTuple<T>& tuple);
    int searchLinkedList(cTuple<T>& tupleLow, cTuple<T>& tupleHigh);
    cBpTreeIterator<T>* searchRangeIterator(cTuple<T>& tupleLow, cTuple<T>& tupleHigh);
    void printTree();
    void printMetadata();

    //UB tree
    void transformDataToZAddress(T* data, char*& za);//msB at index 0
    void transformZAddressToData(char* za, T*& data);//msB at index 0
};

template<typename T, typename B>
cBpTree<T, B>::cBpTree(int dimensions, int maxInnerNodeElements, int maxLeafNodeElements){
    //start with leaf node, after it overflows, it will be split into inner node
    this->metadata = new cTreeMetadata<T>(dimensions, maxInnerNodeElements, maxLeafNodeElements);
    this->root = new cLeafNode<T>(this->metadata);
    this->metadata->order++;
}
template<typename T, typename B>
cBpTree<T, B>::~cBpTree() {
    delete this->root;
    delete this->metadata;
}
template<typename T, typename B>
double cBpTree<T, B>::getBpTreeBytes(){
    return this->metadata->getBpTreeAllocatedBytes();
}
template<typename T, typename B>
int cBpTree<T, B>::getTupleCount(){
    return this->metadata->tupleCount;
}
template<typename T, typename B>
cTreeMetadata<T>* cBpTree<T, B>::getMetadata(){
    return this->metadata;
}
template<typename T, typename B>
bool cBpTree<T, B>::insert(cTuple<T>& tuple) {
    //TODO: dont forget to dealloc
    char * zAddress;
    this->transformDataToZAddress(tuple.getAttributes(), zAddress);
    cTuple<char> zAddrTuple = cTuple<char>(zAddress, this->metadata->zAddressBytes);

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
                    cTuple<char> tupleLowObj = cTuple<char> ((char*)innerNode->getTupleLowPtr(i), innerNode->metadata->zAddressBytes, true);
                    cTuple<char> tupleHighObj = cTuple<char> ((char*)innerNode->getTupleHighPtr(i), innerNode->metadata->zAddressBytes, true);

                    if(zAddrTuple.isLT(tupleLowObj)) {
                        //If tuple is lower than first tuple in inner node, insert into first child
                        if(i == 0){
                            childIndex = 0;
                            innerNode->widenLow(childIndex, &zAddrTuple);
                        }
                        //Else insert into previous child
                        else{
                            childIndex = i-1;
                            innerNode->widenHigh(childIndex, &zAddrTuple);
                        }
                        child = dynamic_cast<cLeafNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                    else if(zAddrTuple.isTupleBetween(tupleLowObj, tupleHighObj)) {
                        childIndex = i;
                        child = dynamic_cast<cLeafNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                }   
                //If child node was not found, insert tuple into last child, as all ranges must have been too low
                if(child == nullptr) {
                    childIndex = count - 1;
                    innerNode->widenHigh(childIndex, &zAddrTuple);
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
                    cTuple<char> tupleLowObj = cTuple<char> ((char*)innerNode->getTupleLowPtr(i), innerNode->metadata->zAddressBytes, true);
                    cTuple<char> tupleHighObj = cTuple<char> ((char*)innerNode->getTupleHighPtr(i), innerNode->metadata->zAddressBytes, true);

                    if(zAddrTuple.isLT(tupleLowObj)) {
                        if(i == 0){
                            childIndex = 0;
                            innerNode->widenLow(childIndex, &zAddrTuple);
                        }
                        else{
                            childIndex = i-1;
                            innerNode->widenHigh(childIndex, &zAddrTuple);
                        }
                        child = dynamic_cast<cInnerNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                    else if(zAddrTuple.isTupleBetween(tupleLowObj, tupleHighObj)) {
                        childIndex = i;
                        child = dynamic_cast<cInnerNode<T>*>(innerNode->getChild(childIndex));
                        break;
                    }
                }
                //If child node was not found, insert tuple into last child, as all ranges must have been too low
                if(child == nullptr) {
                    childIndex = count - 1;
                    innerNode->widenHigh(childIndex, &zAddrTuple);
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
            leafNode->insertTuple(&zAddrTuple);
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
template<typename T, typename B>
bool cBpTree<T, B>::remove(cTuple<T>& tuple){

    //TODO: dont forget to dealloc
    char * zAddress;
    this->transformDataToZAddress(tuple.getAttributes(), zAddress);
    cTuple<char> * zAddrTuple = new cTuple<char>(zAddress, this->metadata->zAddressBytes);

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
                cTuple<char> tupHigh = cTuple<char> ((char*)innerNode->getTupleHighPtr(i), innerNode->metadata->zAddressBytes, true);
                if(tupHigh.isGEQT(tuple, innerNode->metadata->zAddressBytes)){
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
                cTuple<char> tempTuple = cTuple<char>((char*)leafNode->getElementPtr(i), leafNode->metadata->zAddressBytes, true);
                if(tuple.isEQ(tempTuple, leafNode->metadata->zAddressBytes)){
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
                        cTuple<char> borrowedTuple = cTuple<char>((char*)sibling->getElementPtr(0), sibling->metadata->zAddressBytes, true);
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
                        cTuple<char> borrowedTuple = cTuple<char>((char*)sibling->getElementPtr(sibling->getCount()-1), sibling->metadata->zAddressBytes, true);
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

template<typename T, typename B>
int cBpTree<T, B>::searchLinkedList(cTuple<T>& tupleLow, cTuple<T>& tupleHigh) {
    cNode<T>* current = this->root;
    int count = -1;
    int foundTuples = 0;
    while (current != nullptr) {
        if(!current->isLeafNode()){
            count = current->getCount();
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(current);
            int i = 0;
            for(; i < count; i++) {
                cTuple<T> tupLowTree = cTuple<T> ((T*)innerNode->getTupleLowPtr(i), innerNode->metadata->n, true);
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n, true);
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

template<typename T, typename B>
cBpTreeIterator<T>* cBpTree<T, B>::searchRangeIterator(cTuple<T>& tupleLow, cTuple<T>& tupleHigh){
    int resultCount = 0;
    int indexedValues = this->metadata->n;

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
                cTuple<T> tupleTemp = cTuple<T> ((T*)leafNode->getElementPtr(i), first->metadata->n, true);
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
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n, true);
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
                cTuple<T> tupleTemp = cTuple<T> ((T*)leafNode->getElementPtr(j), last->metadata->n, true);
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
                cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(j), innerNode->metadata->n, true);
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
    return new cBpTreeIterator<T>(dynamic_cast<cLeafNode<T>*>(first), dynamic_cast<cLeafNode<T>*>(last), indexFirst, indexLast, this->metadata);

}
template<typename T, typename B>
void cBpTree<T, B>::printTree(){
    //remake using stack
    //this->root->printNodes(true, 0);
}
template<typename T, typename B>
void cBpTree<T, B>::printMetadata(){
    this->metadata->printMetadata();
}

template<typename T, typename B>
void cBpTree<T, B>::transformDataToZAddress(T* data, char*& za){
    if(za == nullptr)
        za = new char[metadata->zAddressBytes];
    T* dataBlock = data;
	uint bitOrder = 0;
	T* zAddrBlock = (T*)za;
	memset(za, 0, metadata->zAddressBytes);

    uint TBitSize = sizeof(T)*8;

	for (uint i = 0; i < TBitSize; i++)
	{
		T mask = 1 << i; //mask saying what bit to pick out
		for (uint j = 0; j < this->metadata->n; j++, bitOrder++)
		{
			T bit = dataBlock[j] & mask;
			if (bit > 0)
			{
				uint blockPosition = bitOrder / TBitSize; // pozice bloku v ramci pole
				uint shift = bitOrder % TBitSize; //bit order v ramci bloku
				zAddrBlock[blockPosition] |= (1 << shift);
			}
		}
	}
}
template<typename T, typename B>
void cBpTree<T, B>::transformZAddressToData(char* za, T*& data){
    if(data == nullptr)
        data = new T[metadata->n];
    T* zAddrBlock = (T*)za;
    uint bitOrder = 0;
    T* dataBlock = data;
    memset(data, 0, metadata->n);

    uint TBitSize = sizeof(T)*8;

    for (uint i = 0; i < TBitSize; i++)
    {
        T mask = 1 << i; //bit we are extracting
        for (uint j = 0; j < this->metadata->n; j++, bitOrder++)
        {
            uint blockPosition = bitOrder / TBitSize; // pozice bloku v ramci pole
            uint shift = bitOrder % TBitSize; //bit order v ramci bloku
            
            T bit = zAddrBlock[blockPosition] & (1 << shift); //
            if (bit > 0)
            {
                dataBlock[j] |= (mask);
            }
        }
    }
}
