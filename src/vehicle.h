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

    void initialize(std::string id, long departureTime);

    void setScenario(Scenario scenario);
    Scenario & getScenario();

    std::string getId();
    void setId(std::string id);

    int getDepartureTime() const;
    int getArrivalTime() const;

    int getTravelTime() const;
    void setArrivalTime(int arrivalTime);

    void requestCurrentVehicleState(double currentTime);
	ovnis::Itinerary & getItinerary();
	std::map<std::string, Route> & getAlternativeRoutes();

    void requestCurrentEdge(double currentTime);
    void updateEdge(Edge & edge, double currentTime);
    void setEdgeLeavingTime(std::string edgeId, double currentTime);

    void requestCurrentSpeed();
    double getCurrentSpeed() const;

    void requestCurrentPosition();
    Position2D getCurrentPosition() const;

//    void tryReroute(std::string unavailableEdge);
    void requestCurrentTravelTimes();

    void reroute(std::string routeId);
	void recordPacket(long id);
	int getPacketCount(long id);
    void recordDouble(std::string id, long packetId, std::string senderId, double time, double value);
    void printLocalDatabase();
    void printDynamicCost();
    std::map<std::string,RecordEntry> getRecords() const;
    void setRecords(std::map<std::string,RecordEntry> records);
    void printPacketCounts(ostream & out);
    void setTravelTime(int travelTime);
    map<string, double> getVanetCosts();
    map<string, double> getGlobalCosts();
    double getLastSpeed() const;
    void setLastSpeed(double lastSpeed);

protected:
    Ptr<ovnis::SumoTraciConnection> traci;
    std::string id;

    int departureTime;
    int arrivalTime;
    int travelTime;

    Position2D currentPosition;
    double currentSpeed;
    double lastSpeed;
    Scenario scenario;

    Route currentRoute ;
    Itinerary itinerary;

private:
    void requestRoute(std::string routeId);

    std::map<std::string, RecordEntry> records;
    std::map<long,int> packets;
};

} /* namespace ovnis */
#endif /* VEHICLE_H_ */
