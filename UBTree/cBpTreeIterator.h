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
class cBpTreeIterator {
    protected:
        cNode<T>* root;
        cZAddrUtils* zTools;
        cTreeMetadata* metadata;
        int dimensions;
        
        virtual bool init() = 0;
    public:
        cBpTreeIterator(cNode<T>* root, cTreeMetadata* metadata, cZAddrUtils* zTools);
        virtual ~cBpTreeIterator();
        virtual bool hasNext() = 0;
        virtual cTuple<T>* next() = 0;
        virtual int skip(int count) = 0;
        virtual bool reset() = 0;
};

template<typename T>
cBpTreeIterator<T>::cBpTreeIterator(cNode<T>* root, cTreeMetadata* metadata, cZAddrUtils* zTools)
    : root(root), metadata(metadata), zTools(zTools){ dimensions = metadata->n;
};
template<typename T>
cBpTreeIterator<T>::~cBpTreeIterator(){
};