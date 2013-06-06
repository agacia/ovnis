/*
 * Logging about:
 * - simulation statistics:
 * 		- start and end times, duration
 * 		- any generic data: e.g. vehicles departed, arrived, left in simulation every step
 * - network performance:
 * 		- number of sent, received, dropped packets, mean distance of received packets
 * 		- keeping track on packetId *
 *
 * Writing to outstream.
 *
 * log.cpp
 *
 *  Created on: Mar 6, 2012
 *      Author: agata
 */

#include "log.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <fstream>
#include <cmath>
#include <math.h>
#include <time.h>
#include "ovnis-constants.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
namespace ovnis {

Log& Log::getInstance() {
	static Log instance; // Guaranteed to be destroyed. Instantiated on first use.
	return instance;
}

Log::Log() {
	start = -1;
	end = -1;
	currentTime = -1;
	sent = 0;
	received = 0;
	dropped[ns3::WifiPhy::TX] = 0;
	dropped[ns3::WifiPhy::RX] = 0;
	dropped[ns3::WifiPhy::SWITCHING] = 0;
	sumDistance = 0;
	countDistance = 0;
	maxDistance = 0;
	packetId = 0;
	logFiles = map<string, ofstream *>();

	congestedTrips = 0;
	cheaters = 0;
}

Log::~Log() {
}

void Log::logMap(string name, double time, map<string,double> data, double sum) {
	getStream(name) << time << "\t" << sum << "\t";
	for (map<string, double>::iterator it = data.begin(); it != data.end(); ++it) {
		getStream(name) << it->first << "," << it->second << "\t";
	}
	getStream(name) << endl;
}

void Log::reportEdgePosition(string edgeId, double x, double y) {
	map<string, ns3::Vector2D>::iterator it = edgePositions.find(edgeId);
	if (it == edgePositions.end()) {
		edgePositions[edgeId] = ns3::Vector2D(x, y);
		getStream("edges_positions") << edgeId << "\t" << x << "\t" << y << endl;
	}
}


void Log::setOutputFolder(string folderName) {
	outputFolder = folderName + "/ovnisOutput";
}

ostream & Log::getStream(string name) {
	ostream * log;
	if (strlen(name.c_str())==0) {
		log = &cout;
	}
	else if (logFiles.count(name) > 0) {
		log = (logFiles.find(name)->second);
	}
	else {
		ofstream * logfile = new ofstream();
		string fileName = outputFolder + "/" + name;
		logfile->open(fileName.c_str());
		logFiles[name] = logfile;
		log = logfile;
	}
	return *log;
}

void Log::closeStream(string name) {
	if (logFiles.count(name) > 0) {
		ofstream * log = logFiles.find(name)->second;
		log->close();
	}
}

void Log::packetSent() {
	sent++;
}

void Log::packetSent(uint32_t size) {
	sent++;
	cout << "sent " << packetId << " size: " << size << endl;
}

void Log::packetForwarded() {
	forwarded++;
}

volatile long Log::getPacketId() {
	return packetId;
}

void Log::nextPacketId() {
	++packetId;
}

void Log::packetReceived() {
	received++;
}

long Log::getSentPackets() {
	return sent;
}

long Log::getForwardedPackets() {
	return forwarded;
}

long Log::getReceivedPackets() {
	return received;
}

void Log::packetDropped(enum ns3::WifiPhy::State state) {
	 dropped[state]++;
}

long Log::getDroppedPackets(enum ns3::WifiPhy::State state) {
	if (dropped.count(state) > 0) {
		return dropped[state];
	}
	return 0;
}

long Log::getDroppedPackets() {
	long totalDropped = 0;
	for (map<enum ns3::WifiPhy::State,long>::iterator i = dropped.begin(); i != dropped.end(); ++i) {
		totalDropped += i->second;
	}
	return totalDropped;
}

void Log::addDistance(double distance) {
	sumDistance += distance;
	if (distance > maxDistance) {
		maxDistance = distance;
	}
	++countDistance;
}

double Log::getAvgDistance() {
	if (countDistance <= 0) {
		return 0;
	}
	return sumDistance / countDistance;
}

double Log::getMaxDistance() {
	return maxDistance;
}

void Log::print(const string & msg) {
	cout << msg;
}

void Log::print(char const * msg) {
	cout << msg;
}

void Log::printStatistic(VariableType key) {
	statEnumType::iterator stats = statistics.find(key);
	if (stats == statistics.end()) {
		return;
	}
	vector<int> values = stats->second.values;
	for (vector<int>::iterator i = values.begin(); i != values.end(); ++i) {
		if (i != values.begin()) {
			cout << ", ";
		}
		cout << (*i);
	}
	cout << endl;
	cout << key << " sum: " << stats->second.sum << endl;
}

void Log::summariseSimulation(string name) {
	getStream(name) << endl;
	getStream(name) << "Simulation: " << endl;
	getStream(name) << "start time [s]:\t\t" << start/SIMULATION_TIME_UNIT <<  endl;
	getStream(name) << "current time [s]:\t" << currentTime/SIMULATION_TIME_UNIT << endl;
	getStream(name) << "duration time [s]:\t" << (currentTime - start)/SIMULATION_TIME_UNIT << endl;
	getStream(name) << "Vehicles: " << endl;
	statEnumType::iterator departured = statistics.find(VEHICLES_DEPARTURED);
	statEnumType::iterator connected = statistics.find(VEHICLES_CONNECTED);
	statEnumType::iterator arrived = statistics.find(VEHICLES_ARRIVED);
	getStream(name) << "departured:\t" << departured->second.sum << endl;
	getStream(name) << "connected:\t" << connected->second.sum << endl;
	getStream(name) << "arrived:\t" << arrived->second.sum  << endl;
	getStream(name) << "Packets:\t" << endl;
	getStream(name) << "created:\t" << packetId <<  endl;
	getStream(name) << "forwarded:\t" << forwarded << endl;
	getStream(name) << "sent:\t" << sent << endl;
	getStream(name) << "received:\t" << received << endl;
	getStream(name) <<	"maxDistance:\t"  << maxDistance << "\tavgDistance "  << getAvgDistance() << endl;


}

void Log::logIn(VariableType key, int value, int time) {
	if (time != -1) {
		logInTime(time);
	}
	statEnumType::iterator iter = statistics.find(key);
	if (iter != statistics.end()) {
		iter->second.sum += value;
		iter->second.count++;
		iter->second.avg = iter->second.sum / iter->second.count;
		iter->second.stepTimes.push_back(currentTime);
		iter->second.values.push_back(value);
	}
	else {
		StatElem elem;
		elem.count = 1;
		elem.sum = value;
		elem.avg = value;
		elem.key = key;
		elem.stepTimes.push_back(currentTime);
		elem.values.push_back(value);
		statistics[key] = elem;
	}
}

void Log::logInTime(const int time) {
	if (start == -1) {
		start = time;
	}
	if (currentTime != time) {
		currentTime = time;
		stepTimes.push_back(currentTime);
	}
}

string Log::outList(std::vector<string> & list) {
	stringstream sout;
	for (vector<string>::iterator i = list.begin(); i != list.end(); ++i) {
		if (i != list.begin()) {
			sout << ", ";
		}
		sout << (*i);
	}
	sout << endl;
	return sout.str();
}

//void Log::openXml(string name) {
//	monitoredRoutesFile = new ofstream();
//	monitoredRoutesFile->open(name.c_str());
//	ostream & outstream = *monitoredRoutesFile;
//	writer = new Writer(outstream);
//	writer->openElt("tripinfos");
//}
//
//void Log::writeXml(const char * name, map<string, string> attrs) {
//	writer->openElt(name);
//	for (map<string,string>::iterator it = attrs.begin(); it != attrs.end(); ++it) {
//		writer->attr(it->first.c_str(), it->second);
//	}
//	writer->closeElt();
//}
//
//void Log::closeXml() {
//	writer->closeAll();
//	monitoredRoutesFile->close();
//}

}
