#include "cLeafNode.h"
#include "cTuple.h"
#include "cTreeMetadata.h"

template<typename T>
class cBpTreeIterator {
    private:
        cLeafNode<T>* currentNode;
        cLeafNode<T>* endNode;
        int index;
        int endIndex;
        int currentMaxIndex;
        cTreeMetadata<T>* metadata;
        cTuple<T>* resultContainer;
    public: 
        cBpTreeIterator(cLeafNode<T>* currentNode, cLeafNode<T>* endNode, int index, int endIndex, cTreeMetadata<T>* metadata);
        ~cBpTreeIterator();
        bool hasNext();
        cTuple<T>* next();
        int skip(int count);
};
template<typename T>
cBpTreeIterator<T>::cBpTreeIterator(cLeafNode<T>* currentNode, cLeafNode<T>* endNode, int index, int endIndex, cTreeMetadata<T>* metadata):
    currentNode(currentNode), 
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
cBpTreeIterator<T>::~cBpTreeIterator(){
    delete resultContainer;
}

template<typename T>
bool cBpTreeIterator<T>::hasNext(){
    return !(index >= currentMaxIndex && currentNode == endNode);
}
template<typename T>
cTuple<T>* cBpTreeIterator<T>::next(){
    
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
int cBpTreeIterator<T>::skip(int count){
    int skipped = 0;
    while(skipped < count || count == -1){
        if(index >= currentMaxIndex && currentNode != nullptr){
            currentNode = currentNode->getNodeLink();
            if(currentNode == endNode){
                currentMaxIndex = endIndex;
            }
            else{
                currentMaxIndex = currentNode->getCount();
            }
            index = 0;
        }
        else if(currentNode == nullptr){
            return skipped;
        }
        index++;
        skipped++;
    }
    return skipped;
}