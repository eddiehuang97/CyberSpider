#include "DiskMultiMap.h"
#include <string>
#include <functional>
#include <iostream>
using namespace std;

DiskMultiMap::DiskMultiMap() {

}

DiskMultiMap::~DiskMultiMap() {
	close();
}
bool DiskMultiMap::createNew(const std::string& filename, unsigned int numBuckets) {
	if (!bf.createNew(filename))
		return false;
	buckets = numBuckets;
	BinaryFile::Offset zero = 0;
	if (!bf.write(buckets, 0))
		return false;
	for (int i = 0; i < buckets; i++)
		if (!bf.write(zero, bf.fileLength()))
			return false;
	return true;
}

bool DiskMultiMap::openExisting(const std::string& filename) {
	if (!bf.openExisting(filename))
		return false;
	if (!bf.read(buckets, 0))
		return false;
	return true;
}

void DiskMultiMap::close() {
	bf.close();
}

bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context) {
	if (key.length() > 120 || value.length() > 120 || context.length() > 120)
		return false;
	hash<string> str_hash;
	BinaryFile::Offset whichBucket = (str_hash(key) % buckets) * sizeof(BinaryFile::Offset) + sizeof(int);
	BinaryFile::Offset nextNode;
	if (!bf.read(nextNode, whichBucket))
		return false;
	DiskNode newNode;
	strcpy_s(newNode.key, key.c_str());
	strcpy_s(newNode.value, value.c_str());
	strcpy_s(newNode.context, context.c_str());
	newNode.next = nextNode;
	newNode.active = true;
	bool isActive;
	BinaryFile::Offset it = sizeof(int) + sizeof(int) * buckets;
	while (it < bf.fileLength() - 363 * sizeof(char) - sizeof(BinaryFile::Offset) && bf.read(isActive, it + 363 * sizeof(char) + sizeof(BinaryFile::Offset))) {
		if (!isActive) {
			bf.write(it, whichBucket);
			bf.write(newNode.key, it);
			bf.write(newNode.value, it + 121 * sizeof(char));
			bf.write(newNode.context, it + 242 * sizeof(char));
			bf.write(newNode.next, it + 363 * sizeof(char));
			bf.write(newNode.active, it + 363 * sizeof(char) + sizeof(BinaryFile::Offset));
			return true;
		}
		it += sizeof(DiskNode);
	}
	bf.write(bf.fileLength(), whichBucket);
	bf.write(newNode.key, bf.fileLength());
	bf.write(newNode.value, bf.fileLength());
	bf.write(newNode.context, bf.fileLength());
	bf.write(newNode.next, bf.fileLength());
	bf.write(newNode.active, bf.fileLength());
	return true;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {
	DiskMultiMap::Iterator test;
	return test;

}

int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
	hash<string> str_hash;
	BinaryFile::Offset it = (str_hash(key) % buckets) * sizeof(BinaryFile::Offset) + sizeof(int);
	BinaryFile::Offset whichBucket = it;
	int nodesRemoved = 0;
	bf.read(it, it);
	char* checkKey = new char[121];
	char* checkValue = new char[121];
	char* checkContext = new char[121];
	if (it == 0 || !bf.read(checkKey, 121, it) || !bf.read(checkValue, 121, it + 121 * sizeof(char))
		|| !bf.read(checkContext, 121, it + 242 * sizeof(char)))
		return nodesRemoved;
	while (strcmp(checkKey, key.c_str()) == 0 && strcmp(checkValue, value.c_str()) == 0 && strcmp(checkContext, context.c_str()) == 0) {
		nodesRemoved++;
		bf.write(false, it + 363 * sizeof(char) + sizeof(BinaryFile::Offset));
		BinaryFile::Offset storeIt;
		bf.read(storeIt, it + 363 * sizeof(char));
		bf.write(storeIt, whichBucket);
		it = storeIt;
		if (it == 0)
			return nodesRemoved;
		bf.read(checkKey, 121, it);
		bf.read(checkValue, 121, it + 121 * sizeof(char));
		bf.read(checkContext, 121, it + 242 * sizeof(char));
	}
	while (it != 0) {
		BinaryFile::Offset storeIt;
		bf.read(storeIt, it + 363 * sizeof(char));
		if (storeIt == 0) {
			return nodesRemoved;
		}
		bf.read(checkKey, 121, storeIt);
		bf.read(checkValue, 121, storeIt + 121 * sizeof(char));
		bf.read(checkContext, 121, storeIt + 242 * sizeof(char));
		if (strcmp(checkKey, key.c_str()) == 0 && strcmp(checkValue, value.c_str()) == 0 && strcmp(checkContext, context.c_str()) == 0) {
			nodesRemoved++;
			bf.write(false, storeIt + 363 * sizeof(char) + sizeof(BinaryFile::Offset));
			bf.read(storeIt, storeIt + 363 * sizeof(char));
			bf.write(storeIt, it + 363 * sizeof(char));
		}
		else {
			bf.read(it, it + 363 * sizeof(char));
		}
	}
	return nodesRemoved;
}

DiskMultiMap::Iterator::Iterator() {}
// You may add additional constructors
bool DiskMultiMap::Iterator::isValid() const {
	return false;
}
DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {
	Iterator test;
	return test;
}
MultiMapTuple DiskMultiMap::Iterator::operator*() {
	MultiMapTuple test;
	return test;
}
