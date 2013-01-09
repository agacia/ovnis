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
#include "ns3/wifi-phy.h"
#include "xml_writer.hpp"


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
		void packetSent();
		void packetForwarded();
		void packetReceived();
		long getSentPackets();
		long getForwardedPackets();
		long getReceivedPackets();
		void packetDropped(enum ns3::WifiPhy::State);
		long getDroppedPackets(enum ns3::WifiPhy::State);
		long getDroppedPackets();
		void addDistance(double distance);
		double getAvgDistance();
		double getMaxDistance();
		std::ostream & getStream(std::string name);
		void closeStream(std::string name);
//		std::vector<std::vector<std::string> > & readRoutesFromFile(std::string name);
//		std::vector<std::vector<std::string> > & getRoutes();
		void setOutputFolder(std::string folderName);
		void openXml(std::string name);
		void writeXml(const char * name, std::map<std::string, std::string> attrs);
		void closeXml();
		long getPacketSent();
		long getPacketForwarded();
		volatile long getPacketId();
		void nextPacketId();
		std::map<int,int> forwardedPackets;
		bool routeInitialization;

		std::map<std::string,int> & getVehiclesOnRoute();
		std::map<std::string,double> & getTravelTimesOnRoute();
		std::map<std::string,double> & getTravelTimeDateOnRoute();

		void vehicleEnter(double time, std::string routeId);
		void vehicleLeave(double time, std::string routeId, double travelTime);
		void vehicleOnRoadsInitialize(std::string routeId);

    private:
		Log();
        Log(Log const&);            // Don't Implement
        void operator=(Log const&); // Don't implement
        static Log* instance;
        std::map<std::string, std::ofstream *> logFiles;
        std::ofstream * monitoredRoutesFile;
		Writer * writer;
        std::string outputFolder;
        std::vector<std::vector<std::string> > routes;

        std::map<std::string,int> vehiclesOnRoute;
        std::map<std::string,double> travelTimesOnRoute;
        std::map<std::string,double> travelTimeDateOnRoute;

    protected:
        typedef std::map<VariableType, StatElem> statEnumType;
        statEnumType statistics;
        int start, end;
        int currentTime;
        std::vector<int> stepTimes;
        long sent;
        long forwarded;
        std::map<ns3::WifiPhy::State,long> dropped;
        long received;
        double sumDistance;
        double avgDistance;
        long countDistance;
        double maxDistance;
        volatile long packetId;
};

}

#endif /* LOG_H_ */
