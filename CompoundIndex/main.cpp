#include <stdio.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstring>

#include "cRowHeapTable.h"

#define TKey int
#define TData int

using namespace std;
using namespace std::chrono;

float GetThroughput(int opsCount, float period, int unit = 1000000);
void RowHeapTableTest(const int rowCount);
void RowHeapTableBenchmark(const int rowCount);
void RowHeapCollection(char * datacol);
void HashIndexBenchmark(const int rowCount, int nodeSize = 2048);
void HashIndexCollection(char * datacol, int *attr_indices, int attr_count);


int main()
{
    const int RowCount = 1e6;
    
    //RowHeapTableTest(10);
    //printf("\n");

    //RowHeapTableBenchmark(RowCount);
    HashIndexBenchmark(RowCount, 2048);

    /*
    RowHeapCollection("../datacol1");
    RowHeapCollection("../datacol2");
    RowHeapCollection("../datacol3");
    */
    printf("\n\nTesting collection 1\n");
    HashIndexCollection("../datacol1", new int[3]{2,3,4}, 3);
    printf("\n\nTesting collection 2\n");
    HashIndexCollection("../datacol2", new int[3]{4,5,6}, 3);
    return 0;
}

float GetThroughput(int opsCount, float period, int unit)
{
    return ((float)opsCount / unit) / period;
}
float BytesToMB(int bytes){
    return (float)bytes / 1024 / 1024;
}
void RowHeapTableTest(const int rowCount)
{
    /*
    fname CHAR(10),
    lanem CHAR(10),
    gender BYTE,
    ranking BYTE,
    department BYTE
    */
    int * schemaSizes = new int[5] {10,10,1,1,1};
    int * schemaCardinalities = new int[5] {-1,-1,2,3,5};
    cSchema * schema = new cSchema(5, schemaSizes, schemaCardinalities);
    cRowHeapTable *rowHeapTable = new cRowHeapTable(schema, rowCount);

    char fNames[20][10] = { "John", "Mary", "Robert", "Patricia", "Michael", "Jennifer", "William", "Linda", "David", "Elizabeth", "Richard", "Barbara", "Joseph", "Susan", "Thomas", "Jessica", "Charles", "Sarah", "Geralt", "Karen" };
    char lNames[20][10] = { "Smith", "Johnson", "Williams", "Brown", "Jones", "Miller", "Davis", "Garcia", "Rodriguez", "Wilson", "Martinez", "Anderson", "Taylor", "Thomas", "Hernandez", "Moore", "Martin", "Jackson", "Thompson", "White" };

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i<rowCount; i++){
        char row[23] = { 0 };
        int fNameIndex = rand() % 20;
        int lNameIndex = rand() % 20;
        memcpy(row, fNames[fNameIndex], 10);
        memcpy(row + 10, lNames[lNameIndex], 10);
        row[20] = rand() % 2 == 0 ? 0 : 1;
        row[21] = rand() % 3  ;
        row[22] = rand() % 5  ;
        rowHeapTable->Insert(row);
    }    
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HeapTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    if (rowCount <= 100){
        for (int i = 0; i < rowCount; i++)
        {
            rowHeapTable->PrintRow(i);
        }
    }
    int * query = new int[5] {-1, -1, 0, 1, -1};

    rowHeapTable->CreateBitmapIndex();

    rowHeapTable->Select(query, 5, true);
    rowHeapTable->SelectBitmap(query, 5, true);
    //rowHeapTable->SelectMask(query, 2, false);
    delete query;
    delete rowHeapTable;
}
void RowHeapTableBenchmark(const int rowCount)
{
    /*
    fname CHAR(10),
    lanem CHAR(10),
    gender BYTE,
    ranking BYTE,
    department BYTE
    */
    int * schemaSizes = new int[5] {10,10,1,1,1};
    int * schemaCardinalities = new int[5] {-1,-1,2,3,5};
    cSchema * schema = new cSchema(5, schemaSizes, schemaCardinalities);

    cRowHeapTable *rowHeapTable = new cRowHeapTable(schema, rowCount);

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i<rowCount; i++){
        char * row = new char[rowHeapTable->getRowSize()];
        int fNameIndex = rand() % 20;
        int lNameIndex = rand() % 20;
        memcpy(row, "fname", 5);
        memcpy(row + 10, "lname", 5);
        row[20] = i % 2 ;
        row[21] = i % 3 ;
        row[22] = i % 5 ;
        rowHeapTable->Insert(row);
        delete []row;
    }    
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HeapRowTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    t1 = high_resolution_clock::now();
    int attr_len[] = {-1,-1,2,3,5};
    rowHeapTable->CreateBitmapIndex();
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("bitMapIndex creation done. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    int queryCnt = 20;
    int (*query)[5] = new int[queryCnt][5];

    for(int i = 0; i < queryCnt; i++){
        query[i][0] = -1;
        query[i][1] = -1;
        query[i][2] =  i % 2;
        query[i][3] =  i % 3;
        query[i][4] =  i % 5;
    }
    int * results1 = new int[queryCnt];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        results1[i] = rowHeapTable->SelectBitmap(query[i], 5);
    }
    //printf("\n");
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select Bitmap done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
    printf("Bitmap size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getBitmapBytes()));

    int * results2 = new int[queryCnt];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        results2[i] = rowHeapTable->Select(query[i], 5);
    }
    //printf("\n");
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select rowHeapTable done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
    printf("Heap table size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getHeapBytes()));
/*
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        rowHeapTable->Select(query[i], 3);
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select rowHeapTable done. Time: %.6fs, Throughput: %.2f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
*/
    //print results1 and results2 in tuples (result1[i], result2[i]]), (...), ...
    
    printf("Results: ");
    for(int i = 0; i < queryCnt; i++){
        printf("(%d, %d), ", results1[i], results2[i]);
        if(i % 10 == 0 && i != 0)
            printf("\n");
    }
    delete []query;
    delete []results1;
    delete []results2;
    delete rowHeapTable;
}
void HashIndexBenchmark(const int rowCount, int nodeSize)
{
    /*
    fname CHAR(10),
    lanem CHAR(10),
    gender BYTE,
    ranking BYTE,
    department BYTE
    */
    int * schemaSizes = new int[5] {10,10,1,1,1};
    int * schemaCardinalities = new int[5] {-1,-1,2,3,5};
    cSchema * schema = new cSchema(5, schemaSizes, schemaCardinalities);

    cRowHeapTable *rowHeapTable = new cRowHeapTable(schema, rowCount);

    auto t1 = high_resolution_clock::now();
    for(int i = 0; i<rowCount; i++){
        char * row = new char[rowHeapTable->getRowSize()];
        int fNameIndex = rand() % 20;
        int lNameIndex = rand() % 20;
        memcpy(row + 0, "fname", 5);
        memcpy(row + 10, "lname", 5);
        row[20] = random() % 2 ;
        row[21] = random() % 3 ;
        row[22] = random() % 5 ;
        rowHeapTable->Insert(row);
        delete []row;
    }    
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HeapRowTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    for(int i = 0; i < 10; i++){
        rowHeapTable->PrintRow(i);
    }

    t1 = high_resolution_clock::now();
    rowHeapTable->CreateBitmapIndex();
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("bitMapIndex creation done. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));

    
    t1 = high_resolution_clock::now();
    int attrs_idx[] = {2,3,4};
    rowHeapTable->CreateHashIndex(attrs_idx, 3, nodeSize);
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("hashIndex creation done. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));
    int queryCnt = 20;
    int (*query)[7] = new int[queryCnt][7];

    for(int i = 0; i < queryCnt; i++){
        query[i][0] = -1;
        query[i][1] = -1;
        query[i][2] =  i % 2;
        query[i][3] =  i % 3;
        query[i][4] =  i % 5;
    }
    int * results1 = new int[queryCnt];
    double * times1 = new double[queryCnt];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        auto t3 = high_resolution_clock::now();
        results1[i] = rowHeapTable->Select(query[i], schema->GetAttrCount());
        auto t4 = high_resolution_clock::now();
        duration<double> subtime_span = duration_cast<duration<double>>(t4 - t3);
        times1[i] = subtime_span.count();
    }
    //printf("\n");
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select rowHeapTable done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
    printf("Heap table size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getHeapBytes()));

    int * results2 = new int[queryCnt];
    double * times2 = new double[queryCnt];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        auto t3 = high_resolution_clock::now();
        results2[i] = rowHeapTable->SelectBitmap(query[i], schema->GetAttrCount());
        auto t4 = high_resolution_clock::now();
        duration<double> subtime_span = duration_cast<duration<double>>(t4 - t3);
        times2[i] = subtime_span.count();
    }
    //printf("\n");
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select Bitmap done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
    printf("Bitmap size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getBitmapBytes()));

    int * results3 = new int[queryCnt];
    double * times3 = new double[queryCnt];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        auto t3 = high_resolution_clock::now();
        results3[i] = rowHeapTable->SelectHash(query[i], schema->GetAttrCount());
        auto t4 = high_resolution_clock::now();
        duration<double> subtime_span = duration_cast<duration<double>>(t4 - t3);
        times3[i] = subtime_span.count();
    }
    //printf("\n");
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select hashIndex done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
    printf("Hash table size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getHashBytes()));
    
    int * results4 = new int[queryCnt];
    double * times4 = new double[queryCnt];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryCnt; i++){
        auto t3 = high_resolution_clock::now();
        results4[i] = rowHeapTable->SelectHashStatistics(query[i], schema->GetAttrCount());
        auto t4 = high_resolution_clock::now();
        duration<double> subtime_span = duration_cast<duration<double>>(t4 - t3);
        times4[i] = subtime_span.count();
    }
    //printf("\n");
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select hashIndexStatistic done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryCnt, time_span.count(), 1));
    printf("Hash stat table size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getHashStatBytes()));

    printf("Results: \n");
    for(int i = 0; i < queryCnt; i++){
        for(int j = 0; j < 7; j++){
            printf("%d, ", query[i][j]);
        }
        printf(": (%d, %.6fs), (%d, %.6fs), (%d, %.6fs), (%d, %.6fs);", results1[i], times1[i], results2[i], times2[i], results3[i], times3[i],results4[i], times4[i]);
            printf("\n");
    }
    
    delete []query;
    delete []results1;
    delete []results2;
    delete []results3;
    delete []results4;
    delete []times1;
    delete []times2;
    delete []times3;
    delete []times4;
    delete rowHeapTable;
}
void RowHeapCollection(char* datacol) {
    // Combine the folder path with the filenames
    auto t1= high_resolution_clock::now();
    auto t2= high_resolution_clock::now();
    duration<double> time_span;

    char schemaFile[128];
    char dataFile[128];
    char queryFile[128];
    char resultsFile[128];
    strcpy(schemaFile, datacol);
    strcpy(dataFile, datacol);
    strcpy(queryFile, datacol);
    strcpy(resultsFile, datacol);
    strcat(schemaFile, "/schema.txt");
    strcat(dataFile, "/data.csv");
    strcat(queryFile, "/query.csv");
    strcat(resultsFile, "/result.csv");

    // Create an instance of the cSchema class and parse the schema
    cSchema * schema = new cSchema(); // Allocate memory for the schema pointer
    schema->ParseFile(schemaFile);
    schema->PrintSchema();

    // Open the data file for reading
    std::ifstream dataStream(dataFile);
    if (!dataStream.is_open()) {
        std::cerr << "Failed to open data.csv for reading." << std::endl;
        return;
    }

    // Parse the RowCount from the data file
    int rowCount = 0;
    char line[512]; // Adjust the buffer size as needed

    while (dataStream.getline(line, sizeof(line))) {
        if (std::strncmp(line, "RowCount:", 9) == 0) {
            rowCount = std::atoi(line + 9);
            break;
        }
    }

    cRowHeapTable *rowHeapTable = new cRowHeapTable(schema, rowCount);

    // Create an array of char arrays (rows)
    int rowSize = rowHeapTable->getRowSize();

    // Create a buffer for parsing values
    char* valueBuffer = new char[rowSize];
    int offset = 0;

    t1 = high_resolution_clock::now();  
    // Fill the rows from the dataset
    for (int i = 0; i < rowCount; ++i) {
        char* row = new char[rowSize];
        if (dataStream.getline(line, sizeof(line))) {
            offset = 0;
            std::stringstream ss(line);
            std::string value;
            int j = 0;
            while (std::getline(ss, value, ';')) {
                int attrSize = schema->GetAttrSize()[j];
                if (attrSize == 1) {
                    int intValue = std::stoi(value);
                    memcpy(row + offset, &intValue, 1);
                } else {
                    memcpy(row + offset, value.c_str(), attrSize);
                }
                offset += attrSize;
                j++;
            }
        }
        rowHeapTable->Insert(row);
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HeapTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));
    printf("Heap table size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getHeapBytes()));

    // Close the data file
    dataStream.close();
    //print header
    for(int i = 0; i < 10; i++){
        rowHeapTable->PrintRow(i);
    }

    // Create a bitmap index for the specified attributes
    t1 = high_resolution_clock::now();
    rowHeapTable->CreateBitmapIndex();
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("bitMapIndex creation done. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));
    printf("Bitmap size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getBitmapBytes()));

    // Process query.csv or perform other operations as needed
    int queryNum = 0;
    int ** queries;
    FILE* file = fopen(queryFile, "r");
    if (file != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            char* key = strtok(line, ":");
            char* value = strtok(NULL, ":");
            if (key != NULL) {
                if (strcmp(key, "QueryCount") == 0  ||strcmp(key, "Query Count") == 0) {
                    sscanf(value, "%d", &queryNum);
                    queries = new int*[queryNum];
                    break;         
                }
            }
        }               
        int p = 0;
        while (fgets(line, sizeof(line), file)) {
            char* token = strtok(line, ";");
            int i = 0;
            int* query = new int[rowHeapTable->getSchema()->GetAttrCount()];
            while (token != NULL) {
                sscanf(token, "%d", &query[i]);
                token = strtok(NULL, ";");
                i++;
            }
            queries[p] = query;
            p++;
        }    
        fclose(file);
    }
/*
*/
    int * results1 = new int[queryNum];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryNum; i++){
        results1[i]=rowHeapTable->Select(queries[i], rowHeapTable->getSchema()->GetAttrCount());
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select rowHeapTable done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryNum, time_span.count(), 1));

    int * results2 = new int[queryNum];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryNum; i++){
        results2[i]= rowHeapTable->SelectBitmap(queries[i], rowHeapTable->getSchema()->GetAttrCount());
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select Bitmap done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryNum, time_span.count(), 1));

    int * resultsFromFile = new int[queryNum];
    file = fopen(resultsFile, "r");
    if (file != NULL) {
        char line[256];
        int i = 0;
        while (fgets(line, sizeof(line), file)) {
            sscanf(line, "%d", &resultsFromFile[i]);
            i++;
        }
        fclose(file);
    }
    //print results1 and results2 in tuples (result1[i], result2[i]]), (...), ...
    /*
    printf("Results:");
    for(int i = 0; i < queryNum; i++){
        if(i % 10 == 0)
            printf("\n");
        printf("(%d, %d, %d), ", results1[i], results2[i], resultsFile[i]);
    }
    */
    bool success = true;
    for(int i = 0; i < queryNum; i++){
        if(results1[i] != resultsFromFile[i] || results2[i] != resultsFromFile[i]){
            success = false;
            printf("Error: result ix:%d, values(%d, %d, %d)\n", i, results1[i], results2[i], resultsFromFile[i]);
        }
    }
    if(success){
        printf("Results are correct.\n");
    }
    // Create result_rt.csv in the same folder and write results1 row by row into it
    // Do the same with result_bi.csv and results2
    file = fopen("../result_rt.csv", "w");
    if (file != NULL) {
        for(int i = 0; i < queryNum; i++){
            fprintf(file, "%d\n", results1[i]);
        }
        fclose(file);
    }
    file = fopen("../result_bi.csv", "w");
    if (file != NULL) {
        for(int i = 0; i < queryNum; i++){
            fprintf(file, "%d\n", results2[i]);
        }
        fclose(file);
    }
    

    // Clean up allocated memory
    delete[] valueBuffer;
    delete rowHeapTable;
    delete schema;
    for(int i = 0; i < queryNum; i++){
        delete queries[i];
    }
    delete[] queries;
    delete[] results1;
    delete[] results2;
    delete[] resultsFromFile;
}
void HashIndexCollection(char * datacol, int* attr_indices, int attr_count){
     // Combine the folder path with the filenames
    auto t1= high_resolution_clock::now();
    auto t2= high_resolution_clock::now();
    duration<double> time_span;

    char schemaFile[128];
    char dataFile[128];
    char queryFile[128];
    char resultsFile[128];
    strcpy(schemaFile, datacol);
    strcpy(dataFile, datacol);
    strcpy(queryFile, datacol);
    strcpy(resultsFile, datacol);
    strcat(schemaFile, "/schema.txt");
    strcat(dataFile, "/data.csv");
    strcat(queryFile, "/query.csv");
    strcat(resultsFile, "/result.csv");

    // Create an instance of the cSchema class and parse the schema
    cSchema * schema = new cSchema(); // Allocate memory for the schema pointer
    schema->ParseFile(schemaFile);
    schema->PrintSchema();

    // Open the data file for reading
    std::ifstream dataStream(dataFile);
    if (!dataStream.is_open()) {
        std::cerr << "Failed to open data.csv for reading." << std::endl;
        return;
    }

    // Parse the RowCount from the data file
    int rowCount = 0;
    char line[512]; // Adjust the buffer size as needed

    while (dataStream.getline(line, sizeof(line))) {
        if (std::strncmp(line, "RowCount:", 9) == 0) {
            rowCount = std::atoi(line + 9);
            break;
        }
    }

    cRowHeapTable *rowHeapTable = new cRowHeapTable(schema, rowCount);

    // Create an array of char arrays (rows)
    int rowSize = rowHeapTable->getRowSize();

    // Create a buffer for parsing values
    char* valueBuffer = new char[rowSize];
    int offset = 0;

    t1 = high_resolution_clock::now();  
    // Fill the rows from the dataset
    for (int i = 0; i < rowCount; ++i) {
        char* row = new char[rowSize];
        if (dataStream.getline(line, sizeof(line))) {
            offset = 0;
            std::stringstream ss(line);
            std::string value;
            int j = 0;
            while (std::getline(ss, value, ';')) {
                int attrSize = schema->GetAttrSize()[j];
                if (attrSize == 1) {
                    int intValue = std::stoi(value);
                    memcpy(row + offset, &intValue, 1);
                } else {
                    memcpy(row + offset, value.c_str(), attrSize);
                }
                offset += attrSize;
                j++;
            }
        }
        rowHeapTable->Insert(row);
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Records insertion done, HeapTable. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));
    printf("Heap table size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getHeapBytes()));

    // Close the data file
    dataStream.close();
    //print header
    for(int i = 0; i < 10; i++){
        rowHeapTable->PrintRow(i);
    }

    // Create a bitmap index for the specified attributes
    t1 = high_resolution_clock::now();
    rowHeapTable->CreateBitmapIndex();
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("bitMapIndex creation done. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));
    printf("Bitmap size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getBitmapBytes()));

    // Create a hash index for the specified attributes
    t1 = high_resolution_clock::now();
    rowHeapTable->CreateHashIndex(attr_indices, attr_count);
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("HashIndex creation done. Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(rowCount, time_span.count()));
    printf("Hash index size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getHashBytes()));
    printf("Hash index statistics size: %.6f Megabytes.\n", BytesToMB(rowHeapTable->getHashStatBytes()));
    // Process query.csv or perform other operations as needed
    int queryNum = 0;
    int ** queries;
    FILE* file = fopen(queryFile, "r");
    if (file != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            char* key = strtok(line, ":");
            char* value = strtok(NULL, ":");
            if (key != NULL) {
                if (strcmp(key, "QueryCount") == 0  ||strcmp(key, "Query Count") == 0) {
                    sscanf(value, "%d", &queryNum);
                    queries = new int*[queryNum];
                    break;         
                }
            }
        }               
        int p = 0;
        while (fgets(line, sizeof(line), file)) {
            char* token = strtok(line, ";");
            int i = 0;
            int* query = new int[rowHeapTable->getSchema()->GetAttrCount()];
            while (token != NULL) {
                sscanf(token, "%d", &query[i]);
                token = strtok(NULL, ";");
                i++;
            }
            queries[p] = query;
            p++;
        }    
        fclose(file);
    }

    
    int * results1 = new int[queryNum];/*
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryNum; i++){
        results1[i]=rowHeapTable->Select(queries[i], rowHeapTable->getSchema()->GetAttrCount());
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select rowHeapTable done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryNum, time_span.count(), 1));
    */

    int * results2 = new int[queryNum];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryNum; i++){
        results2[i]= rowHeapTable->SelectBitmap(queries[i], rowHeapTable->getSchema()->GetAttrCount());
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select Bitmap done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryNum, time_span.count(), 1));

    int * results3 = new int[queryNum];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryNum; i++){
       results3[i] = rowHeapTable->SelectHash(queries[i], rowHeapTable->getSchema()->GetAttrCount());
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select Hash done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryNum, time_span.count(), 1));

    int * results4 = new int[queryNum];
    t1 = high_resolution_clock::now();
    for(int i = 0; i < queryNum; i++){
       results4[i] = rowHeapTable->SelectHashStatistics(queries[i], rowHeapTable->getSchema()->GetAttrCount());
    }
    t2 = high_resolution_clock::now();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select Hash Stat done. Time: %.6fs, Throughput: %.6f op/s.\n", time_span.count(), GetThroughput(queryNum, time_span.count(), 1));


    int * resultsFromFile = new int[queryNum];
    file = fopen(resultsFile, "r");
    if (file != NULL) {
        char line[256];
        int i = 0;
        while (fgets(line, sizeof(line), file)) {
            sscanf(line, "%d", &resultsFromFile[i]);
            i++;
        }
        fclose(file);
    }
    //print results1 and results2 in tuples (result1[i], result2[i]]), (...), ...
    /*
    printf("Results:");
    for(int i = 0; i < queryNum; i++){
        if(i % 10 == 0)
            printf("\n");
        printf("(%d, %d, %d), ", results1[i], results2[i], resultsFile[i]);
    }
    */
    bool success = true;
    for(int i = 0; i < queryNum; i++){
        if(/*results1[i] != resultsFromFile[i] || results2[i] != resultsFromFile[i] ||*/ results3[i] != resultsFromFile[i] || results4[i] != resultsFromFile[i]){
            success = false;
            printf("Error: result ix:%d, query:%d %d %d %d %d, values(%d, %d, %d, %d, %d)\n", i, queries[i][0], queries[i][1], queries[i][2], queries[i][3], queries[i][4], results1[i], results2[i], results3[i], results4[i], resultsFromFile[i]);
        }
    }
    if(success){
        printf("Results are correct.\n");
    }
    // Clean up allocated memory
    delete[] valueBuffer;
    delete rowHeapTable;
    delete schema;
    for(int i = 0; i < queryNum; i++){
        delete queries[i];
    }
    delete[] queries;
    delete[] results1;
    delete[] results2;
    delete[] results3;
    delete[] results4;
    delete[] resultsFromFile;
}