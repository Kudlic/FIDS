#pragma once
#include "cLeafNode.h"
#include "cTuple.h"
#include "cTreeMetadata.h"
#include "cBpTreeIterator.h"

template<typename T>
class cBpTreeIteratorRange : public cBpTreeIterator<T>{
    protected:
    //TODOMBY: save state and set it
        cLeafNode<T>* startNode;
        cLeafNode<T>* currentNode;
        cLeafNode<T>* endNode;
        int index;
        int startIndex;
        int endIndex;
        int currentMaxIndex;
        cTuple<T>* resultContainer;
    public: 
        cBpTreeIteratorRange(cLeafNode<T>* startNode, cLeafNode<T>* endNode, int index, int endIndex, cTreeMetadata<T>* metadata);
        ~cBpTreeIteratorRange() override;
        bool hasNext() override;
        cTuple<T>* next() override;
        int skip(int count) override;
        bool reset() override;
};
template<typename T>
cBpTreeIteratorRange<T>::cBpTreeIteratorRange(cLeafNode<T>* startNode, cLeafNode<T>* endNode, int startIndex, int endIndex, cTreeMetadata<T>* metadata):
    cBpTreeIterator<T>(metadata),
    startNode(startNode), 
    currentNode(startNode),
    endNode(endNode), 
    startIndex(startIndex), 
    endIndex(endIndex)
    {
        resultContainer = new cTuple<T>(metadata->n, true);
        index = startIndex;
        if(currentNode == endNode){
            currentMaxIndex = endIndex;
        }
        else{
            currentMaxIndex = currentNode->getCount();
        }
};
template<typename T>
cBpTreeIteratorRange<T>::~cBpTreeIteratorRange(){
    delete resultContainer;
}

template<typename T>
bool cBpTreeIteratorRange<T>::hasNext(){
    return !(index >= currentMaxIndex && currentNode == endNode);
}
template<typename T>
cTuple<T>* cBpTreeIteratorRange<T>::next(){
    
    if(currentNode == nullptr){
        return nullptr;
    }
    resultContainer->setTuple((T*)currentNode->getElementPtr(index), this->metadata->n);
    index++;
    if(index >= currentMaxIndex && currentNode != endNode){
        currentNode = currentNode->getNodeLink();
        if(currentNode == endNode){
            currentMaxIndex = endIndex;
        }
        else{
            currentMaxIndex = currentNode->getCount();
        }
        index = 0;
    }
    return resultContainer;
}
template<typename T>
int cBpTreeIteratorRange<T>::skip(int count){
    int skipped = 0;
    while((skipped < count || count == -1) && hasNext()){
        if(currentNode == nullptr){
            return skipped;
        }
        index++;
        skipped++;
        if(index >= currentMaxIndex && currentNode != endNode){
            currentNode = currentNode->getNodeLink();
            if(currentNode == endNode){
                currentMaxIndex = endIndex;
            }
            else{
                currentMaxIndex = currentNode->getCount();
            }
            index = 0;
        }
    }
    return skipped;
}
template<typename T>
bool cBpTreeIteratorRange<T>::reset(){
    currentNode = startNode;
    index = startIndex;
    if(currentNode == endNode){
        currentMaxIndex = endIndex;
    }
    else{
        currentMaxIndex = currentNode->getCount();
    }
    return true;
}