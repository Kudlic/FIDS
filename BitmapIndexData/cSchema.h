#pragma once
class cSchema {
public:
    int AttrCount;
    int* AttrSize;
    int* AttrValueCount;

    cSchema();
    cSchema(int count, int* sizeArray, int* valueCountArray);
    // Getter method for AttrCount
    int GetAttrCount() ;
    // Getter method for AttrSize
    int* GetAttrSize();
    // Getter method for AttrValueCount
    int* GetAttrValueCount();
    // Function to parse the schema.txt file
    void ParseFile(const char* filename);
    // Function to allocate dynamic arrays
    void AllocateArrays();
    void PrintSchema();
    // Destructor to free dynamic arrays
    ~cSchema();
};