#pragma once
#pragma once
#include "cBpTreeIterator.h"
#include "cStack.h"
#include <stack>

template<typename T>
class cBpTreeIteratorRangeStackBin : public cBpTreeIterator<T> {
private:
    struct halvingRange {
        cNode<T>* node;
        int lowIndex;
        int highIndex;
    };
protected:
    cTuple<T>* zAddressLow;
    cTuple<T>* zAddressHigh;
    //TODO: save/load state of iterator

    std::stack<halvingRange>* stack;
    cTuple<T>* resultContainer;
    bool init() override;
public:
    cBpTreeIteratorRangeStackBin(cNode<T>* root, cTuple<T>* zAddrTupLow, cTuple<T>* zAddrTupHigh, cTreeMetadata* metadata, cZAddrUtils* zTools);
    ~cBpTreeIteratorRangeStackBin() override;
    bool hasNext() override;
    cTuple<T>* next() override;
    int skip(int count) override;
    bool reset() override;
};
template<typename T>
cBpTreeIteratorRangeStackBin<T>::cBpTreeIteratorRangeStackBin(cNode<T>* root, cTuple<T>* zAddrTupLow, cTuple<T>* zAddrTupHigh, cTreeMetadata* metadata, cZAddrUtils* zTools) :
    cBpTreeIterator<T>(root, metadata, zTools),
    zAddressLow(zAddrTupLow),
    zAddressHigh(zAddrTupHigh)
{   
    this->initIsIntersectedCalls = 0;
    this->initRectangeCalls = 0;
    this->isIntersectedCalls = 0;
    this->rectangleCalls = 0;
    this->stack = new std::stack<halvingRange>;
    this->init();
    this->resultContainer = new cTuple<T>(metadata->n, true);
};
template<typename T>
cBpTreeIteratorRangeStackBin<T>::~cBpTreeIteratorRangeStackBin() {
    delete resultContainer;
    delete zAddressLow;
    delete zAddressHigh;
    delete stack;
}

