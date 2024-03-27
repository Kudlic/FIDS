#include "cLeafNode.h"
#include "cTuple.h"
#include "cTreeMetadata.h"

template<typename T>
class cBpTreeIteratorRange {
    private:
    //TODOMBY: save state and set it
        cLeafNode<T>* startNode;
        cLeafNode<T>* currentNode;
        cLeafNode<T>* endNode;
        int index;
        int endIndex;
        int currentMaxIndex;
        cTreeMetadata<T>* metadata;
        cTuple<T>* resultContainer;
    public: 
        cBpTreeIteratorRange(cLeafNode<T>* startNode, cLeafNode<T>* endNode, int index, int endIndex, cTreeMetadata<T>* metadata);
        ~cBpTreeIteratorRange();
        bool hasNext();
        cTuple<T>* next();
        int skip(int count);
        bool reset();
};
template<typename T>
cBpTreeIteratorRange<T>::cBpTreeIteratorRange(cLeafNode<T>* startNode, cLeafNode<T>* endNode, int index, int endIndex, cTreeMetadata<T>* metadata):
    startNode(startNode), 
    currentNode(startNode),
    endNode(endNode), 
    index(index), 
    endIndex(endIndex),
    metadata(metadata)
    {
        resultContainer = new cTuple<T>(metadata->n, true);
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
    resultContainer->setTuple((T*)currentNode->getElementPtr(index), metadata->n);
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
    index = 0;
    if(currentNode == endNode){
        currentMaxIndex = endIndex;
    }
    else{
        currentMaxIndex = currentNode->getCount();
    }
    return true;
}