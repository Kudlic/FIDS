#include <iostream>
class DataHolder {
public:
DataHolder() {
mData = new int[10];
}
// A destructor should be defined to release the allocated memory
~DataHolder() {
delete[] mData;
}
void processData() {
// Processing the data
}
private:
int* mData;
};

int main() {
DataHolder* holder = new DataHolder();
holder->processData();
// We forgot to delete the 'holder' instance
// delete holder;
return 0; // Memory leak occurs here as 'holder' is not deleted
}