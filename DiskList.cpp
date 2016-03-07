#include "DiskList.h"
#include <string>
using namespace std;

DiskList::DiskList(const std::string& filename) {
	bool success = bf.createNew(filename);
	if (!success) {
		cout << "Error! Unable to create " << filename << endl;
		return;
	}
	bf.write(sizeof(BinaryFile::Offset), 0);
}
bool DiskList::push_front(const char* data) {
	if (strlen(data) > 255)
		return false;
	BinaryFile::Offset readVal;
	bf.read(readVal, 0);
	if (bf.fileLength() == sizeof(BinaryFile::Offset)) {
		DiskNode newNode;
		strcpy_s(newNode.value, data);
		newNode.next = 0;
		newNode.active = true;
		bf.write(newNode, readVal);
	}
	else {
		BinaryFile::Offset prevHead;
		bf.read(prevHead, 0);
		DiskNode newNode;
		strcpy_s(newNode.value, data);
		newNode.next = prevHead;
		newNode.active = true;
		bool isActive;
		BinaryFile::Offset it = sizeof(BinaryFile::Offset);
		while (it < bf.fileLength() - 256 * sizeof(char) - sizeof(BinaryFile::Offset) && bf.read(isActive, it + 256 * sizeof(char) + sizeof(BinaryFile::Offset))) {
			if (!isActive) {
				bf.write(it, 0);
				bf.write(newNode, it);
				return true;
			}
			it += sizeof(DiskNode);
		}
		bf.write(bf.fileLength(), 0);
		bf.write(newNode, bf.fileLength());
	}
	return true;
}
bool DiskList::remove(const char* data) {
	BinaryFile::Offset it = 0;
	bool nodeRemoved = false;
	bf.read(it, it);
	char* test = new char[256];
	if (!bf.read(test, 256, it))
		return nodeRemoved;
	while (strcmp(data, test) == 0) {
		nodeRemoved = true;
		bf.write(false, it + 256 * sizeof(char) + sizeof(BinaryFile::Offset));
		BinaryFile::Offset storeIt;
		bf.read(storeIt, it + 256 * sizeof(char));
		bf.write(storeIt, 0);
		it = storeIt;
		if (!bf.read(test, 256, it))
			return nodeRemoved;
	}
	while (it != 0) {
		BinaryFile::Offset storeIt;
		bf.read(storeIt, it + 256 * sizeof(char));
		bf.read(test, 256, storeIt);
		if (strcmp(test, data) == 0) {
			nodeRemoved = true;
			bf.write(false, storeIt + 256 * sizeof(char) + sizeof(BinaryFile::Offset));
			bf.read(storeIt, storeIt + 256 * sizeof(char));
			bf.write(storeIt, it + 256 * sizeof(char));
		}
		else {
			bf.read(it, it + 256 * sizeof(char));
		}
	}
	return nodeRemoved;
}
void DiskList::printAll() {
	BinaryFile::Offset it;
	bf.read(it, 0);
	while (it != 0) {
		char* output = new char[256];
		bf.read(output, 256, it);
		int i = 0;
		while (output[i] != '\0' && i < 256) {
			cout << output[i];
			i++;
		}
		cout << endl;
		bf.read(it, it + 256 * sizeof(char));
	}
}