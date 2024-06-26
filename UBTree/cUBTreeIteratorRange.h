#include "cUBTreeIterator.h"

template<typename T>
class cUBTreeIteratorRange : public cUBTreeIterator<T>{
    protected:
        cTuple<T>* zAddressLow;
        cTuple<T>* zAddressHigh;
    //TODO: save/load state of iterator
        cLeafNode<T>* startNode;
        cLeafNode<T>* currentNode;
        cLeafNode<T>* endNode;
        int startIndex;
        int index;
        int endIndex;
        int currentMaxIndex;
        cTuple<T>* resultContainer;
        bool init() override;
    public: 
        cUBTreeIteratorRange(cNode<T>* root, cTuple<T>* zAddrTupLow, cTuple<T>* zAddrTupHigh, cTreeMetadata* metadata, cZAddrUtils* zTools);
        ~cUBTreeIteratorRange() override;
        bool hasNext() override;
        cTuple<T>* next() override;
        int skip(int count) override;
        bool reset() override;
};
template<typename T>
cUBTreeIteratorRange<T>::cUBTreeIteratorRange(cNode<T>* root, cTuple<T>* zAddrTupLow, cTuple<T>* zAddrTupHigh, cTreeMetadata* metadata, cZAddrUtils* zTools):
    cUBTreeIterator<T>(root, metadata, zTools),
    zAddressLow(zAddrTupLow),
    zAddressHigh(zAddrTupHigh)
    {
        this->init();
        resultContainer = new cTuple<T>(metadata->n, true);
        currentNode = startNode;
        if(currentNode == endNode){
            currentMaxIndex = endIndex;
        }
        else if (currentNode != nullptr){

            currentMaxIndex = currentNode->getCount()-1;
        }
        else{
            currentMaxIndex = 0;
        
        }
};
template<typename T>
cUBTreeIteratorRange<T>::~cUBTreeIteratorRange(){
    delete resultContainer;
    delete zAddressLow;
    delete zAddressHigh;
}

