/*
 * An abstraction of a physical vehicle.
 * Information:
 * current speed
 * position (x,y)
 * Itinerary: sequence of edge ids being travelled. It records entering and leaving times
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

	bool requestCurrentEdge(double currentTime);

	void requestCurrentSpeed();
	double getCurrentSpeed() const;

	void requestCurrentPosition();
    Position2D getCurrentPosition() const;

    void updateEdge(Edge & edge, double currentTime);
    void setEdgeLeavingTime(std::string edgeId, double currentTime);

    std::vector<std::string> getEdgesAhead();

    void reroute(std::string routeId);

//    void printPacketCounts(ostream & out);
//    std::map<std::string,RecordEntry> getRecords() const;
//    void recordEdge(std::string edgeId, long packetId, std::string senderId, double time, double travelTime);
//    std::map<std::string,RecordEntry> getlocalKnowledge() const;
//    map<std::string, double> computeTravelTimesOnRoutes(map<std::string,RecordEntry> edges, map<std::string, Route> routes, string startEdgeId, string endEdgeId);
//    map<string, double> getVanetCosts(std::string startEdgeId, std::string endEdgeId);
//    map<string, double> getGlobalCosts(std::string startEdgeId, std::string endEdgeId);

//    std::string getLastEdgeId();

protected:
    Ptr<ovnis::SumoTraciConnection> traci;
    std::string id;

    Position2D currentPosition;
    double currentSpeed;

    Scenario scenario;
    Route currentRoute ;
    Itinerary itinerary;
    void requestRoute(std::string routeId);

private:

};

} /* namespace ovnis */
#endif /* VEHICLE_H_ */
