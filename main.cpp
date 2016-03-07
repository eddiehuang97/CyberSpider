#include "DiskMultiMap.h"
#include <string>
#include <iostream>
using namespace std;

int main() {
	DiskMultiMap x;
	x.createNew("myhashtable.dat", 100);
	x.insert("hmmm.exe", "pfft.exe", "m52902");
	x.insert("hmmm.exe", "pfft.exe", "m52902");
	x.insert("hmmm.exe", "pfft.exe", "m52902");
	x.insert("blah.exe", "bletch.exe", "m0003");
	x.insert("hmmm.exe", "pfft.exe", "m52903");
	x.insert("hmmm.exe", "pfft.exe", "m52902");
	x.insert("blah.exe", "bletch.exe", "m0003");
	x.insert("hmmm.exe", "pfft.exe", "m52903");
	cout << x.erase("hmmm.exe", "pfft.exe", "m52902");
	x.insert("hmmm.exe", "pfft.exe", "m52902");
	x.insert("hmmm.exe", "pfft.exe", "m52902");
	x.insert("hmmm.exe", "pfft.exe", "m52902");
	x.insert("blah.exe", "bletch.exe", "m0003");
	x.insert("hmmm.exe", "pfft.exe", "m52903");
	x.insert("hmmm.exe", "pfft.exe", "m52902");
	x.insert("blah.exe", "bletch.exe", "m0003");
	x.insert("hmmm.exe", "pfft.exe", "m52903");
	cout << x.erase("hmmm.exe", "pfft.exe", "m52902");
}