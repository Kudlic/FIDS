#pragma once
#include <random>
#include "cTuple.h"

template<typename T>
class cGaussianTupleGenerator {
private:
	int dimensions;
	int count;
	int seed;
	T maxValue;
	cTuple<T>* container;
	int position;

	T* data;
public:
	cGaussianTupleGenerator(int dimensions, int count, T maxVal = 100, int seed = 1704);
	~cGaussianTupleGenerator();
	void generateRandom();
	cTuple<T>* nextTuple(); //returns next tuple
	cTuple<T>* getTuple(int n); //returns tuple at position n
	bool hasNext(); //returns true if there are more tuples to return
	void reset();
};

template<typename T>
cGaussianTupleGenerator<T>::cGaussianTupleGenerator(int dimensions, int count, T maxVal, int seed) {
	this->dimensions = dimensions;
	this->count = count;
	this->maxValue = maxVal;
	this->seed = seed;
	this->container = new cTuple<T>(dimensions, true);
	this->position = 0;
	this->data = new T[dimensions * count];
}
template<typename T>
cGaussianTupleGenerator<T>::~cGaussianTupleGenerator() {
	delete container;
	delete[] data;
}
template<typename T>
void cGaussianTupleGenerator<T>::generateRandom() {
	std::default_random_engine generator(seed);
	std::normal_distribution<double> distribution(0.0, 1.0);
	for (int i = 0; i < dimensions * count; i++) {
		double val = distribution(generator);
		if(val < 0) val *= -1;
		while (val > 1) {
			val = val / 2;
		}
		data[i] = (T)(val * maxValue);
	}
}
template<typename T>
cTuple<T>* cGaussianTupleGenerator<T>::nextTuple() {
	if (position < count) {
		container->setTuple(&data[position * dimensions], dimensions);
		position++;
		return container;
	}
	return nullptr;
}
template<typename T>
cTuple<T>* cGaussianTupleGenerator<T>::getTuple(int n) {
	if (n < count) {
		container->setTuple(&data[n * dimensions], dimensions);	
		return container;
	}
	return nullptr;
}
template<typename T>
bool cGaussianTupleGenerator<T>::hasNext() {
	return position < count;
}
template<typename T>
void cGaussianTupleGenerator<T>::reset() {
	position = 0;
}