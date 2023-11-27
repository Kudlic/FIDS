#include <iostream>
#include "cBpTree.h"
#include <stdlib.h>


int main() {
    // Create a B+ tree with 2 indexed attributes and 2 non-indexed attributes
    // and maximum 3 elements in a inner node and 8 elements in a leaf node

//TEST 1 recasting
/*
    cTuple<int> tuple(new int[4]{2897, 54863, 71389, 111111}, 4);
    char* tuplePtr = new char[16];
    memcpy(tuplePtr, tuple.attributes, 16);
    cTuple<int> tuple2((int*)tuplePtr, 4);

    tuple.printTuple();
    tuple2.printTuple();
*/
//TEST 2 pointer serialisation
/*
    cTuple<int> *tuple = new cTuple<int>(new int[4]{2897, 54863, 71389, 111111}, 4);
    printf("Tuple pointer: %p\n", tuple);
    printf("Tuple address: %p\n", &tuple);

    char* tuplePtr = new char[16];
    printf("Sizeof tuplePtr: %d\n", sizeof(tuple));
    printf("Sizeof tuple: %d\n", sizeof(cTuple<int>));
    memcpy(tuplePtr, &tuple, sizeof(tuple));

    //print tuplePtr in hexa
    for(int i = 0; i < 16; i++){
        printf("%02x", tuplePtr[i]);
    }
    printf("\n");

    cTuple<int> *tuple2;
    memcpy(&tuple2, tuplePtr, sizeof(char*));

    printf("Tuple2 pointer: %p\n", tuple2);
    printf("Tuple2 address: %p\n", &tuple2);
    tuple->printTuple();
    tuple2->printTuple();
*/

//TESTCASE 1

    cBpTree<int> tree(4, 2, 2, 5, 6);

    // Insert some data
    srand(170400);
    for(int i = 0; i < 20; i++) {
        cTuple tuple(new int[4]{rand()%7, rand()%13, i+17, i+31}, 4);
        //tuple.printTuple();
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
        tree.printBpTree();
    }
    tree.printMetadata();
    //cTuple searchTup = cTuple(new int[4]{1, 0, 18, 32}, 4);
    //tree.search(searchTup);

//TESTCASE 2
/*
    cBpTree<int> tree(2, 1, 1, 6, 5);
    int* vals = new int[9]{1,11,17,12,5,13,6,7,8};
    for(int i = 0; i < 9; i++) {
        cTuple tuple(new int[2]{vals[i], i}, 2);
        if(!tree.insert(tuple)){
            printf("Insertion failed!\n");
            break;
        }
        tree.printBpTree();
        printf("\n");
        //tree.printMetadata();
    }
*/
    return 0;
}
