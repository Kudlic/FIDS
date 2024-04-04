#pragma once
#include "cLeafNode.h"
#include "cTuple.h"
#include "cTreeMetadata.h"
#include "cBpTreeIterator.h"

//this one will work as such:
//1. Find first node that has element that is in rectangle
//2. Find last node, that is lower than higher bound, not necessarily in rectangle
//Next will return current element and move to next, when it definitely moved out of bounds, sets current to nullptr
template<typename T>
class cBpTreeIteratorRectangle : public cBpTreeIterator<T>{
    protected:
    //TODOMBY: save state and set it
        cLeafNode<T>* startNode;
        cLeafNode<T>* currentNode;
        cLeafNode<T>* endNode;
        cTuple<T>* lowBound;
        cTuple<T>* highBound;
        int startIndex;
        int index;
        int endIndex;
        int currentMaxIndex;
        cTuple<T>* resultContainer;
    public: 
        cBpTreeIteratorRectangle(cTuple<T>* lowBound, cTuple<T>* highBound, cNode<T>* root, cTreeMetadata<T>* metadata);
        ~cBpTreeIteratorRectangle() override;
        bool hasNext() override;
        cTuple<T>* next() override;
        int skip(int count) override;
        bool reset() override;
};
//function to compare two tuples
template<typename T>
bool isInRectangle(cTuple<T>& lb, cTuple<T>& hb, cTuple<T>& tuple,int n){
    T* aData = lb.getAttributes();
    T* bData = hb.getAttributes();
    T* tupData = tuple.getAttributes();
    for(int i = 0; i < n; i++){
        if(tupData[i] < aData[i] || tupData[i] > bData[i]){
            return false;
        }
    }
    return true;
}
/*
template<typename T>
int cmpTuples(cTuple<T>* a, cTuple<T>* b, int n){
    T* aData = a->getAttributes();
    T* bData = b->getAttributes();
    for(int i = 0; i < n; i++){
        if(aData[i] < bData[i]){
            return -1;
        }
        else if(aData[i] > bData[i]){
            return 1;
        }
    }
    return 0;
}
*/

template<typename T>
cBpTreeIteratorRectangle<T>::cBpTreeIteratorRectangle(cTuple<T>* lowBound, cTuple<T>* highBound, cNode<T>* root, cTreeMetadata<T>* metadata):
    cBpTreeIterator<T>(metadata),
    lowBound(lowBound),
    highBound(highBound)
    {
        cNode<T>* first = root;
        cNode<T>* last = root;
        startIndex = -1;
        endIndex = -1;
        bool firstTreshold = false;

        while(first != nullptr && startIndex == -1){
            bool isLeaf = first->isLeafNode();
            int count = first->getCount();
            if(isLeaf){
                //find index of first element to copy
                cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(first);
                int i = 0;
                if(!firstTreshold){
                    for(; i < count; i++){
                        cTuple<T> element = cTuple<T>((T*)leafNode->getElementPtr(i), this->metadata->n1, true);
                        if(element.isGEQT(*lowBound, this->metadata->n1)){//at this point we are traversing elements that can be in rectangle
                            firstTreshold = true;
                            break;
                        }
                    }
                }
                //in this loop, every element is higher than lower bound of query, now we need to find one that is lower than high bound
                //and is in rectangle
                for(; i < count; i++){
                    cTuple<T> element = cTuple<T>((T*)leafNode->getElementPtr(i), this->metadata->n1, true);
                    if(element.isLEQT(*highBound)){
                        if(isInRectangle(*lowBound, *highBound, element, this->metadata->n1)){
                            startIndex = i;
                            startNode = leafNode;
                            break;
                        }
                    }
                    //if we went past highBound, there are no more elements in rectangle
                    else{
                        first = nullptr;
                        startNode = nullptr;
                    }
                }
                if(i == count){
                    first = leafNode->getNodeLink();
                }
            
            }
            else{
                cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(first);
                int i = 0;
                for(; i < count; i++) {
                    cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), this->metadata->n1, true);
                    //Easy to find last interval it fits in, just HigherEQthan, because lower bound can be lower than first element
                    if(tupHighTree.isGEQT(*lowBound, this->metadata->n1)){
                        first = innerNode->getChild(i);
                        break;
                    }
                }
                if(i == count){
                    first = nullptr;
                    startNode = nullptr;
                }
            }
        }
        currentNode = startNode;
        if(startNode != nullptr){
            while(last != nullptr && endIndex == -1){
                bool isLeaf = last->isLeafNode();
                int count = last->getCount();
                if(isLeaf){
                    //find index of first element to not copy
                    cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(last);
                    int j = count-1;
                    for(; j >= 0; j--) {
                        cTuple<T> tupleTemp = cTuple<T> ((T*)leafNode->getElementPtr(j), this->metadata->n1, true);
                        if(tupleTemp.isLEQT(*highBound, this->metadata->n1)){
                            endIndex = j;
                            endNode = leafNode;
                            break;
                        }
                    }
                    if(j < 0){
                        last = nullptr;
                        endIndex = 0;
                        endNode = nullptr;
                        break;
                    }
                }
                else{
                    cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(last);
                    int j = count-1;
                    for(; j >=0; j--) {
                        cTuple<T> tupLowTree = cTuple<T> ((T*)innerNode->getTupleLowPtr(j), this->metadata->n1, true);
                        if(tupLowTree.isLEQT(*highBound, this->metadata->n1)){
                            last = innerNode->getChild(j);
                            break;
                        }
                    }
                    if(j == -1){
                        last = nullptr;
                        endIndex = 0;
                        break;
                    }
                }
            }
        }

        resultContainer = new cTuple<T>(this->metadata->n, true);
        if(currentNode == endNode && currentNode != nullptr){
            currentMaxIndex = endIndex;
        }
        else if(currentNode != nullptr){
            currentMaxIndex = currentNode->getCount();
        }
};
template<typename T>
cBpTreeIteratorRectangle<T>::~cBpTreeIteratorRectangle(){
    delete resultContainer;
}

