/*
 * Stats.h
 *
 *  Created on: Mar 6, 2012
 *      Author: agata
 */

#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <sstream>
#include <cmath>
#include <math.h>
#include <time.h>
#include "ovnis-constants.h"
#include <vector>
#include <map>
#include "ns3/ptr.h"
#include "ns3/wifi-phy.h"
#include "xml_writer.hpp"
#include "ns3/vector.h"

#include <cstdlib>
#include <string>
//#include <set>
#include <limits.h>
#include <stdint.h>


namespace ovnis {

class StatInput {
	public:
	double sum;
	int inputs;
	std::string key;
};

class StatElem {
	public:
	double sum;
	double avg;
	int count;
	std::string key;
	std::vector<int> stepTimes;
	std::vector<int> values;
};

class Log
{
    public:
    	~Log();
		static Log & getInstance();// Guaranteed to be destroyed. Instantiated on first use.

		void summariseSimulation(std::string name);
		void logIn(VariableType key, int value, int time = -1);
		void logIn(std::string key, int value);
		void logInTime(int time);
		void printStatistic(VariableType key);
		std::string outList(std::vector<std::string> & list);
		void print(const std::string & msg);
		void print(char const * msg);
		void logMap(std::string name, double time, std::map<std::string,double> data, double sum);
		void packetSent();
		void packetSent(uint32_t size);
		void packetForwarded();
		void packetReceived();
		void packetReceived2();
		long getSentPackets();
		long getForwardedPackets();
		long getReceivedPackets();
		void packetDropped(enum ns3::WifiPhy::State);
		long getDroppedPackets(enum ns3::WifiPhy::State);
		long getDroppedPackets();
		void addDistance(double distance);
		double getAvgDistance();
		double getMaxDistance();

		void reportEdgePosition(std::string edgeId, double x, double y);

		volatile long getPacketId();
		void nextPacketId();

		std::ostream & getStream(std::string name);
		void closeStream(std::string name);
		void setOutputFolder(std::string folderName);
//		void openXml(std::string name);
//		void writeXml(const char * name, std::map<std::string, std::string> attrs);
//		void closeXml();

        int needProbabilistic;
        int couldCheat;
        int cheaters;

    private:
		Log();
        Log(Log const&);            // Don't Implement
        void operator=(Log const&); // Don't implement
        static Log* instance;

        std::map<std::string, std::ofstream *> logFiles;
		Writer * writer;
        std::string outputFolder;
        std::map<std::string,ns3::Vector2D> edgePositions;

    protected:
        typedef std::map<VariableType, StatElem> statEnumType;
        statEnumType statistics;

        int start, end, currentTime;
        std::vector<int> stepTimes;


        volatile double sumDistance;
        volatile long countDistance;
        double maxDistance;

        volatile long packetId;
        std::map<ns3::WifiPhy::State,long> dropped;
        volatile long received;
        volatile long received2;
        volatile long sent;
        long forwarded;

};

}

#endif /* LOG_H_ */
