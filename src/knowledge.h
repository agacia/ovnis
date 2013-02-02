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

	int getNumberOfVehicles(std::string edgeId);
	int addNumberOfVehicles(std::string edgeId);
	int substractNumberOfVehicles(std::string edgeId);

	void record(Data data);
	void record(vector<Data> data);

	map<string,RecordEntry> & getRecords();

	map<string, double> getCosts(map<string, Route> routes, string startEdgeId, string endEdgeId);
	map<string, double> computeTravelTimesOnRoutes(map<string, Route> routes, string startEdgeId, string endEdgeId);

/*
	void PrintCosts(std::string fileName, std::string vehicleId, double time, map<std::string, double> costs);
	void recordDouble(std::string id, long packetId, std::string senderId, double time, double value, int numberOfVehicles);
	void setRecords(std::map<std::string,RecordEntry> records);
	void recordPacket(long id);
	int getPacketCount(long id);
	void recordEdge(string edgeId, long packetId, string senderId, double time, double travelTime, int numberOfVehicles);
	void printPacketCounts(ostream & out);
	std::map<std::string,RecordEntry> getRecords() const;

	void InitializeStaticKnowledge(map<string, Route> routes);
	void InitializeGlobalKnowledge(std::map<std::string,Route> routes);
	void InitializeLocalKnowledge();

*/
protected:
	map<long,int> packets; // counter of packets
	std::map<std::string, RecordEntry> travelTimes; // info about travel times on routes
	std::map<std::string, int> numberOfVehicles;
};

}

#endif /* KNOWLEDGE_H_ */
