/*
 * recordEntry.h
 *
 *  Created on: Sep 30, 2012
 *      Author: agata
 */

#ifndef RECORDENTRY_H_
#define RECORDENTRY_H_

#include <cstdlib>
#include <string>
#include <stdint.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <limits.h>
#include <stdint.h>

#include "ns3/ptr.h"
#include "ovnis-constants.h"

namespace ovnis {

class RecordEntry {
public:
	RecordEntry();
	virtual ~RecordEntry();
	void add(long packetId, std::string senderId, double time, double value);
//	void printValues();
//	double computeAverageValue();
	void setLocalMemorySize(int size); // size of an array
	void setLocalMemoryLength(int length); // time in seconds
//	double getValue();
	double getTime();
	double getValue(std::string estimationMethod);
	double setCapacity(double expectedValue);
	double getActualCapacity();
	double getExpectedValue();
	double getAlfa();
	void setAlfa(double alfa);
	std::string getLatestSenderId();
	long getLatestPacketId();
	void reset();
	std::string getInfo();
	double getAverageValue(int min, int max);

private:
	double getLatestValue();
	double getAverageValue();
	double getDecayValue();
	double getLatestTime();
	double getAverageTime();
	double getDecayTime();
	std::string id;
	double decayValue;
	double decayTime;
	double alfa;
//	int count;
//	int memoryStartTime;
//	double times[LOCAL_MEMORY_SIZE];
//	double values[LOCAL_MEMORY_SIZE];
//	std::string senders[LOCAL_MEMORY_SIZE];
//	long packetIds[LOCAL_MEMORY_SIZE];
	std::vector<double> times;
	std::vector<double> values;
	std::vector<long> packetIds;
	std::vector<std::string> senders;
	double expectedValue;
	double actualCapacity;
	int size;
	int memoryLength;
	int memoryMin;
	int memoryMax;
};

} /* namespace ovnis */

#endif /* RECORDENTRY_H_ */
