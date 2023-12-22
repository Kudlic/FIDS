#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include "cSchema.h"

cSchema::cSchema() : AttrCount(0), AttrSize(nullptr), AttrValueCount(nullptr) {}
cSchema::cSchema(int count, int* sizeArray, int* valueCountArray)
    : AttrCount(count), AttrSize(sizeArray), AttrValueCount(valueCountArray) {}

// Getter method for AttrCount
int cSchema::GetAttrCount() {
    return AttrCount;
}

// Getter method for AttrSize
int* cSchema::GetAttrSize() {
    return AttrSize;
}

// Getter method for AttrValueCount
int* cSchema::GetAttrValueCount() {
    return AttrValueCount;
}
// Function to parse the schema.txt file
void cSchema::ParseFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            char* key = strtok(line, ":");
            char* value = strtok(NULL, ":");
            if (key != NULL) {
                if (strcmp(key, "AttrCount") == 0) {
                    if (sscanf(value, "%d", &AttrCount) == 1) {
                        AllocateArrays();
                    }
                } else if (strcmp(key, "AttrSize") == 0) {
                    char * arrVal = strtok(value, ",");
                    for (int i = 0; i < AttrCount; ++i) {
                        if (sscanf(arrVal, "%d", &AttrSize[i]) != 1) {
                            // Handle potential error if parsing fails
                        }
                        arrVal = strtok(NULL, ",");
                    }
                } else if (strcmp(key, "AttrValueCount") == 0) {
                    char * arrVal = strtok(value, ",");
                    for (int i = 0; i < AttrCount; ++i) {
                        if (sscanf(arrVal, "%d", &AttrValueCount[i]) != 1) {
                            // Handle potential error if parsing fails
                        }
                        arrVal = strtok(NULL, ",");
                    }
                }
            }
        }
        fclose(file);
    }
}

// Function to allocate dynamic arrays
void cSchema::AllocateArrays() {
    AttrSize = new int[AttrCount];
    AttrValueCount = new int[AttrCount];
}

void cSchema::PrintSchema(){
    std::cout << "AttrCount: " << AttrCount << std::endl;
    std::cout << "AttrSize: ";
    for (int i = 0; i < AttrCount; ++i) {
        std::cout << AttrSize[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "AttrValueCount: ";
    for (int i = 0; i < AttrCount; ++i) {
        std::cout << AttrValueCount[i] << " ";
    }
    std::cout << std::endl;
}
// Destructor to free dynamic arrays
cSchema::~cSchema() {
    delete[] AttrSize;
    delete[] AttrValueCount;
}