template<typename T>
bool cUBTreeIteratorRange<T>::init(){
    //TODO: Fix case, where range can be intersected, but no tuples of the range are in rectangle
    this->initIsIntersectedCalls = 0;
    this->initRectangeCalls = 0;
    cNode<T>* first = this->root;
    cNode<T>* last = this->root;        
    index = -1;
    endIndex = -1;

    //Now find first leaf node, search until Leaf node is reached
    //std::cout << "Finding first path: ";
    //std::cout.flush();

    while(first != nullptr && index == -1){
        bool isLeaf = first->isLeafNode();
        int count = first->getCount();
        if(isLeaf){
            //find index of first element to copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(first);
            int i = 0;
            for(; i < count; i++) {
                this->initRectangeCalls++;
                if(this->zTools->IsInRectangle(leafNode->getElementPtr(i), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    index = i;
                    //std::cout << i <<"; ";
                    //std::cout.flush();
                    break;
                }
            }
            if(i == count){
                first = dynamic_cast<cNode<T>*>(leafNode->getNodeLink());
                //std::cout << -1 <<"; ";
                //std::cout.flush();
            }
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(first);
            int i = 0;
            for(; i < count; i++) {
                //cTuple<T> tupHighTree = cTuple<T> ((T*)innerNode->getTupleHighPtr(i), innerNode->metadata->n, true);
                //Easy to find last interval it fits in, just HigherEQthan, because lower bound can be lower than first element
                this->initIsIntersectedCalls++;
                if(this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(i), innerNode->getTupleHighPtr(i),(char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    first = innerNode->getChild(i);
                    //std::cout << i <<", ";
                    //std::cout.flush();
                    break;
                }
            }
            //TODO: rethink how we could reach this state, 
            //either rectangle is before or after all zAddrs in this node
            //This should not matter, because with no intersection, no tuples can be found anyways.
            //Therefore always set it to nullptr and in iterator, if nullptr, just return nothing because we reached end
            if(i == count){
                //temporary patchup
                first = innerNode->getChild(count-1);
                //index = 0;
                //std::cout << -1 <<", ";
                //std::cout.flush();
            }
        }
    }
    //Now find last leaf node, search until Leaf node is reached
    //std::cout << "Finding last path: ";
    while(last != nullptr && endIndex == -1){
        bool isLeaf = last->isLeafNode();
        int count = last->getCount();
        if(isLeaf){
            //find index of first element to not copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(last);
            int j = count - 1;
            for(; j >= 0; j--) {
                this->initRectangeCalls++;
                if(this->zTools->IsInRectangle(leafNode->getElementPtr(j), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    endIndex = j;
                    //std::cout << j <<"; ";
                    //std::cout.flush();
                    break;
                }
            }
            if (j == -1){
                endIndex = 0;
                //std::cout << -1 <<"; ";
                //std::cout.flush();
            }
            
        }
        else{//if there is no tuple that is not supposed to be included, we will set inner node tu nullptr,
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(last);
            int j = count-1;
            //check if tuple is higher than last tuple in inner node
            for(; j >= 0; j--) {
                this->initIsIntersectedCalls++;
                if(this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(j), innerNode->getTupleHighPtr(j), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    last = innerNode->getChild(j);
                    //std::cout << j <<", ";
                    //std::cout.flush();
                    break;
                }
            }
            if(j == -1){
                //temporary patchup
                last = innerNode->getChild(0);
                //endIndex = 0;
                //std::cout << -1 <<", ";
            }
        }
    }
    startNode = dynamic_cast<cLeafNode<T>*>(first);
    endNode = dynamic_cast<cLeafNode<T>*>(last);
    startIndex = index;
    //std::cout << std::endl;
    
    return true;
}

template<typename T>
bool cUBTreeIteratorRange<T>::hasNext(){
    //when we went past end or we are at nullptr
    return !((index > currentMaxIndex && currentNode == endNode) || currentNode == nullptr);
}
template<typename T>
cTuple<T>* cUBTreeIteratorRange<T>::next(){
    bool quit = false;
    if(currentNode == nullptr){
        return nullptr;
    }
    resultContainer->setTuple((T*)currentNode->getElementPtr(index), this->metadata->n);
    do{
        index++;
        if(index > currentMaxIndex && currentNode != endNode){
            //TODO: make a loop that will take first and last element and attempt isIntersectedZrQr to know, whether to outright select next linked block
            int count = 0;
            do{
                currentNode = currentNode->getNodeLink();
                count = currentNode->getCount();
                if(currentNode == endNode){
                    currentMaxIndex = endIndex;
                }
                else{
                    currentMaxIndex = count -1;
                }
                index = 0;
                this->isIntersectedCalls++;
            }
            while(
                !this->zTools->IsIntersected_ZrQr_block(
                    currentNode->getElementPtr(0), 
                    currentNode->getElementPtr(count-1),
                    (char*)zAddressLow->getAttributes(), 
                    (char*)zAddressHigh->getAttributes()
                )
            );
        }
        //Do a check if selected result is in rectangle, if not, try next until found
        if((index > endIndex && currentNode == endNode) || currentNode == nullptr){
            quit = true;
        }//first check to avoid checking nonexistent element
        else if (this->zTools->IsInRectangle(currentNode->getElementPtr(index), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
            quit = true;
            this->rectangleCalls++;
        }
        else this->rectangleCalls++;
    } while(!quit);
    return resultContainer;
}

template<typename T>
int cUBTreeIteratorRange<T>::skip(int count){
    int skipped = 0;
    if(!this->hasNext()){
        return skipped;
    }
    while(skipped < count || count == -1){//this iterates count number of elements, at start of this loop, we always have ready element in rectangle
        skipped++;
        while(1){//prepares next valid element
            index++;
            if (currentNode == endNode) {
                int k = 0;
            }
            if(index > currentMaxIndex && currentNode != endNode){
                //TODO: make a loop that will take first and last element and attempt isIntersectedZrQr to know, whether to outright select next linked block
                int currCount = 0;
                do{
                    currentNode = currentNode->getNodeLink();
                    if (currentNode == endNode) {
                        int k = 0;
                    }
                    currCount = currentNode->getCount();
                    if(currentNode == endNode){
                        currentMaxIndex = endIndex;
                    }
                    else{
                        currentMaxIndex = currCount -1;
                    }
                    index = 0;
                    this->isIntersectedCalls++;
                }
                while(
                    !this->zTools->IsIntersected_ZrQr_block(
                        currentNode->getElementPtr(0), 
                        currentNode->getElementPtr(currCount -1),
                        (char*)zAddressLow->getAttributes(), 
                        (char*)zAddressHigh->getAttributes()
                    ) && currentNode != endNode //Necessary check to avoid case, when endNode is not actually intersected, but is only close to actual end
                );
            }
            //first check to avoid checking nonexistent element
            if((index > endIndex && currentNode == endNode) || currentNode == nullptr){
                return skipped;
            }
            else {
                this->rectangleCalls++;
                if(this->zTools->IsInRectangle(currentNode->getElementPtr(index), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())){
                    break;
                }
            }
        }
    } 
    return skipped;
}

template<typename T>
bool cUBTreeIteratorRange<T>::reset(){
    currentNode = startNode;
    index = startIndex;
    if (currentNode == endNode) {
        currentMaxIndex = endIndex;
    }
    else if (currentNode != nullptr) {
        currentMaxIndex = currentNode->getCount() - 1;
    }
    else {
        currentMaxIndex = 0;
    }
    return true;
}