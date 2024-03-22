#include <iostream>

class BlockProcessor {
private:
    int blockSize;
    typedef bool (BlockProcessor::*BlockFunc)(bool); // Function pointer type

public:
    BlockProcessor(int size) : blockSize(size) {
        // Save function pointer based on block size
        if (blockSize == 1) {
            blockFuncPtr = &BlockProcessor::block1;
        } else if (blockSize == 2) {
            blockFuncPtr = &BlockProcessor::block2;
        } else if (blockSize == 4) {
            blockFuncPtr = &BlockProcessor::block4;
        } else if (blockSize == 8) {
            blockFuncPtr = &BlockProcessor::block8;
        } else {
            std::cerr << "Unsupported block size\n";
            blockFuncPtr = nullptr;
        }
    }

    // Call the appropriate function based on block size
    void processBlock() {
        if (blockFuncPtr) {
            (this->*blockFuncPtr)(true);
        } else {
            std::cerr << "No function pointer set\n";
        }
    }

private:
    BlockFunc blockFuncPtr; // Function pointer member
    bool block1(bool placeholder = 1);
    bool block2(bool placeholder = 1);
    bool block4(bool placeholder = 1) {std::cout << "Processing block size 4\n";}
    bool block8(bool placeholder = 1) {std::cout << "Processing block size 8\n";}
};
bool BlockProcessor::block1(bool placeholder){
    std::cout << "Processing block size 1\n";
}
bool BlockProcessor::block2(bool placeholder){
    std::cout << "Processing block size 2\n";
}

class Info{
    public:
        int tupSize;
        int blockSize;
        int dims;

        Info(int tupSize, int blockSize, int dims){
            this->tupSize = tupSize;
            this->blockSize = blockSize;
            this->dims = dims;
        }
        int getTupSize(){
            return this->tupSize;
        }
        int getBlockSize(){
            return this->blockSize;
        }
        int getDims(){
            return this->dims;
        }
};
class Tools{
    public:
        typedef bool (Tools::*Intersect)(char*, char*);
        Info * info;
        Intersect intersect;
        Tools(Info * info){
            this->info = info;
            if(info->getBlockSize() == 32){
                intersect = &Tools::isIntersected32;
            }else if(info->getBlockSize() == 8){
                intersect = &Tools::isIntersected8;
            }
            
        }
        virtual void procZAddr(char* data, char*& za) = 0;

        bool isIntersected32(char* za1, char* za2){
            printf("Intersected 32\n");
            return true;
        }
        bool isIntersected8(char* za1, char* za2){
            printf("Intersected 8\n");
            return true;
        }
};
template <typename T>
class ZAddrUtils : public Tools{
    public:
        ZAddrUtils(Info* info) : Tools(info){
        }
        void procZAddr(char* data, char*& za){
            //print data as dim number of T elements
            printf("Processing as size %d\n", sizeof(T));
            for(int i = 0; i < info->dims; i++){
                printf("%x ", ((T*)data)[i]);
            }
        }
};

int main() {
    // Create an instance of BlockProcessor
    BlockProcessor processor1(1); // Block size 1
    processor1.processBlock();

    BlockProcessor processor4(4); // Block size 4
    processor4.processBlock();

    BlockProcessor processor10(10); // Unsupported block size
    processor10.processBlock();


    Info * info = new Info(2, 4, 2);
    ZAddrUtils<char> zAddrUtils(info);

    Tools* tools = &zAddrUtils;
    char * zadr;
    tools->procZAddr((char*)new uint[2]{0x1234, 0x7894}, zadr);


    return 0;
}
/*
#include <iostream>
#include <iomanip> // for std::hex

int main() {
    uint32_t num = 0x12345678;
    char* ptr = reinterpret_cast<char*>(&num);
    
    // Print each byte using char* pointer
    for (int i = 0; i < sizeof(uint32_t); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(ptr[i]);
    }
    std::cout << std::endl;

    return 0;
}

*/