// fidbs3.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include <iostream>
#include <fstream>
#include "cRowHeapTable.h"
#include <chrono>
#include <tuple>
#include "HashIndex.h"
#include "cMemory.h"
#include "cResults.h"

using namespace std;
using namespace std::chrono;


float GetThroughput(int opsCount, float period, int unit = 10e5);
int ReadSchema(int rowCount, char* a, char * b);
void AddRecords(char* source, int* target, int size);
void AddSelect(char* source, int* target, int size);
cRowHeapTable* Insert(int* atrs_size, int len, int* attr_max_value, int atr_count, int bytes);
void InsertIndex(int* atrs_size, int len, int* attr_max_value, int atr_count, int bytes, int index, cRowHeapTable* table);
void Select(cRowHeapTable* table, int atr_count);
void SelectHashIndex(int* atrs_size, int len, int* attr_max_value, int atr_count, cRowHeapTable* table);
bool CheckResults(string file, int length);

static string SCHEMA;
static string DATA;
static string QUERY;
static string RESULT = "2result.csv";

static string MYRESULT = "result_rt.csv";
static string MYRESULTINDEX = "result_bi.csv";

int main(int argc, char*argv[])
{
    SCHEMA = argv[1];
    DATA = argv[2];
    QUERY = argv[3];
    
    int len=0;
    int atr_count = 0;
    int bytes = 0;
    char* attrsize = new char[1024];
    char* attrvalc = new char[1024];

    

    atr_count=ReadSchema(atr_count, attrsize, attrvalc);
    int* atrs_size = new int[atr_count];
    int* attr_max_value = new int[atr_count];

    AddRecords(attrsize, atrs_size, atr_count);
    AddRecords(attrvalc, attr_max_value, atr_count);
    delete attrsize;
    delete attrvalc;
    
    for (int i = 0; i < atr_count; i++) {
        len += atrs_size[i];
    }
    
    for (int i = 0; i < atr_count; i++) {
        int a = attr_max_value[i];
        if (attr_max_value[i] != -1) {
            bytes += attr_max_value[i];
        }
    }

    bytes = ceil((double)bytes / 8);
    
    // without index
    cout << "heap table insert:" << endl;
    cRowHeapTable* table = Insert(atrs_size, len, attr_max_value, atr_count, bytes);
    cout << endl;
    // with index
    cout << "create bitmap index:" << endl;
    InsertIndex(atrs_size, len, attr_max_value, atr_count, bytes, 1, table);
    cout << endl;

    cout << "create hash index:" << endl;
    InsertIndex(atrs_size, len, attr_max_value, atr_count, bytes, 2, table);
    cout << endl;


    SelectHashIndex(atrs_size, len, attr_max_value, atr_count, table);
    Select(table, atr_count);
    delete table;
    

    
  
    
}

float GetThroughput(int opsCount, float period, int unit)
{
    return ((float)opsCount / unit) / period;
}


int ReadSchema(int atr_count, char* a, char * b) {
    ifstream out;
    out.open(SCHEMA);
    const int MAX_LEN = 1024;
    char* str = new char[MAX_LEN];

    out.getline(str, MAX_LEN);
    sscanf_s(str, "AttrCount:%d", &atr_count);

    out.getline(a, MAX_LEN);
    out.getline(b, MAX_LEN);


    out.close();
    delete str;
    return atr_count;
}

void AddRecords(char* source, int* target, int size) {
    string s;
    string val;
    s.assign(source, 1024);
    
    s.erase(0, s.find(":") +1);

    for (int i = 0; i < size; i++) {
        val = s.substr(0, s.find(","));
        s.erase(0, s.find(",") + 1);
        target[i] = atoi(val.c_str());
    }
}

void AddSelect(char* source, int* target, int size) {
    string s;
    string val;
    s.assign(source, 1024);

    for (int i = 0; i < size; i++) {
        val = s.substr(0, s.find(";"));
        s.erase(0, s.find(";") + 1);
        target[i] = atoi(val.c_str());
    }
}

cRowHeapTable* Insert(int* atrs_size, int len, int* attr_max_value, int atr_count, int bytes) {
    int length = 0;
    ifstream out;
    out.open(DATA);
    const int MAX_LEN = 1024;
    char* str = new char[MAX_LEN];

    out.getline(str, MAX_LEN);
    sscanf_s(str, "RowCount:%d", &length);
    cRowHeapTable* table = new cRowHeapTable(atrs_size, len, length, attr_max_value, atr_count, bytes);
    auto t1 = high_resolution_clock::now();
    TData id = 0;
    while (!out.eof() && id < length)
    {
        out.getline(str, MAX_LEN);
        char* rec = new char[len];
        cRowHeapTable::CreateRecord(rec, atrs_size, attr_max_value, atr_count, str);
        table->Insert(rec);
        id++;
        
    }
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(length, time_span.count()));
    out.close();
    delete str;
    return table;
}

