#pragma once
#include "cTuple.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

template<typename T>
class cDatasetTupleGenerator {
private:
	int dimensions;
	int count;
	cTuple<T>* container;
	int position;

	T* data;
public:
	cDatasetTupleGenerator(const std::string& filename);
	~cDatasetTupleGenerator();
	cTuple<T>* nextTuple(); //returns next tuple
	cTuple<T>* getTuple(int n); //returns tuple at position n
	bool hasNext(); //returns true if there are more tuples to return
	void reset();
	int getDimensions() { return dimensions; }
	int getCount() { return count; }
};

template<typename T>
cDatasetTupleGenerator<T>::cDatasetTupleGenerator(const std::string& filename) {
	this->count = 0;
	this->dimensions = 0;

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error opening file " << filename << "!" << std::endl;
		return;
	}

	std::string line;

	// Read the first line to extract dimension
	if (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string temp;
		std::getline(iss, temp, ':'); // Skip "Dimension:"
		iss >> this->dimensions;

		// Read the second line to extract tuples count
		std::getline(file, line);
		std::istringstream iss2(line);
		std::getline(iss2, temp, ':'); // Skip "Tuples Count:"
		iss2 >> this->count;

		this->container = new cTuple<T>(this->dimensions, true);
		this->position = 0;
		this->data = new T[this->dimensions * this->count];

		int index = 0;
		while (std::getline(file, line) && index < (this->dimensions*this->count)) {
			if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
				// Skip line if it contains only whitespace characters
				continue;
    		}
			std::istringstream iss3(line);
			std::string value;
			while (std::getline(iss3, value, ',')) {
				data[index++] = std::stoi(value);
			}
		}

	}
	else {
		std::cerr << "Error reading file " << filename << "!" << std::endl;
		return;
	}

	file.close();
}
template<typename T>
cDatasetTupleGenerator<T>::~cDatasetTupleGenerator() {
	delete container;
	delete[] data;
}

template<typename T>
cTuple<T>* cDatasetTupleGenerator<T>::nextTuple() {
	if (position < count) {
		container->setTuple(&data[position * dimensions], dimensions);
		position++;
		return container;
	}
	return nullptr;
}
template<typename T>
cTuple<T>* cDatasetTupleGenerator<T>::getTuple(int n) {
	if (n < count) {
		container->setTuple(&data[n * dimensions], dimensions);
		return container;
	}
	return nullptr;
}
template<typename T>
bool cDatasetTupleGenerator<T>::hasNext() {
	return position < count;
}
template<typename T>
void cDatasetTupleGenerator<T>::reset() {
	position = 0;
}