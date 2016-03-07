#pragma once

#include <string>
#include "BinaryFile.h"

class DiskList
{
public:
	DiskList(const std::string& filename);
	bool push_front(const char* data);
	bool remove(const char* data);
	void printAll();
private:
	BinaryFile bf;
	struct DiskNode {
		char value[256];
		BinaryFile::Offset next;
		bool active;
	};
};