void InsertIndex(int* atrs_size, int len, int* attr_max_value, int atr_count, int bytes, int index, cRowHeapTable* table) {
    int length = 0;
    ifstream out;
    out.open(DATA);
    const int MAX_LEN = 1024;
    char* str = new char[MAX_LEN];

    out.getline(str, MAX_LEN);
    sscanf_s(str, "RowCount:%d", &length);
    auto t1 = high_resolution_clock::now();
    TData id = 0;
    while (!out.eof() && id < length)
    {
        out.getline(str, MAX_LEN);
        char* rec = new char[len];
        cRowHeapTable::CreateRecord(rec, atrs_size, attr_max_value, atr_count, str);

        if (index == 1) {
            table->CreateBitmapIndex(rec);
        }
        else if (index == 2) {
            table->CreateHashIndex(rec, id);
        }       
        id++;

    }
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Time: %.2fs, Throughput: %.2f mil. op/s.\n", time_span.count(), GetThroughput(length, time_span.count()));
    out.close();
    delete str;
}

void SelectHashIndex(int* atrs_size, int len, int* attr_max_value, int atr_count, cRowHeapTable* table) {
    //cMemory* memory = new cMemory(300000000);
    int length = 0;
    ifstream out;
    const int MAX_LEN = 1024;
    char* str = new char[atr_count + atr_count - 1];
    out.open(QUERY);
    out.getline(str, MAX_LEN);
    sscanf_s(str, "Query Count:%d", &length);
    int result = 0;
    int i = 0;
    int throughput = 0;
    int bitmapcount = 0;
    cResults* resultIds = new cResults();
    auto t1 = high_resolution_clock::now();
    while (!out.eof() && i < length)
    {
        out.getline(str, MAX_LEN);

        int* select = new int[atr_count];
        AddSelect(str, select, atr_count);
        //char* rec = new char[len];
        //cRowHeapTable::CreateHashRecord(rec, select, attr_max_value, atr_count, str);

        int stat = table->SelectHashIndexStatistics(select);
        if (stat != -1) {
            if (stat <= 50) {
                
                int* ids = new int[stat];
                result = table->SelectHashIndex(ids, select);
                resultIds->Add(ids, stat);
                throughput++;
            }
            else {
                int* ids = new int[stat];
                table->SelectHashIndex(ids, select);
                throughput++;
            }
        }
        else {
            bitmapcount++;
            table->SelectIndexWithoutTuple(select, atr_count);
        }

        i++;
    }
    auto t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    cout << throughput << " selects with hash index" << endl;
    cout << bitmapcount << " selects with bitmap index" << endl;
    printf("Select with hash index:\nTime: %.2fs, Throughput: %.2f op/s.\n\n", time_span.count(), GetThroughput(throughput+bitmapcount, time_span.count(), 1));
    out.close();

    resultIds->Write(table);
}

void Select(cRowHeapTable* table, int atr_count) {
    int length = 0;
    ifstream out;
    ofstream in;
    out.open(QUERY);
    const int MAX_LEN = 1024;
    char* str = new char[atr_count+atr_count-1];
    int result = 0;
    out.getline(str, MAX_LEN);
    sscanf_s(str, "Query Count:%d", &length);

    int i = 0;
    in.open(MYRESULT);
    auto t1 = high_resolution_clock::now();
    while (!out.eof() && i<length)
    {
        out.getline(str, MAX_LEN);
        int* select = new int[atr_count];
        AddSelect(str, select, atr_count);
        result = table->SelectWithoutTuple(select, atr_count);
        in << result << "\n";
        i++;

    }
    auto t2 = high_resolution_clock::now();
    in.close();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select without index:\nTime: %.2fs, Throughput: %.2f op/s.\n\n", time_span.count(), GetThroughput(length, time_span.count(), 1));
    out.close();
    /*
    if (CheckResults(MYRESULT, length)) {
        cout << "results are same" << endl << endl;
    }
    else {
        cout << "results are not same" << endl << endl;
    }
    */

    out.open(QUERY);
    out.getline(str, MAX_LEN);
    in.open(MYRESULTINDEX);

    i = 0;
    t1 = high_resolution_clock::now();
    while (!out.eof() && i < length)
    {
        out.getline(str, MAX_LEN);
        int* select = new int[atr_count];
        AddSelect(str, select, atr_count);
        result = table->SelectIndexWithoutTuple(select, atr_count);
        in << result << "\n";
        i++;
    }
    t2 = high_resolution_clock::now();
    in.close();
    time_span = duration_cast<duration<double>>(t2 - t1);
    printf("Select with bitmap index:\nTime: %.2fs, Throughput: %.2f op/s.\n\n", time_span.count(), GetThroughput(length, time_span.count(), 1));

    out.close();


    /*
    if (CheckResults(MYRESULTINDEX, length)) {
        cout << "results are same" << endl << endl;
    }
    else {
        cout << "results are not same" << endl << endl;
    }
    */
    delete str;
}

bool CheckResults(string file, int length) {
    bool ret = true;
    ifstream out1, out2;
    out1.open(RESULT);
    out2.open(file);

    const int MAX_LEN = 1024;
    char* str1 = new char[MAX_LEN];
    char* str2 = new char[MAX_LEN];
    int i = 0;
    while (!out1.eof() && !out2.eof() && i < length)
    {
        out1.getline(str1, MAX_LEN);
        out2.getline(str2, MAX_LEN);
        string a = str1;
        string b = str2;
        if (a != b) {
            ret = false;
        }
        i++;
    }
    out1.close();
    out2.close();
    delete str1;
    delete str2;
    return ret;
}

