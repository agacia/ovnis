/*
 * vehicle.h
 *
 *  Created on: Mar 18, 2012
 *      Author: agata
 */

#ifndef VEHICLE_H_
#define VEHICLE_H_

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <limits.h>
#include <stdint.h>

#include "ns3/ptr.h"

#include "scenario.h"
#include "route.h"
#include "itinerary.h"
#include "recordEntry.h"
#include "traci/sumoTraciConnection.h"
#include <traci-server/TraCIConstants.h>

using namespace ns3;

namespace ovnis
{

class Vehicle {

public:
	Vehicle();
	virtual ~Vehicle();

    void initialize(std::string id);

    std::string getId();

    Scenario & getScenario();
    void setScenario(Scenario scenario);

    ovnis::Itinerary & getItinerary();
	std::map<std::string, Route> & getAlternativeRoutes();
	string getDestinationEdgeId();

	double getLastSpeed() const;
	void setLastSpeed(double lastSpeed);

	void requestCurrentEdge(double currentTime);

	void requestCurrentSpeed();
	double getCurrentSpeed() const;

	void requestCurrentPosition();
    Position2D getCurrentPosition() const;

    void updateEdge(Edge & edge, double currentTime);
    void setEdgeLeavingTime(std::string edgeId, double currentTime);


    void reroute(std::string routeId);

  void printPacketCounts(ostream & out);
    std::map<std::string,RecordEntry> getRecords() const;

    void recordEdge(std::string edgeId, long packetId, std::string senderId, double time, double travelTime);
    std::map<std::string,RecordEntry> getlocalKnowledge() const;
    map<std::string, double> computeTravelTimesOnRoutes(map<std::string,RecordEntry> edges, map<std::string, Route> routes, string startEdgeId, string endEdgeId);

    map<string, double> getVanetCosts(std::string startEdgeId, std::string endEdgeId);
    map<string, double> getGlobalCosts(std::string startEdgeId, std::string endEdgeId);


protected:
    Ptr<ovnis::SumoTraciConnection> traci;
    std::string id;

    Position2D currentPosition;
    double currentSpeed;
    double lastSpeed;

    Scenario scenario;
    Route currentRoute ;
    Itinerary itinerary;

    std::map<std::string, double> neighbourList;

private:
    void requestRoute(std::string routeId);

    std::map<long,int> packets;
};

} /* namespace ovnis */
#endif /* VEHICLE_H_ */
