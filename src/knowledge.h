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

using namespace std;

namespace ovnis
{

class Knowledge {
public:
	Knowledge();
	virtual ~Knowledge();

	void PrintCosts(std::string fileName, std::string vehicleId, double time, map<std::string, double> costs);
	void recordDouble(std::string id, long packetId, std::string senderId, double time, double value);
	void setRecords(std::map<std::string,RecordEntry> records);
	void recordPacket(long id);
	int getPacketCount(long id);
	void recordEdge(string edgeId, long packetId, string senderId, double time, double travelTime);
	void printPacketCounts(ostream & out);
//	void setScenario(Scenario scenario);
	std::map<std::string,RecordEntry> getRecords() const;
	map<string,RecordEntry> getlocalKnowledge() const;
	map<string, double> getGlobalCosts(map<string, Route> routes, string startEdgeId, string endEdgeId);
	map<string, double> getVanetCosts(map<string, Route> routes, string startEdgeId, string endEdgeId);
	void InitializeStaticKnowledge(map<string, Route> routes);
	void InitializeGlobalKnowledge(std::map<std::string,Route> routes);
	void InitializeLocalKnowledge();
	map<string, double> computeTravelTimesOnRoutes(map<string, Route> routes, string startEdgeId, string endEdgeId);

protected:
	std::map<std::string, EdgeInfo> staticRecords; // info about travel times on routes
	std::map<std::string, RecordEntry> records; // info about travel times on routes
	std::map<std::string, RecordEntry> localKnowledge; // info about travel times on edges
	map<long,int> packets;
};

}

#endif /* KNOWLEDGE_H_ */
