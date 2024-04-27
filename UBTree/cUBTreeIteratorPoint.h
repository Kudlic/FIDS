#include "cUBTreeIterator.h"

template<typename T>
class cUBTreeIteratorPoint : public cUBTreeIterator<T>{
    protected:
        cTuple<T>* zAddress;
    //TODO: save/load state of iterator
        cLeafNode<T>* startNode;
        cLeafNode<T>* currentNode;
        int startIndex;
        int index;
        int currentMaxIndex;
        cTuple<T>* resultContainer;
        bool init() override;
    public: 
        cUBTreeIteratorPoint(cNode<T>* root, cTuple<T>* zAddrTup, cTreeMetadata* metadata, cZAddrUtils* zTools);
        ~cUBTreeIteratorPoint() override;
        bool hasNext() override;
        cTuple<T>* next() override;
        int skip(int count) override;
        bool reset() override;
};
template<typename T>
cUBTreeIteratorPoint<T>::cUBTreeIteratorPoint(cNode<T>* root, cTuple<T>* zAddrTup, cTreeMetadata* metadata, cZAddrUtils* zTools):
    cUBTreeIterator<T>(root, metadata, zTools),
    zAddress(zAddrTup)
    {
        this->init();
        resultContainer = new cTuple<T>(metadata->n, true);
        currentNode = startNode;
        if (currentNode != nullptr){
            currentMaxIndex = currentNode->getCount();
        }
        else{
            currentMaxIndex = 0;
        }
};
template<typename T>
cUBTreeIteratorPoint<T>::~cUBTreeIteratorPoint(){
    delete resultContainer;
    delete zAddress;
}

template<typename T>
bool cUBTreeIteratorPoint<T>::init(){
    cNode<T>* first = this->root;
    index = -1;

    //since we search for a specific element, we can use basic comparisons to find it
    while(first != nullptr && index == -1){
        bool isLeaf = first->isLeafNode();
        int count = first->getCount();
        if(isLeaf){
            //find index of first element to copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(first);
            int left = 0;
            int right = count - 1;
            
            while (left <= right) {
                int mid = left + (right - left) / 2;
                int cmpRes = this->zTools->cmpZAddress(leafNode->getElementPtr(mid), (char*)zAddress->getAttributes());
                if (cmpRes == 0) {
                    index = mid; // Update result when target is found
                    right = mid - 1; // Continue searching towards the left
                } else if (cmpRes < 0) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }

            if(index == -1){
                first = nullptr;
            }
            else{
                first = leafNode;
                break;
            }
        }
        else{
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(first);
            int i = -1;
            int left = 0;
            int right = count - 1;
            
            while (left <= right) {
                int mid = left + (right - left) / 2;
                int cmpResLeft = this->zTools->cmpZAddress(innerNode->getTupleLowPtr(mid), (char*)zAddress->getAttributes());
                int cmpResRight = this->zTools->cmpZAddress(innerNode->getTupleHighPtr(mid), (char*)zAddress->getAttributes());
                //using binary search, find the first occurence of range that includes zAddress
                if (cmpResLeft <= 0 && cmpResRight >= 0) {
                    //if we are in valid range, save it and try to find if there is any earlier occurence, if tupLow is equal to zAddress. If tupLow is lower, there isnt any earlier range
                    i = mid;
                    if(cmpResLeft == 0){
                        right = mid - 1;
                    }
                    else{
                        break;
                    }
                } else if (cmpResRight < 0) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }

            if(i == -1){
                first = nullptr;
                index = -1;
            }
            else{
                first = innerNode->getChild(i);
            }
        }
    }
    startIndex = index;
    startNode = dynamic_cast<cLeafNode<T>*>(first);
    return true;
}

template<typename T>
bool cUBTreeIteratorPoint<T>::hasNext(){
    //when we went past end or we are at nullptr
    return (currentNode != nullptr);
}
//returns current element and moves to next if possible, otherwise sets currentNode to nullptr
template<typename T>
cTuple<T>* cUBTreeIteratorPoint<T>::next(){
    if(currentNode == nullptr){
        return nullptr;
    }
    resultContainer->setTuple((T*)currentNode->getElementPtr(index), this->metadata->n);
    index++;
    //if we need to move to next node, attempt to move and check first element
    if(index >= currentMaxIndex){
        currentNode = currentNode->getNodeLink();
        if(currentNode != nullptr){
            currentMaxIndex = currentNode->getCount();
            index = 0;
            //check validity of current element
            if(this->zTools->cmpZAddress(currentNode->getElementPtr(index), (char*)zAddress->getAttributes()) != 0){
                currentNode = nullptr;
            }
        }
    }
    //check validity of current element
    else if(this->zTools->cmpZAddress(currentNode->getElementPtr(index), (char*)zAddress->getAttributes()) != 0){
        currentNode = nullptr;
    }
    return resultContainer;
}

template<typename T>
int cUBTreeIteratorPoint<T>::skip(int count){
    int skipped = 0;
    while((skipped < count || count == -1) && this->hasNext()){//this iterates count number of elements, at start of this loop, we always have ready element in rectangle
        skipped++;
        index++;
        if(index >= currentMaxIndex && currentNode != nullptr){
            currentNode = currentNode->getNodeLink();
            if(currentNode != nullptr){
                currentMaxIndex = currentNode->getCount();
                index = 0;
                //check validity of current element
                if(this->zTools->cmpZAddress(currentNode->getElementPtr(index), (char*)zAddress->getAttributes()) != 0){
                    currentNode = nullptr;
                }
            }
        }
        //check validity of current element
        else if(this->zTools->cmpZAddress(currentNode->getElementPtr(index), (char*)zAddress->getAttributes()) != 0){
            currentNode = nullptr;
        }
    } 
    return skipped;
}

template<typename T>
bool cUBTreeIteratorPoint<T>::reset(){
    currentNode = startNode;
    index = startIndex;
    if (currentNode != nullptr){
        currentMaxIndex = currentNode->getCount();
    }
    return true;
}