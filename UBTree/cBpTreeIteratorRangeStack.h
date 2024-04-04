#pragma once
#include "cBpTreeIterator.h"
#include "cStack.h"

template<typename T>
class cBpTreeIteratorRangeStack : public cBpTreeIterator<T> {
protected:
    cTuple<T>* zAddressLow;
    cTuple<T>* zAddressHigh;
    //TODO: save/load state of iterator

    cStack<cNode<T>*>* nodeStack;

    cTuple<T>* resultContainer;
    bool init() override;
public:
    cBpTreeIteratorRangeStack(cNode<T>* root, cTuple<T>* zAddrTupLow, cTuple<T>* zAddrTupHigh, cTreeMetadata* metadata, cZAddrUtils* zTools);
    ~cBpTreeIteratorRangeStack() override;
    bool hasNext() override;
    cTuple<T>* next() override;
    int skip(int count) override;
    bool reset() override;
};
template<typename T>
cBpTreeIteratorRangeStack<T>::cBpTreeIteratorRangeStack(cNode<T>* root, cTuple<T>* zAddrTupLow, cTuple<T>* zAddrTupHigh, cTreeMetadata* metadata, cZAddrUtils* zTools) :
    cBpTreeIterator<T>(root, metadata, zTools),
    zAddressLow(zAddrTupLow),
    zAddressHigh(zAddrTupHigh)
{   
    this->nodeStack = new cStack<cNode<T>*>(this->metadata->order);
    nodeStack->push(root, 0);
    int direction = 1;
    this->init();
    resultContainer = new cTuple<T>(metadata->n, true);
};
template<typename T>
cBpTreeIteratorRangeStack<T>::~cBpTreeIteratorRangeStack() {
    delete resultContainer;
    delete zAddressLow;
    delete zAddressHigh;
    delete nodeStack;
}

template<typename T>
bool cBpTreeIteratorRangeStack<T>::init() {
    //TODO: Fix case, where range can be intersected, but no tuples of the range are in rectangle
    //TOMBYDO: Rethink stack, you could save up to order-1 * innerNodeSize + 1 * leafNodeSize
    // This way stack could schedule all nodes needed to examine and nodes could be added
    // to stack using binary halving of intervals, so that we can skip whole intervals
    this->initIsIntersectedCalls = 0;
    this->initRectangeCalls = 0;
    while (!nodeStack->isEmpty()) {
        int position = 0;
        cNode<T>* node = nodeStack->pop(position);
        bool isLeaf = node->isLeafNode();
        int count = node->getCount();
        if (isLeaf) {
            //find index of first element to copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(node);
            for (; position < count; position++) {
                this->initRectangeCalls++;
                if (this->zTools->IsInRectangle(leafNode->getElementPtr(position), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    nodeStack->push(node, position);//push current node back to stack with starting position
                    return true;
                }
            }
        }
        else {
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(node);
            for (; position < count; position++) {
                this->initIsIntersectedCalls++;
                if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(position), innerNode->getTupleHighPtr(position), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    nodeStack->push(node, position);//push current node back to stack with position we dive in at
                    nodeStack->push(innerNode->getChild(position), 0);//push child to stack at starting position
                    break;
                }
            }
        }
        if (position == count) {
            //no more elements in leaf node, continue to next node
            if (!nodeStack->isEmpty()) {
                int tmpPos = 0;
                cNode<T>* tmpNode = nodeStack->pop(tmpPos);
                nodeStack->push(tmpNode, tmpPos + 1);
            }
            continue;
        }
    }
    return false;
}

