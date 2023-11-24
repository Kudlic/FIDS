#pragma once
#include <iostream>
#include <fstream>
using namespace std;

class cResults
{
private:
	bool mEmptyNode;
	cResults* mNextData;
	int* mData;
	int mSize;

public:
	cResults();
	~cResults();
	void Add(int*& data, int size);
	void Write(cRowHeapTable* table);
};

cResults::cResults() {
	
	mEmptyNode = true;
}

cResults::~cResults()
{
}

void cResults::Add(int*& data, int size) {
	cResults* node = this;
	do {
		if (node->mEmptyNode) {
			node->mData = new int[size];
			node->mSize = size;
			node->mData = data;
			node->mNextData = new cResults();
			node->mEmptyNode = false;
			return;
		}
		else {
			node = node->mNextData;
		}
	} while (node != nullptr);
}

void cResults::Write(cRowHeapTable* table) {
	ofstream in;
	in.open("result_select_star.csv");
	cResults* node = this;
	do {
		if (node->mEmptyNode) {
			return;
		}
		else {
			for (int i = 0; i < node->mSize; i++) {
				int id = node->mData[i];
				string row = table->GetRecord(id);
				in << row << "\n";				
			}
			node = node->mNextData;
		}
	} while (node != nullptr);
	in.close();
}

