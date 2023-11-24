#include <iostream>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace std::chrono;

int main() {
    const int GB = 1024 * 1024 * 1024; // 1GB in bytes
    const int numLongs = GB / sizeof(long);
    
    // Allocate dynamic memory for 1GB of longs
    long* data = new long[numLongs];
    
    // Fill the array with numbers corresponding to the index
    for (int i = 0; i < numLongs; i++) {
        data[i] = i;
    }

    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    const int numOperations = 1000000; // Adjust this as needed

    // Sequential Write Benchmark
    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < numOperations; i++) {
        data[i % numLongs] = i; // Write sequentially
    }
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "Sequential Write: " << numOperations << " operations in " << time_span.count() << " seconds." << endl;

    // Sequential Read Benchmark
    t1 = high_resolution_clock::now();
    long sum = 0;
    for (int i = 0; i < numOperations; i++) {
        sum += data[i % numLongs]; // Read sequentially
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "Sequential Read: " << numOperations << " operations in " << time_span.count() << " seconds." << endl;

    // Random Write Benchmark
    t1 = high_resolution_clock::now();
    for (int i = 0; i < numOperations; i++) {
        int randomIndex = rand() % numLongs;
        data[randomIndex] = i; // Write randomly
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "Random Write: " << numOperations << " operations in " << time_span.count() << " seconds." << endl;

    // Random Read Benchmark
    t1 = high_resolution_clock::now();
    for (int i = 0; i < numOperations; i++) {
        int randomIndex = rand() % numLongs;
        sum += data[randomIndex]; // Read randomly
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    cout << "Random Read: " << numOperations << " operations in " << time_span.count() << " seconds." << endl;

    // Clean up
    delete[] data;

    return 0;
}
