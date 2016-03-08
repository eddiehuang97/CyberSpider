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
	Header header;
	header.head = 0;
	header.next = 0;
	if (!bf.write(header, bf.fileLength()))
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
	BinaryFile::Offset whichBucket = (str_hash(key) % buckets) * sizeof(BinaryFile::Offset) + sizeof(int) + sizeof(Header);
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
		bf.write(it, whichBucket);
		bf.write(newNode.key, it);
		bf.write(newNode.value, it + 121 * sizeof(char));
		bf.write(newNode.context, it + 242 * sizeof(char));
		bf.write(newNode.next, it + 363 * sizeof(char));
		bf.read(it, it + sizeof(BinaryFile::Offset));
		bf.write(it, sizeof(int));
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
	DiskMultiMap::Iterator test;
	return test;

}

int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context) {
	hash<string> str_hash;
	BinaryFile::Offset it = (str_hash(key) % buckets) * sizeof(BinaryFile::Offset) + sizeof(int) + sizeof(Header);
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
		BinaryFile::Offset prevHeaderHead;
		bf.read(prevHeaderHead, sizeof(int) + sizeof(BinaryFile::Offset));
		Header header;
		header.head = it;
		header.next = prevHeaderHead;
		bf.write(bf.fileLength(), sizeof(int) + sizeof(BinaryFile::Offset));
		bf.write(header, bf.fileLength());
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
			BinaryFile::Offset prevHeaderHead;
			bf.read(prevHeaderHead, sizeof(int) + sizeof(BinaryFile::Offset));
			Header header;
			header.head = storeIt;
			header.next = prevHeaderHead;
			bf.write(bf.fileLength(), sizeof(int) + sizeof(BinaryFile::Offset));
			bf.write(header, bf.fileLength());
			bf.read(storeIt, storeIt + 363 * sizeof(char));
			bf.write(storeIt, it + 363 * sizeof(char));
		}
		else {
			bf.read(it, it + sizeof(char[363]));
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
