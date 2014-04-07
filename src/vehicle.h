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

    void initialize(std::string id, double time);

    std::string getId();
    double getX();
    double getY();

    Network & getScenario();
    void setScenario(Network scenario);

    ovnis::Itinerary & getItinerary();
	std::map<std::string, Route> & getAlternativeRoutes();
	string getDestinationEdgeId();
	string getOriginEdgeId();

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

	double getEdgeTravelTime(std::string edgeId);
	map<std::string, double> getSumoCosts(std::string startEdgeId);

	double getStart();

protected:
    Ptr<ovnis::SumoTraciConnection> traci;
    std::string id;

    Position2D currentPosition;
    double currentSpeed;
    double start;
    Network scenario;
    Route currentRoute ;
    Itinerary itinerary;
    void requestRoute(std::string routeId);

private:

};

} /* namespace ovnis */
#endif /* VEHICLE_H_ */