template<typename T>
bool cBpTreeIteratorRangeStack<T>::hasNext() {
    //when we went past end or we are at nullptr
    return !nodeStack->isEmpty();
}
template<typename T>
cTuple<T>* cBpTreeIteratorRangeStack<T>::next() {
    if (nodeStack->isEmpty()) {
        return nullptr;
    }
    int index = 0;
    cNode<T>* currentNode = nodeStack->pop(index);
    bool isLeaf = currentNode->isLeafNode();
    nodeStack->push(currentNode, index + 1); //push current node back to stack with next element, so we start search from it further
    //Setting result
    if(isLeaf){
        resultContainer->setTuple(
            (T*)dynamic_cast<cLeafNode<T>*>(currentNode)->getElementPtr(index), 
            this->metadata->n);
    }
    else {
        //error, we should always start at leaf node with next element
        std::cerr << "cBpTreeIteratorRangeStack::next() - error, inner node should not be next element" << std::endl;
        return nullptr;
    }
    //Find next element
    bool quit = false;
    while (!nodeStack->isEmpty() && !quit) {
        int position = 0;
        cNode<T>* node = nodeStack->pop(position);
        bool isLeaf = node->isLeafNode();
        int count = node->getCount();
        
        if (isLeaf) {
            //find index of first element to copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(node);
            for (; position < count; position++) {
                this->rectangleCalls++;
                if (this->zTools->IsInRectangle(leafNode->getElementPtr(position), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    nodeStack->push(node, position);//push current node back to stack with starting position
                    quit = true;
                    break;
                }
            }
        }
        else {
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(node);
            for (; position < count; position++) {
                this->isIntersectedCalls++;
                if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(position), innerNode->getTupleHighPtr(position), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    nodeStack->push(node, position);//push current node back to stack with position we dive in at
                    nodeStack->push(innerNode->getChild(position), 0);//push child to stack at starting position
                    break;
                }
            }
        }
        //In case we reached end of a node, we should not have pushed anything to stack,
        //thus we should move to next node in parent
        if (position == count) {
            //no more elements in leaf node, continue to next node
            if (!nodeStack->isEmpty()) {
                int tmpPos = 0;
                cNode<T>* tmpNode = nodeStack->pop(tmpPos);
                nodeStack->push(tmpNode, tmpPos + 1);
            }
            continue;
        }
    }
   
    return resultContainer;
}

template<typename T>
int cBpTreeIteratorRangeStack<T>::skip(int count) {
    int skipped = 0;
    while ((count == -1 || skipped < count) && !nodeStack->isEmpty()) {
        skipped++;
        int tmpPos = 0;
        cNode<T>* tmpNode = nodeStack->pop(tmpPos);
        nodeStack->push(tmpNode, tmpPos + 1);
        bool quit = false;
        while (!nodeStack->isEmpty() && !quit) {
            int position = 0;
            cNode<T>* node = nodeStack->pop(position);
            bool isLeaf = node->isLeafNode();
            int nodeCount = node->getCount();
            if (isLeaf) {
                //find index of first element to copy
                cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(node);
                for (; position < nodeCount; position++) {
                    this->rectangleCalls++;
                    if (this->zTools->IsInRectangle(leafNode->getElementPtr(position),  (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                        nodeStack->push(node, position);//push current node back to stack with starting position
                        quit = true;
                        break;
                    }
                }
            }
            else {
                cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(node);
                for (; position < nodeCount; position++) {
                    this->isIntersectedCalls++;
                    if (this->zTools->IsIntersected_ZrQr_block(
                            innerNode->getTupleLowPtr(position), 
                            innerNode->getTupleHighPtr(position), 
                            (char*)zAddressLow->getAttributes(), 
                            (char*)zAddressHigh->getAttributes()
                            )
                        ) 
                    {
                        nodeStack->push(node, position);//push current node back to stack with position we dive in at
                        nodeStack->push(innerNode->getChild(position), 0);//push child to stack at starting position
                        break;
                    }
                }
            }
            //In case we reached end of a node, we should not have pushed anything to stack,
            //thus we should move to next node in parent
            if (position >= nodeCount) {
                //no more elements in leaf node, continue to next node
                if (!nodeStack->isEmpty()) {
                    int tmpPos = 0;
                    cNode<T>* tmpNode = nodeStack->pop(tmpPos);
                    nodeStack->push(tmpNode, tmpPos + 1);
                }
                continue;
            }
        }
    }
    return skipped;
}

template<typename T>
bool cBpTreeIteratorRangeStack<T>::reset() {
    delete nodeStack;
    nodeStack = new cStack<cNode<T>*>(this->metadata->order);
    nodeStack->push(this->root, 0);
    this->init();

    return true;
}