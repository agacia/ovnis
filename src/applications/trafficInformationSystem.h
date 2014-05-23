/*
 * trafficInformationSystem.h
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#ifndef TRAFFICINFORMATIONSYSTEM_H_
#define TRAFFICINFORMATIONSYSTEM_H_

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
#include "traci/sumoTraciConnection.h"
#include <traci-server/TraCIConstants.h>


using namespace std;

namespace ovnis
{

class TIS {
public:
	virtual ~TIS();
	static TIS & getInstance(); // Guaranteed to be destroyed. Instantiated on first use.

	void reportStartingRoute(string vehicleId, string currentEdgeId, string currentRouteId, string newEdgeId, string newRouteId,
			string originEdgeId, string destinationEdgeId, bool isCheater, bool isCongested,
			double expectedTravelTime, double shortestExpectedTravelTime);
	void reportEndingRoute(string vehicleId, std::string routeId, std::string startEdgeId, std::string endEdgeId,
			double startReroute, double travelTime, bool isCheater, double selfishExpectedTravelTime, double expectedTravelTime, bool wasCongested,
			string routingStrategy, double start, double staticCost);
	void reportEndingEdge(string vehicleId, string edgeId, double travelTime);

    int getVehiclesOnRoute(std::string routeId);
	std::map<std::string,double> & getTravelTimesOnRoute();
	std::map<std::string,double> & getTravelTimeDateOnRoute();

	double getEdgePerfectCost(std::string edgeId);
	void vehicleOnRoadsInitialize(std::string routeId);

	void initializeStaticTravelTimes(map<string, Route> routes);
	map<string, double> getCosts(map<string, Route> routes, string startEdgeId, string endEdgeId);
	double computeStaticCostExcludingMargins(string routeId, string startEdgeId, string endEdgeId);
	std::map<std::string, EdgeInfo> & getStaticRecords();
	double getEdgeLength(std::string edgeId);
	double getEdgeStaticCost(std::string edgeId);
	bool isCongestion();
	void setCongestion(bool congestion, bool ifDense, bool ifCongested);
	std::string getTimeEstimationMethod();
	void setTimeEstimationMethod(std::string method);
	double getDecayFactor();
	void setDecayFactor(double factor);

	std::string chooseMinCostRoute(std::map<std::string,double> costs);
	std::string chooseProbTravelTimeRoute(std::map<std::string,double> costs);
	std::string chooseProbTravelTimeRoute(std::map<std::string,double> costs, std::map<std::string, double> correlated);
	std::string chooseFlowAwareRoute(double flow, std::map<std::string,double> costs);
	std::string chooseRandomRoute();
	std::string getEvent(std::map<std::string, double> probabilities);
    std::map<std::string, RecordEntry> getPerfectTravelTimes();

    bool executeOnce;
    int communities;
    //	void DetectJam(double currentSpeed, double maxSpeed, std::string currentEdge);
private:
    TIS();
    TIS(const TIS& ); // Don't Implement
    void operator =(const TIS& ); // Don't implement
    static TIS *instance;
    Ptr<ovnis::SumoTraciConnection> traci;
    string timeEstimationMethod;
    double decayFactor;
    std::map<std::string,EdgeInfo> staticRecords; // info about expected travel times on routes (whith max speed)

    std::map<std::string,Route> staticRoutes;
    std::map<std::string,int> vehiclesOnRoute;
    std::map<std::string,double> travelTimesOnRoute;
    std::map<std::string,double> travelTimeDateOnRoute;
	std::map<std::string, RecordEntry> perfectTravelTimes; // info about travel times on routes

    bool congestion;
    bool comp_prob(const pair<string,double> & v1, const pair<string,double> & v2);
    UniformVariable rando;
    static const double eps = 1e-9;
    static const double alfa = 0.;

};

}
#endif /* TRAFFICINFORMATIONSYSTEM_H_ */
