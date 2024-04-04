#pragma once
#include "cLeafNode.h"
#include "cTuple.h"
#include "cTreeMetadata.h"

//Try to change it, so all logic is inside iterator
//Just pass root and two tuples, start and end
//Itarator will have initialise function to find boundaries, including envelopes, to avoid traversing 
//too many nodes.
//Iterator could also store stack if we dont want to utilise linked list in leaf nodes  

template<typename T>
class cBpTreeIterator {
    protected:
        cTreeMetadata<T>* metadata;        
    public:
        cBpTreeIterator(cTreeMetadata<T>* metadata);
        virtual ~cBpTreeIterator();
        virtual bool hasNext() = 0;
        virtual cTuple<T>* next() = 0;
        virtual int skip(int count) = 0;
        virtual bool reset() = 0;
};

template<typename T>
cBpTreeIterator<T>::cBpTreeIterator(cTreeMetadata<T>* metadata)
    : metadata(metadata){
};
template<typename T>
cBpTreeIterator<T>::~cBpTreeIterator(){
};