template<typename T>
bool cBpTreeIteratorRangeStackBin<T>::init() {
    //New stack works as follows:
    //  Root is pushed to stack with nodePos = -1
    //  Before diving into any innerNode, all innerNodes must be tested for intersection
    //  If innerNode is intersected, it is pushed to stack as (parent, posInParent) -> this is therefore link to either a child LeafNode or child InnerNode
    //  If leafNode is inRectangle, it is pushed to stack as (leafNode, posInParent) -> this is a link to Tuple in LeafNode
    //  
    //  Intersection is tested utilizing binary halving of intervals, where we split interval in half and test both for intersection
    //  If we are processing leafNode, test intersection for halves similarly to innerNode
    //  When testing a single point, we test if it is in rectangle and push it to stack with position if is. NodeStack will therefore 
    this->initIsIntersectedCalls = 0;
    this->initRectangeCalls = 0;
    this->stack->push({ this->root, 0, this->root->getCount()-1 });
    bool quit = false;
    while (!stack->empty() && !quit) {
        halvingRange range = stack->top();
        stack->pop();
        cNode<T>* node = range.node;
        int low = range.lowIndex;
        int high = range.highIndex;
		int halfPosition = (low + high) / 2;

        bool isLeaf = node->isLeafNode();
        int count = node->getCount();

        if (isLeaf) {
            //find index of first element to copy
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(node);
            if(low == high) {//if single point
				this->initRectangeCalls++;
				if (this->zTools->IsInRectangle(leafNode->getElementPtr(low), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
					stack->push({ node, low, low });//Push only if valid. Elements with high high are verified in rectangle
                    quit = true;
                    break;
				}
                else {
                    //error, impossible to have single point not in rectangle
                    std::cerr << "cBpTreeIteratorRangeStack::init() - error, single point not in rectangle" << std::endl;
                    quit = true;
                    break;
                }
			}
            //here we have to check if left and right are same before, since we only have one element on one index
            if (halfPosition+1 < high) {//if not single point
				this->initIsIntersectedCalls++;
				if (this->zTools->IsIntersected_ZrQr_block(leafNode->getElementPtr(halfPosition + 1), leafNode->getElementPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    stack->push({ node, halfPosition + 1, high }); //If we reached point where left and right are same, we can push it to stack
				}
			}
            else {//if single point
                this->initRectangeCalls++;
                if(this->zTools->IsInRectangle(leafNode->getElementPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    stack->push({ node, high, high });//Push only if valid. Elements with high high are verified in rectangle
				}
            }
            if (low < halfPosition) {//if not single point
                this->initIsIntersectedCalls++;
                if (this->zTools->IsIntersected_ZrQr_block(leafNode->getElementPtr(low), leafNode->getElementPtr(halfPosition), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    stack->push({ node, low, halfPosition }); //If we reached point where left and right are same, we can push it to stack
                }
            }
            else {//if single point
                this->initRectangeCalls++;
				if (this->zTools->IsInRectangle(leafNode->getElementPtr(low), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    stack->push({ node, low, low });//Push only if valid. Elements with high high are verified in rectangle
				}
			}
        }
        else {
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(node);
            //Insertion in reverse order, so we can pop in correct order
			this->initIsIntersectedCalls++;
            if(this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(halfPosition+1), innerNode->getTupleHighPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                if (halfPosition + 1 < high)
                    stack->push({ innerNode, halfPosition + 1, high });
                else
                    stack->push({ innerNode->getChild(high), 0, innerNode->getChild(high)->getCount() - 1 }); //If we reached point where left and right are same, we can push it to stack
            }
            this->initIsIntersectedCalls++;
            if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(low), innerNode->getTupleHighPtr(halfPosition), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                if (low < halfPosition)
                    stack->push({ innerNode, low, halfPosition });
                else 
                    stack->push({ innerNode->getChild(low), 0, innerNode->getChild(low)->getCount() - 1 }); //If we reached point where left and right are same, we can push it to stack
			}
        }
    }
    return true;
}

template<typename T>
bool cBpTreeIteratorRangeStackBin<T>::hasNext() {
    //when we went past end or we are at nullptr
    return !stack->empty();
}
template<typename T>
cTuple<T>* cBpTreeIteratorRangeStackBin<T>::next() {
    if (stack->empty()) {
        return nullptr;
    }
    //First extract 1st element in stack, which should always be in rectangle
    halvingRange resultRange = stack->top();
    stack->pop();
    cNode<T>* resultNode = resultRange.node;
    if(resultNode->isLeafNode() && (resultRange.lowIndex == resultRange.highIndex))
        {
		resultContainer->setTuple(
			(T*)dynamic_cast<cLeafNode<T>*>(resultNode)->getElementPtr(resultRange.lowIndex),
			this->metadata->n);
	}
    else{
		//error, we should always start at leaf node with next element
		std::cerr << "cBpTreeIteratorRangeStack::next() - error, inner node should not be next element" << std::endl;
		return nullptr;
	}
    bool quit = false;
    //Process nodes until next suitable element
    while (!stack->empty() && !quit) {
        halvingRange range = stack->top();
        stack->pop();
        cNode<T>* node = range.node;
        int low = range.lowIndex;
        int high = range.highIndex;
        int halfPosition = (low + high) / 2;

        bool isLeaf = node->isLeafNode();
        int count = node->getCount();

        if (isLeaf) {
            cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(node);
            //Element in rectangle found, stop in there
			quit = true;
            break;
            
            if(halfPosition+1 < high){
                this->isIntersectedCalls++;
                if (this->zTools->IsIntersected_ZrQr_block(leafNode->getElementPtr(halfPosition + 1), leafNode->getElementPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    stack->push({ node, halfPosition + 1, high }); //If we reached point where left and right are same, we can push it to stack
                }
            }
            else {//if single point
                this->rectangleCalls++;
                if (this->zTools->IsInRectangle(leafNode->getElementPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    stack->push({ node, high, high });//Push only if valid. Elements with high high are verified in rectangle
                }
            }

            if (low < halfPosition) {//if not single point
                this->isIntersectedCalls++;
                if (this->zTools->IsIntersected_ZrQr_block(leafNode->getElementPtr(low), leafNode->getElementPtr(halfPosition), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    stack->push({ node, low, halfPosition }); //If we reached point where left and right are same, we can push it to stack
                }
            }
            else {//if single point
                this->initRectangeCalls++;
                if (this->zTools->IsInRectangle(leafNode->getElementPtr(low), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    stack->push({ node, low, low });//Push only if valid. Elements with high high are verified in rectangle
                }
            }
        }
        else {
            cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(node);
			//Insertion in reverse order, so we can pop in correct order
			this->isIntersectedCalls++;
			if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(halfPosition + 1), innerNode->getTupleHighPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                if (halfPosition + 1 < high)
                    stack->push({ innerNode, halfPosition + 1, high });
                else
                    stack->push({ innerNode->getChild(high), 0, innerNode->getChild(high)->getCount() - 1 }); //If we reached point where left and right are same, we can push it to stack
			}
			this->isIntersectedCalls++;
			if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(low), innerNode->getTupleHighPtr(halfPosition), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                if (low < halfPosition)
                    stack->push({ innerNode, low, halfPosition });
                else
                    stack->push({ innerNode->getChild(low), 0, innerNode->getChild(low)->getCount() - 1 }); //If we reached point where left and right are same, we can push it to stack
			}
            /*
            for(int i = high; i >= low; i--){
				this->isIntersectedCalls++;
				if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(i), innerNode->getTupleHighPtr(i), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
					stack->push({ innerNode->getChild(i), 0, innerNode->getChild(i)->getCount() - 1 });
				}
			}
            */
        }
    }
    return resultContainer;
}

template<typename T>
int cBpTreeIteratorRangeStackBin<T>::skip(int count) {
    int skipped = 0;
    if (stack->empty()) {
        return skipped;
    }
    while (!stack->empty() && (skipped < count || count == -1)) {
        //First extract 1st element in stack, which should always be in rectangle
        halvingRange resultRange = stack->top();
        stack->pop();
        skipped++;
        bool quit = false;
        
        //Process nodes until next suitable element
        while (!stack->empty() && !quit) {
            halvingRange range = stack->top();
            stack->pop();
            cNode<T>* node = range.node;
            int low = range.lowIndex;
            int high = range.highIndex;
            int halfPosition = (low + high) / 2;

            bool isLeaf = node->isLeafNode();
            int count = node->getCount();

            if (isLeaf) {
                cLeafNode<T>* leafNode = dynamic_cast<cLeafNode<T>*>(node);
                //Element in rectangle found, stop in there
                if (low == high) {
                    this->rectangleCalls++;
                    if (this->zTools->IsInRectangle(leafNode->getElementPtr(low), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                        stack->push({ node, low, low });//Push only if valid. Elements with high high are verified in rectangle
                        quit = true;
                        break;
                    }
                    else {
                        //error, impossible to have single point not in rectangle
                        std::cerr << "cBpTreeIteratorRangeStack::init() - error, single point not in rectangle" << std::endl;
                        quit = true;
                        break;
                    }
                }

                if (halfPosition + 1 < high) {
                    this->isIntersectedCalls++;
                    if (this->zTools->IsIntersected_ZrQr_block(leafNode->getElementPtr(halfPosition + 1), leafNode->getElementPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                        stack->push({ node, halfPosition + 1, high }); //If we reached point where left and right are same, we can push it to stack
                    }
                }
                else {//if single point
                    this->rectangleCalls++;
                    if (this->zTools->IsInRectangle(leafNode->getElementPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                        stack->push({ node, high, high });//Push only if valid. Elements with high high are verified in rectangle
                    }
                }

                if (low < halfPosition) {//if not single point
                    this->isIntersectedCalls++;
                    if (this->zTools->IsIntersected_ZrQr_block(leafNode->getElementPtr(low), leafNode->getElementPtr(halfPosition), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                        stack->push({ node, low, halfPosition }); //If we reached point where left and right are same, we can push it to stack
                    }
                }
                else {//if single point
                    this->initRectangeCalls++;
                    if (this->zTools->IsInRectangle(leafNode->getElementPtr(low), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                        stack->push({ node, low, low });//Push only if valid. Elements with high high are verified in rectangle
                    }
                }
            }
            else {
                cInnerNode<T>* innerNode = dynamic_cast<cInnerNode<T>*>(node);
                //Insertion in reverse order, so we can pop in correct order
                this->isIntersectedCalls++;
                if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(halfPosition + 1), innerNode->getTupleHighPtr(high), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    if (halfPosition + 1 < high)
                        stack->push({ innerNode, halfPosition + 1, high });
                    else
                        stack->push({ innerNode->getChild(high), 0, innerNode->getChild(high)->getCount() - 1 }); //If we reached point where left and right are same, we can push it to stack
                }
                this->isIntersectedCalls++;
                if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(low), innerNode->getTupleHighPtr(halfPosition), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
                    if (low < halfPosition)
                        stack->push({ innerNode, low, halfPosition });
                    else
                        stack->push({ innerNode->getChild(low), 0, innerNode->getChild(low)->getCount() - 1 }); //If we reached point where left and right are same, we can push it to stack
                }
                /*
                for (int i = high; i >= low; i--) {
                    this->isIntersectedCalls++;
					if (this->zTools->IsIntersected_ZrQr_block(innerNode->getTupleLowPtr(i), innerNode->getTupleHighPtr(i), (char*)zAddressLow->getAttributes(), (char*)zAddressHigh->getAttributes())) {
						stack->push({ innerNode->getChild(i), 0, innerNode->getChild(i)->getCount() - 1 });
					}
				
                }
                */
            }
        }
    }
    return skipped;
}

template<typename T>
bool cBpTreeIteratorRangeStackBin<T>::reset() {
    delete this->stack;
    this->stack = new std::stack<halvingRange>;
    this->initIsIntersectedCalls = 0;
    this->initRectangeCalls = 0;
    this->isIntersectedCalls = 0;
    this->rectangleCalls = 0;
    this->init();

    return true;
}