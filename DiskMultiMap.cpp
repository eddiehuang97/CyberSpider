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
	if (!bf.write(zero, bf.fileLength())) // used to store pointer to LinkedList of deleted nodes
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
	BinaryFile::Offset whichBucket = (str_hash(key) % buckets) * sizeof(BinaryFile::Offset) + sizeof(int) + sizeof(BinaryFile::Offset);
	BinaryFile::Offset nextNode;
	if (!bf.read(nextNode, whichBucket))
		return false;
	DiskNode newNode;
	strcpy_s(newNode.key, key.c_str());
	strcpy_s(newNode.value, value.c_str());
	strcpy_s(newNode.context, context.c_str());
	newNode.next = nextNode;
	BinaryFile::Offset it = sizeof(int);
	bf.read(it, it);
	if(it != 0) {
		BinaryFile::Offset storeIt;
		bf.read(storeIt, it + sizeof(char[363]));
		bf.write(storeIt, sizeof(int));
		bf.write(it, whichBucket);
		bf.write(newNode.key, it);
		bf.write(newNode.value, it + sizeof(char[121]));
		bf.write(newNode.context, it + sizeof(char[242]));
		bf.write(newNode.next, it + sizeof(char[363]));
		return true;
	}
	bf.write(bf.fileLength(), whichBucket);
	bf.write(newNode.key, bf.fileLength());
	bf.write(newNode.value, bf.fileLength());
	bf.write(newNode.context, bf.fileLength());
	bf.write(newNode.next, bf.fileLength());
	return true;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key) {
	hash<string> str_hash;
	BinaryFile::Offset it = (str_hash(key) % buckets) * sizeof(BinaryFile::Offset) + sizeof(int) + sizeof(BinaryFile::Offset);
	bf.read(it, it);
	while (it != 0) {
		char* checkKey = new char[121];
		bf.read(checkKey, 121, it);
		if (strcmp(checkKey, key.c_str()) == 0) {
			Iterator foundIt(this, it);
			return foundIt;
		}
		bf.read(it, it + sizeof(char[363]));
	}
	Iterator notFound;
	return notFound;
}

int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
	hash<string> str_hash;
	BinaryFile::Offset it = (str_hash(key) % buckets) * sizeof(BinaryFile::Offset) + sizeof(int) + sizeof(BinaryFile::Offset);
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
		BinaryFile::Offset storeIt;
		bf.read(storeIt, it + sizeof(char[363]));
		bf.write(storeIt, whichBucket);
		BinaryFile::Offset prevDeletedHead;
		bf.read(prevDeletedHead, sizeof(int));
		if (prevDeletedHead != 0)
			bf.write(prevDeletedHead, it + sizeof(char[363]));
		bf.write(it, sizeof(int));
		it = storeIt;
		if (it == 0)
			return nodesRemoved;
		bf.read(checkKey, 121, it);
		bf.read(checkValue, 121, it + sizeof(char[121]));
		bf.read(checkContext, 121, it + sizeof(char[242]));
	}
	while (it != 0) {
		BinaryFile::Offset storeIt;
		bf.read(storeIt, it + sizeof(char[363]));
		if (storeIt == 0) {
			return nodesRemoved;
		}
		bf.read(checkKey, 121, storeIt);
		bf.read(checkValue, 121, storeIt + sizeof(char[121]));
		bf.read(checkContext, 121, storeIt + sizeof(char[242]));
		if (strcmp(checkKey, key.c_str()) == 0 && strcmp(checkValue, value.c_str()) == 0 && strcmp(checkContext, context.c_str()) == 0) {
			nodesRemoved++;
			BinaryFile::Offset nextIt;
			bf.read(nextIt, storeIt + sizeof(char[363]));
			bf.write(nextIt, it + sizeof(char[363]));
			BinaryFile::Offset prevDeletedHead;
			bf.read(prevDeletedHead, sizeof(int));
			bf.write(prevDeletedHead, storeIt + sizeof(char[363]));
			bf.write(storeIt, sizeof(int));
		}
		else {
			bf.read(it, it + sizeof(char[363]));
		}
	}
	return nodesRemoved;
}

DiskMultiMap::Iterator::Iterator() {
	pointingAt = -1;
}
// You may add additional constructors

DiskMultiMap::Iterator::Iterator(DiskMultiMap* reference, BinaryFile::Offset whereAt) {
	dmm = reference;
	pointingAt = whereAt;
}
bool DiskMultiMap::Iterator::isValid() const {
	return pointingAt != -1;
}
DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++() {
	if (pointingAt == -1)
		return *this;
	dmm->bf.read(pointingAt, pointingAt + sizeof(char[363]));
	if (pointingAt == 0)
		pointingAt = -1;
	return *this;
}
MultiMapTuple DiskMultiMap::Iterator::operator*() {
	MultiMapTuple mmt;
	char* readKey = new char[121];
	char* readValue= new char[121];
	char* readContext = new char[121];
	dmm->bf.read(readKey, 121, pointingAt);
	dmm->bf.read(readValue, 121, pointingAt + sizeof(char[121]));
	dmm->bf.read(readContext, 121, pointingAt + sizeof(char[242]));
	mmt.key = readKey;
	mmt.value = readValue;
	mmt.context = readContext;
	return mmt;
}


