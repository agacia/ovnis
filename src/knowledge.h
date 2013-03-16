/*
 * knowledge.h
 *
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

	map<string, double> getCosts(map<string, Route> routes, string startEdgeId, string endEdgeId);
	bool isCapacityDrop(map<string, Route> routes, string startEdgeId, string endEdgeId) ;
	map<std::string,double> getCongestedLengthOnRoutes();
    map<std::string,double> getEdgesCosts(map<std::string, Route> routes, std::string startEdgeId, std::string endEdgeId);

protected:
	map<long,int> packets; // counter of packets
	std::map<std::string, RecordEntry> travelTimes; // info about travel times on routes
	std::map<std::string, int> numberOfVehicles;
	map<std::string,double> congestedLengthOnRoutes;
//	map<std::string, double> edgesCosts;

	double maxInformationAge;
};

}

#endif /* KNOWLEDGE_H_ */
