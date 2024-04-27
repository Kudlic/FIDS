#pragma once
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
class cUBTreeIterator {
    protected:
        cNode<T>* root;
        cZAddrUtils* zTools;
        cTreeMetadata* metadata;
        int dimensions;
        virtual bool init() = 0;
    public:
        int initRectangeCalls;
        int initIsIntersectedCalls;
        int rectangleCalls;
        int isIntersectedCalls;

        cUBTreeIterator(cNode<T>* root, cTreeMetadata* metadata, cZAddrUtils* zTools);
        virtual ~cUBTreeIterator();
        virtual bool hasNext() = 0;
        virtual cTuple<T>* next() = 0;
        virtual int skip(int count) = 0;
        virtual bool reset() = 0;
        void resetCallCounters();
};

template<typename T>
cUBTreeIterator<T>::cUBTreeIterator(cNode<T>* root, cTreeMetadata* metadata, cZAddrUtils* zTools)
    : root(root), metadata(metadata), zTools(zTools){ dimensions = metadata->n; rectangleCalls = 0; isIntersectedCalls = 0;
};
template<typename T>
cUBTreeIterator<T>::~cUBTreeIterator(){
};
template<typename T>
void cUBTreeIterator<T>::resetCallCounters() { rectangleCalls = 0; isIntersectedCalls = 0; };