template<typename T>
bool cBpTreeIteratorRectangle<T>::hasNext(){
    return currentNode != nullptr && index <= currentMaxIndex;
}

template<typename T>
cTuple<T>* cBpTreeIteratorRectangle<T>::next(){
    if(currentNode == nullptr){
        return nullptr;
    }
    resultContainer->setTuple((T*)currentNode->getElementPtr(index), this->metadata->n);
    cTuple<T> element = cTuple<T>(this->metadata->n1, true);
    do{
        index++;
        if(index >= currentMaxIndex && currentNode == endNode){
            currentNode = nullptr;
            break;
        }
        else if(index >= currentMaxIndex && currentNode != endNode){
            currentNode = currentNode->getNodeLink();
            if(currentNode == endNode){
                currentMaxIndex = endIndex;
            }
            else{
                currentMaxIndex = currentNode->getCount();
            }
            index = 0;
        }
        element.setTuple((T*)currentNode->getElementPtr(index), this->metadata->n1);
    } while (currentNode != endNode && index <= currentMaxIndex && !isInRectangle(*lowBound, *highBound, element, this->metadata->n1));
    if (!hasNext()) {
        currentNode = nullptr;
    }
    return resultContainer;
}
template<typename T>
int cBpTreeIteratorRectangle<T>::skip(int count){
    int skipped = 0;
    while((skipped < count || count == -1) && hasNext()){
        cTuple<T> element = cTuple<T>(this->metadata->n1, true);
        do{
            index++;
            if(index >= currentMaxIndex && currentNode == endNode){
                currentNode = nullptr;
                break;
            }
            else if(index >= currentMaxIndex && currentNode != endNode){
                currentNode = currentNode->getNodeLink();
                if(currentNode == endNode){
                    currentMaxIndex = endIndex;
                }
                else{
                    currentMaxIndex = currentNode->getCount();
                }
                index = 0;
            }
            element.setTuple((T*)currentNode->getElementPtr(index), this->metadata->n1);
        } while (currentNode != endNode && index <= currentMaxIndex && !isInRectangle(*lowBound, *highBound, element, this->metadata->n1));
        skipped++;
    }
    return skipped;
}
template<typename T>
bool cBpTreeIteratorRectangle<T>::reset(){
    currentNode = startNode;
    index = 0;
    if(currentNode == endNode){
        currentMaxIndex = endIndex;
    }
    else{
        currentMaxIndex = currentNode->getCount();
    }
    return true;
}