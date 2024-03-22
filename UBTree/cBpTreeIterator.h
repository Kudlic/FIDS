#include "cLeafNode.h"
#include "cTuple.h"
#include "cTreeMetadata.h"
#include "cZAddrUtils.h"

//Try to change it, so all logic is inside iterator
//Just pass root and two tuples, start and end
//Itarator will have initialise function to find boundaries, including envelopes, to avoid traversing 
//too many nodes.
//Iterator could also store stack if we dont want to utilise linked list in leaf nodes  
template<typename T>
class cBpTreeIterator {
    private:
        cNode<T>* root;
        cTuple<T>* zAddressLow;
        cTuple<T>* zAddressHigh;
        cZAddrUtils* zTools;
    //TODOMBY: save state and set it
        cLeafNode<T>* startNode;
        cLeafNode<T>* currentNode;
        cLeafNode<T>* endNode;
        int startIndex;
        int index;
        int endIndex;
        int currentMaxIndex;
        cTreeMetadata* metadata;
        cTuple<T>* resultContainer;
        bool init();
    public: 
        cBpTreeIterator(cNode<T>* root, cTuple<T>* zAddrTupLow, cTuple<T>* zAddrTupHigh, cTreeMetadata* metadata, cZAddrUtils* zTools);
        ~cBpTreeIterator();
        bool hasNext();
        cTuple<T>* next();
        int skip(int count);
        bool reset();
};
template<typename T>
cBpTreeIterator<T>::cBpTreeIterator(cNode<T>* root, cTuple<T>* zAddrTupLow, cTuple<T>* zAddrTupHigh, cTreeMetadata* metadata, cZAddrUtils* zTools):
    root(root), 
    zAddressLow(zAddrTupLow),
    zAddressHigh(zAddrTupHigh),
    metadata(metadata),
    zTools(zTools)
    {
        this->init();
        resultContainer = new cTuple<T>(metadata->n, true);
        currentNode = startNode;
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
bool cBpTreeIterator<T>::init(){
    //TODO: Fix case, where range can be intersected, but no tuples of the range are in rectangle
    cNode<T>* first = this->root;
    cNode<T>* last = this->root;        
    index = -1;
    endIndex = -1;

    //Now find first leaf node, search until Leaf node is reached
    std::cout << "Finding first path: ";
    std::cout.flush();

    while(first != nullptr && index == -1){
        bool isLeaf = first->isLeafNode();
        int count = first->getCount();
        if(isLeaf){
            //find index of first element to copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(first);
            int i = 0;
            for(; i < count; i++) {
                if(zTools->IsInRectangle(leafNode->getElementPtr(i), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    index = i;
                    std::cout << i <<"; ";
                    std::cout.flush();
                    break;
                }
            }
            
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(first);
            int i = 0;
            for(; i < count; i++) {
                //cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n, true);
                //Easy to find last interval it fits in, just HigherEQthan, because lower bound can be lower than first element
                if(zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(i), innerNode->getTupleHighPtr(i),(char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    first = innerNode->getChild(i);
                    std::cout << i <<", ";
                    std::cout.flush();
                    break;
                }
            }
            //TODO: rethink how we could reach this state, 
            //either rectangle is before or after all zAddrs in this node
            //This should not matter, because with no intersection, no tuples can be found anyways.
            //Therefore always set it to nullptr and in iterator, if nullptr, just return nothing because we reached end
            if(i == count){
                first = nullptr;
                index = 0;
                std::cout << -1 <<", ";
                std::cout.flush();
            }
        }
    }
    //Now find last leaf node, search until Leaf node is reached
    std::cout << "Finding last path: ";
    while(last != nullptr && endIndex == -1){
        bool isLeaf = last->isLeafNode();
        int count = last->getCount();
        if(isLeaf){
            //find index of first element to not copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(last);
            int j = count - 1;
            for(; j >= 0; j--) {
                if(zTools->IsInRectangle(leafNode->getElementPtr(j), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    endIndex = j;
                    std::cout << j <<"; ";
                    std::cout.flush();
                    break;
                }
            }
            if (j == -1){
                endIndex = 0;
                std::cout << -1 <<"; ";
                std::cout.flush();
            }
            
        }
        else{//if there is no tuple that is not supposed to be included, we will set inner node tu nullptr,
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(last);
            int j = count-1;
            //check if tuple is higher than last tuple in inner node
            for(; j >= 0; j--) {
                if(zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(j), innerNode->getTupleHighPtr(j), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    last = innerNode->getChild(j);
                    std::cout << j <<", ";
                    std::cout.flush();
                    break;
                }
            }
            if(j == -1){
                last = nullptr;
                endIndex = 0;
                std::cout << -1 <<", ";
            }
        }
    }
    startNode = dynamic_cast<cLeafNode<T>*>(first);
    endNode = dynamic_cast<cLeafNode<T>*>(last);
    startIndex = index;
    std::cout << std::endl;
    
    return true;
}

template<typename T>
bool cBpTreeIterator<T>::hasNext(){
    //when we went past end or we are at nullptr
    return !((index > currentMaxIndex && currentNode == endNode) || currentNode == nullptr);
}
template<typename T>
cTuple<T>* cBpTreeIterator<T>::next(){
    bool quit = false;
    if(currentNode == nullptr){
        return nullptr;
    }
    do{
        resultContainer->setTuple((T*)currentNode->getElementPtr(index), metadata->n);
        index++;
        if(index >= currentMaxIndex && currentNode != endNode){
            //TODO: make a loop that will take first and last element and attempt isIntersectedZrQr to know, whether to outright select next linked block
            currentNode = currentNode->getNodeLink();
            if(currentNode == endNode){
                currentMaxIndex = endIndex;
            }
            else{
                currentMaxIndex = currentNode->getCount();
            }
            index = 0;
        }
        //Do a check if selected result is in rectangle, if not, try next until found
        if(index > endIndex && currentNode == endNode){
            quit = true;
        }//first check to avoid checking nonexistent element
        else if(zTools->IsInRectangle((char*)resultContainer->getAttributes(), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes()))
            quit = true;

    } while(!quit);
    return resultContainer;
}

template<typename T>
int cBpTreeIterator<T>::skip(int count){
    int skipped = 0;
    if(!this->hasNext()){
        return skipped;
    }
    while(skipped < count || count == -1){//this iterates count number of elements, at start of this loop, we always have ready element in rectangle
        skipped++;
        while(1){//prepares next valid element
            index++;
            if(index >= currentMaxIndex && currentNode != endNode){
                //TODO: make a loop that will take first and last element and attempt isIntersectedZrQr to know, whether to outright select next linked block
                currentNode = currentNode->getNodeLink();
                if(currentNode == endNode){
                    currentMaxIndex = endIndex;
                }
                else{
                    currentMaxIndex = currentNode->getCount();
                }
                index = 0;
            }
            //first check to avoid checking nonexistent element
            if(index > endIndex && currentNode == endNode){
                return skipped;
            }
            else if(zTools->IsInRectangle(currentNode->getElementPtr(index), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                break;
            }
        }
    } 
    return skipped;
}

template<typename T>
bool cBpTreeIterator<T>::reset(){
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