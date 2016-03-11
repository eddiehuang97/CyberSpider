#include "IntelWeb.h"
using namespace std;

bool operator<(const InteractionTuple &a, const InteractionTuple &b) {
	if (a.context < b.context)
		return true;
	if (a.context > b.context)
		return false;
	if (a.from < b.from)
		return true;
	if (a.from > b.from)
		return false;
	if (a.to < b.to)
		return true;
	else
		return false;
}


IntelWeb::IntelWeb() {}
IntelWeb::~IntelWeb() {
	close();
}
bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxDataItems) {
	int hashTableSize = maxDataItems / (.75);
	if (!forward.createNew(filePrefix + "-forward.dat", hashTableSize) ||
		!reverse.createNew(filePrefix + "-reverse.dat", hashTableSize))
		return false;
	return true;
}
bool IntelWeb::openExisting(const std::string& filePrefix) {
	forward.close();
	reverse.close();
	if (!forward.openExisting(filePrefix + "-forward.dat") ||
		!reverse.openExisting(filePrefix + "-reverse.dat"))
		return false;
	return true;
}
void IntelWeb::close() {
	forward.close();
	reverse.close();
}
bool IntelWeb::ingest(const std::string& telemetryFile) {
	ifstream inf(telemetryFile);
	if (!inf)
		return false;
	string line;
	while (getline(inf, line)) {
		istringstream iss(line);
		string field1;
		string field2;
		string field3;
		iss >> field1 >> field2 >> field3;
		forward.insert(field2, field3, field1);
		reverse.insert(field3, field2, field1);
	}
	return true;
}
unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
	unsigned int minPrevalenceToBeGood,
	std::vector<std::string>& badEntitiesFound,
	std::vector<InteractionTuple>& badInteractions
	) {
	badInteractions.clear();
	badEntitiesFound.clear();
	unsigned int numMaliciousEntities = 0;
	map<string, int> prevalenceMap;
	set<string> entitiesChecked;
	queue<string> entitiesToCheck;
	set<InteractionTuple> storeBadInteractions;
	vector<string>::const_iterator vectorIt = indicators.begin();
	while (vectorIt != indicators.end()) {
		entitiesToCheck.push(*vectorIt);
		entitiesChecked.insert(*vectorIt);
		numMaliciousEntities++;
		vectorIt++;
	}
	while (!entitiesToCheck.empty()) {
		string currentlyChecking = entitiesToCheck.front();
		entitiesChecked.insert(currentlyChecking);
		entitiesToCheck.pop();
		DiskMultiMap::Iterator it = forward.search(currentlyChecking);
		if (it.isValid()) {
			do {
				MultiMapTuple m = *it;
				InteractionTuple i;
				i.context = m.context;
				i.from = m.key;
				i.to = m.value;
				if (i.from != currentlyChecking) {
					++it;
					continue;
				}
				storeBadInteractions.insert(i);
				map<string, int>::iterator mapIt = prevalenceMap.find(i.to);
				int prevalence;
				if (mapIt == prevalenceMap.end()) {
					prevalence = getPrevalence(i.to);
					prevalenceMap[i.to] = prevalence;
				}
				else {
					prevalence = (*mapIt).second;
				}
				if (prevalence < minPrevalenceToBeGood) {
					set<string>::iterator checkChecked = entitiesChecked.find(i.to);
					if (checkChecked == entitiesChecked.end()) {
						entitiesToCheck.push(i.to);
						numMaliciousEntities++;
					}
				}
				++it;
			} while (it.isValid());
			it = reverse.search(currentlyChecking);
			if (it.isValid()) {
				do {
					MultiMapTuple m = *it;
					InteractionTuple i;
					i.context = m.context;
					i.from = m.value;
					i.to = m.key;
					if (i.from != currentlyChecking) {
						++it;
						continue;
					}
					storeBadInteractions.insert(i);
					map<string, int>::iterator mapIt = prevalenceMap.find(i.to);
					int prevalence;
					if (mapIt == prevalenceMap.end()) {
						prevalence = getPrevalence(i.to);
						prevalenceMap[i.to] = prevalence;
					}
					else {
						prevalence = (*mapIt).second;
					}
					if (prevalence < minPrevalenceToBeGood) {
						set<string>::iterator checkChecked = entitiesChecked.find(i.to);
						if (checkChecked == entitiesChecked.end()) {
							entitiesToCheck.push(i.to);
							numMaliciousEntities++;
						}
					}
					++it;
				} while (it.isValid());
			}
		}
	}
	set<string>::iterator setBadEntities = entitiesChecked.begin();
	while (setBadEntities != entitiesChecked.end()) {
		string badEntity = *setBadEntities;
		vector<string>::const_iterator indicatorIt = find(indicators.begin(), indicators.end(), badEntity);
		if (indicatorIt == indicators.end()) {
			badEntitiesFound.push_back(badEntity);
		}
		setBadEntities++;
	}
	set<InteractionTuple>::iterator setBadInteractions = storeBadInteractions.begin();
	while (setBadInteractions != storeBadInteractions.end()) {
		badInteractions.push_back(*setBadInteractions);
		setBadInteractions++;
	}
	return numMaliciousEntities;
}
bool IntelWeb::purge(const std::string& entity) {
	bool wasPurged = false;
	queue<string> forwardValues;
	queue<string> reverseValues;
	queue<MultiMapTuple> toDeleteForwards;
	queue<MultiMapTuple> toDeleteReverse;
	DiskMultiMap::Iterator it = forward.search(entity);
	if (it.isValid()) {
		wasPurged = true;
		do {
			MultiMapTuple m = *it;
			if (m.key == entity) {
				forwardValues.push(m.value);
				toDeleteForwards.push(m);
			}
			++it;
		} while (it.isValid());
	}
	it = reverse.search(entity);
	if (it.isValid()) {
		wasPurged = true;
		do {
			MultiMapTuple m = *it;
			if (m.key == entity) {
				reverseValues.push(m.value);
				toDeleteReverse.push(m);
			}
			++it;
		} while (it.isValid());
	}
	while (!forwardValues.empty()) {
		string currentlyChecking = forwardValues.front();
		forwardValues.pop();
		it = reverse.search(currentlyChecking);
		while (it.isValid()) {
			MultiMapTuple m = *it;
			if (m.value == entity) {
				toDeleteReverse.push(m);
			}
			++it;
		}
	}
	while (!reverseValues.empty()) {
		string currentlyChecking = reverseValues.front();
		reverseValues.pop();
		it = forward.search(currentlyChecking);
		while (it.isValid()) {
			MultiMapTuple m = *it;
			if (m.value == entity) {
				toDeleteForwards.push(m);
			}
			++it;
		}
	}
	while (!toDeleteForwards.empty()) {
		MultiMapTuple m = toDeleteForwards.front();
		toDeleteForwards.pop();
		forward.erase(m.key, m.value, m.context);
	}
	while (!toDeleteReverse.empty()) {
		MultiMapTuple m = toDeleteReverse.front();
		toDeleteReverse.pop();
		forward.erase(m.key, m.value, m.context);
	}
	return wasPurged;
}

int IntelWeb::getPrevalence(string& field) {
	int prevalence = 0;
	DiskMultiMap::Iterator it = forward.search(field);
	if (it.isValid()) {
		do {
			prevalence++;
			++it;
		} while (it.isValid());
	}
	it = reverse.search(field);
	if (it.isValid()) {
		do {
			prevalence++;
			++it;
		} while (it.isValid());
	}
	return prevalence;
}
