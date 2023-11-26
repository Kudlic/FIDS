#include <iostream>
#include "cBpTree.h"

int main() {
    // Create a B+ tree with 2 indexed attributes and 2 non-indexed attributes
    // and maximum 3 elements in a inner node and 8 elements in a leaf node
    cBpTree<int> tree(4, 2, 2, 3, 8);

    // Insert some data
    for(int i = 0; i < 10; i++) {
        cTuple tuple(new int[4]{i%6, i/2, i+17, i+31}, 4);
        //tuple.printTuple();
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
    }
    tree.printMetadata();
    tree.printInnerNode();
    cTuple searchTup = cTuple(new int[4]{1, 0, 18, 32}, 4);
    tree.search(searchTup);

    return 0;
}
