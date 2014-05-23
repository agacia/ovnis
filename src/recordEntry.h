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
	void printValues();
//	double computeAverageValue();

	double getValue();
	double getValue(std::string estimationMethod);
	double getLatestTime();
	double getAverageTime();
	double setCapacity(double expectedValue);
	double getActualCapacity();
	double getExpectedValue();
	double getAlfa();
	void setAlfa(double alfa);
	std::string getLatestSenderId();
	long getLatestPacketId();
	void reset();

private:
	double getLatestValue();
	double getAverageValue();
	double getDecayValue();
	std::string id;
	double decayValue;
	double decayTime;
	double alfa;
	int count;
	double times[LOCAL_MEMORY_SIZE];
	double values[LOCAL_MEMORY_SIZE];
	std::string senders[LOCAL_MEMORY_SIZE];
	long packetIds[LOCAL_MEMORY_SIZE];
	double expectedValue;
	double actualCapacity;
};

} /* namespace ovnis */

#endif /* RECORDENTRY_H_ */
