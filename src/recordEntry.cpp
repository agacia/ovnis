/*
 * recordEntry.cpp
 *
 *  Created on: Sep 30, 2012
 *      Author: agata
 */

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
#include <sstream>

#include "ns3/ptr.h"
#include "ovnis-constants.h"
#include "recordEntry.h"
#include "applications/trafficInformationSystem.h"

using namespace std;

namespace ovnis {

RecordEntry::RecordEntry() {
	size = LOCAL_MEMORY_SIZE;
	reset();
}

void RecordEntry::reset() {
	senders = vector<string>();
	times = vector<double>();
	values = vector<double>();
	packetIds = vector<long>();
	alfa = TIS::getInstance().getDecayFactor();
	decayValue = -1.0;
	decayTime = 0.0;
	memoryLength = 300;
	memoryMin = 0;
	memoryMax = memoryLength;
}


double RecordEntry::getAlfa() {
	return alfa;
}
void RecordEntry::setAlfa(double a) {
	alfa = a;
}

void RecordEntry::setLocalMemorySize(int s) {
	size = s;
	reset();
}

void RecordEntry::setLocalMemoryLength(int length) {
	memoryLength = length;
}


string RecordEntry::getInfo() {
	std::stringstream ss;
	ss << Simulator::Now().GetSeconds() << "\tsize:\t" << times.size() << "\ttimes\t";
	for (vector<double>::iterator it = times.begin(); it != times.end(); ++it) {
		ss << "\t" << *it;
	}
	ss << endl;
	std::string s = ss.str();
	return s;
}

void RecordEntry::add(long packetId, string senderId, double time, double value) {
	// check if the history exceeds memory length in second
//	int secondNow = static_cast<int>(Simulator::Now().GetSeconds());
//	if (secondNow >= memoryMax) { // history memory time passed
//		memoryMin = secondNow/memoryLength * memoryLength;
//		memoryMax = memoryMin + memoryLength;
////		std::stringstream ss;
////		ss << "reset " << Simulator::Now().GetSeconds() << "\tsize:\t" << times.size() << "\ttimes\t";
////		for (vector<double>::iterator it = times.begin(); it != times.end(); ++it) {
////			ss << "\t" << *it;
////		}
////		ss << " memoryMin" << memoryMin << " memoryMax " << memoryMax << endl;
////		std::string s = ss.str();
////		cout << s << endl;
//		times = vector<double>();
//		values = vector<double>();
//		packetIds = vector<long>();
//	}

	// save only if newer and falls into the memory period, or the first
	if (time >= memoryMin && (time > getLatestTime() || times.size() == 0)) {
		times.push_back(time);
		values.push_back(value);
		packetIds.push_back(packetId);
	}

}

//void RecordEntry::add(long packetId, string senderId, double time, double value) {
//	// add only newer or first!
////	if (getLatestTime() < time || count < 1) {
//	if (true) {
//		times[count%size] = time;
//		if (value == -1) {
//			cerr << "what is this? " << packetId << " " << senderId << " " << time << " " << value << endl;
//			value = values[(count-1)%size];
//		}
//		values[count%size] = value;
//		senders[count%size] = senderId;
//		packetIds[count%size] = packetId;
//		if (time > 0) {
//			if (count%size == 0) {
//				//oldestIndex = 0;
//			}
//			++count;
//		}
//	}
//	if (decayValue < 0) {
//		decayValue = value;
//		decayTime = time;
//	}
//	else if (time > decayTime) { // newer than the last timestamp in db 0.8historical + 0.2received
//		decayValue = alfa*decayValue + (1-alfa)*value;
//		decayTime = time;
//	}
////	else if (time < decayTime ){ // older than existing value in db
////		decayValue = (1-alfa)*decayValue + alfa*value;
////	}
//}

//void RecordEntry::printValues() {
//	cout << "[";
//	for (int i = 0; i < size; ++i) {
//		cout << times[i] << "," << values[i] << "," << senders[i] << " ";
//	}
//	cout << "]";
//}


//double RecordEntry::getValue() {
//	string estimationMethod = TIS::getInstance().getTimeEstimationMethod();
//	return getValue(estimationMethod);
//}

double RecordEntry::getValue(string estimationMethod) {
	double value = 0;
	if (estimationMethod == "last") {
		value= getLatestValue();
	}
	if (estimationMethod == "decay") {
		value = getDecayValue();
	}
	if (estimationMethod == "average") {
		int nowSeconds = static_cast<int>(Simulator::Now().GetSeconds());
		int delay = memoryLength;
		int min = nowSeconds - memoryLength;
		int max = min + memoryLength;
		value = getAverageValue(min, max);
//		if (times.size()  > 0) {
//			cout << Simulator::Now().GetSeconds() << " estimationMethod " << estimationMethod << " " << value << " " << min << "-" << max << " times.size " << times.size() << endl;
//		}
	}
	if (estimationMethod == "tmc") {
		int nowSeconds = static_cast<int>(Simulator::Now().GetSeconds());
		int delay = memoryLength;
		int min = nowSeconds / memoryLength * memoryLength - delay;
		int max = min + memoryLength;
		value = getAverageValue(min, max);
//		if (times.size()  > 0) {
//			cout << Simulator::Now().GetSeconds() << " estimationMethod " << estimationMethod << " " << value << " " << min << "-" << max << " times.size " << times.size() << endl;
//		}
		// remove
		int eraseTo = 0;
		int i = 0;
		for (vector<double>::iterator it = times.begin(); it != times.end(); ++it) {
			if (*it >= min && *it < max) {
				eraseTo = i;
				break;
			}
			++i;
		}
		if (eraseTo > 0) {
//			cout << Simulator::Now().GetSeconds() << "  Erasing " << times.size() << " from " << times[0] << " to " << times[eraseTo];
			times.erase(times.begin() + 1, times.begin() + eraseTo + 1);
			values.erase(values.begin() + 1, values.begin() + eraseTo + 1);
			packetIds.erase(packetIds.begin() + 1, packetIds.begin() + eraseTo + 1);
//			cout << " after " << times.size() << endl;
		}
	}
	return value;
}

double RecordEntry::getAverageValue(int min, int max) {
	double sum = 0;
	int count = 0;
	int i = 0;
	int eraseTo = 0;
	for (vector<double>::iterator it = times.begin(); it != times.end(); ++it) {
		if (*it >= min && *it < max) {
			++count;
			sum += values[i];
			eraseTo = i;
			++i;
		}
	}
	double average = sum / count;
	return average;
}

double RecordEntry::getTime() {
	string estimationMethod = TIS::getInstance().getTimeEstimationMethod();
	double value = 0;
	if (estimationMethod == "last") {
		value= getLatestTime();
	}
	if (estimationMethod == "decay") {
		value = getDecayTime();
	}
	if (estimationMethod == "average") {
		value = getAverageTime();
	}
	return value;
}

double RecordEntry::getDecayTime() {
	return decayTime;
}




double RecordEntry::getDecayValue() {
	return decayValue;
}

double RecordEntry::getLatestValue() {
//	return values[(count-1+size)%size];
	if (values.size() == 0) {
		return 0;
	}
	return values[values.size()-1];
}

double RecordEntry::getExpectedValue() {
	return expectedValue;
}

double RecordEntry::getActualCapacity() {
	return actualCapacity;
}

double RecordEntry::setCapacity(double expectedValue) {
	this->expectedValue = expectedValue;
	actualCapacity = expectedValue / getLatestValue(); // expected value < latestValue
	return actualCapacity;
}

double RecordEntry::getAverageValue() {
	double sum = 0;
	int num = 0;
	int length = size;
	length = values.size();
	for (int i = 0; i < length; ++i) {
		if (values[i] != 0) {
			sum += values[i];
			++num;
		}
	}
	return sum/num;
}

double RecordEntry::getAverageTime() {
	double sum = 0;
	int num = 0;
	int length = size;
	length = values.size();
	for (int i = 0; i < length; ++i) {
		if (values[i] != 0) {
			sum += times[i];
			++num;
		}
	}
	return sum/num;
}

double RecordEntry::getLatestTime() {
//	return times[(count-1+size)%size];
	if (times.size() == 0) {
		return 0;
	}
	return times[times.size()-1];
}

string RecordEntry::getLatestSenderId() {
//	return senders[(count-1+size)%size];
	if (senders.size() == 0) {
		return "";
	}
	return senders[senders.size()-1];
}

long RecordEntry::getLatestPacketId() {
//	return packetIds[(count-1+size)%size];
	if (packetIds.size() == 0) {
		return 0;
	}
	return packetIds[packetIds.size()-1];
}

//double RecordEntry::computeAverageValue() {
//	double sum = 0;
//	double count = 0;
//	for (int i = 0; i < size; ++i) {
//		sum += values[i];
//		if (values[i] != 0) {
//			++count;
//		}
//	}
//	return sum/count;
//}

RecordEntry::~RecordEntry() {
}

} /* namespace ovnis */
