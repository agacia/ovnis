/*
 * knowledge.h
 *
/*
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#ifndef KNOWLEDGE_H_
#define KNOWLEDGE_H_

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <limits.h>
#include <stdint.h>

#include "ns3/ptr.h"
#include "applications/ovnis-application.h"
#include "vehicle.h"
#include "scenario.h"
#include "knowledge.h"
#include "route.h"
#include "ovnisPacket.h"
#include "itinerary.h"
#include "recordEntry.h"
#include "applications/trafficInformationSystem.h"
#include "traci/sumoTraciConnection.h"
#include <traci-server/TraCIConstants.h>

using namespace std;

namespace ovnis
{

class Knowledge {
public:
	Knowledge();
	virtual ~Knowledge();

	int getNumberOfVehicles(std::string edgeId);
	int addNumberOfVehicles(std::string edgeId);
	int substractNumberOfVehicles(std::string edgeId);

	bool record(Data data);
	void record(vector<Data> data);
	map<string,RecordEntry> & getRecords();

	void analyseLocalDatabase(map<string, Route> routes, string startEdgeId, string endEdgeId, map<string,double> routeTTL, bool usePerfectInformation);
	map<string,map<string,vector<string> > > analyseCorrelation(map<string, Route> routes, string startEdgeId, string endEdgeId);


	map<std::string,double> getCongestedLengthOnRoutes();
	map<std::string,double> getDenseLengthOnRoutes();
	map<std::string,double> getTravelTimesOnRoutes();
	map<std::string,double> getDelayOnRoutes();
	vector<string> getEdgesList(Route route, string startEdgeId, string endEdgeId);
	std::vector<std::string> getEdgesList(std::map<std::string, Route> routes, std::string startEdgeId, std::string endEdgeId);
	map<std::string,double> getEdgesCosts(std::vector<std::string> edgesList, double ttl, string knowledgeType);
    bool isCongestedFlow() const;
    bool isDenseFlow() const;

    double getSumDelay();
    double getSumCongested();
    double getSumDensed();
    double getSumLength();

protected:
    Ptr<ovnis::SumoTraciConnection> traci;
	map<long,int> packets; // counter of packets
	std::map<std::string, RecordEntry> travelTimes; // info about travel times on edges
	std::map<std::string, int> numberOfVehicles;
	map<std::string,double> congestedLengthOnRoutes;
	map<std::string,double> delayOnRoutes;
	map<std::string,double> denseLengthOnRoutes;
	map<std::string,double> travelTimesOnRoutes;
	map<string, double> packetAgesOnRoutes;
	map<string,int> numberOfUpdatedEdges;
	map<string,int> numberOfEdges;

//	map<string, double> packetAgesOnRoutes;
	double maxInformationAge;
	double congestionThreshold;
	double densityThreshold;
	bool ifCongestedFlow;
	bool ifDenseFlow;
	double sumLength;
	double congestedLength;
	double denseLength;
	double sumDelay;
};

}

#endif /* KNOWLEDGE_H_ */
