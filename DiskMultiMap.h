#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include <functional>
#include <iostream>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap
{
public:

	class Iterator
	{
	public:
		Iterator();
		// You may add additional constructors
		Iterator(DiskMultiMap* reference, BinaryFile::Offset whereAt);
		bool isValid() const;
		Iterator& operator++();
		MultiMapTuple operator*();

	private:
		// Your private member declarations will go here
		BinaryFile::Offset pointingAt;
		DiskMultiMap* dmm;
	};

	DiskMultiMap();
	~DiskMultiMap();
	bool createNew(const std::string& filename, unsigned int numBuckets);
	bool openExisting(const std::string& filename);
	void close();
	bool insert(const std::string& key, const std::string& value, const std::string& context);
	Iterator search(const std::string& key);
	int erase(const std::string& key, const std::string& value, const std::string& context);

private:
	// Your private member declarations will go here
	BinaryFile bf;
	int buckets;
	struct DiskNode {
		char key[121];
		char value[121];
		char context[121];
		BinaryFile::Offset next;
	};
};

#endif // DISKMULTIMAP_H_
