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


using namespace std;

namespace ovnis
{

class TIS {
public:

	virtual ~TIS();
	static TIS & getInstance(); // Guaranteed to be destroyed. Instantiated on first use.

	void reportStartingRoute(std::string routeId, std::string startEdgeId, std::string endEdgeId);
    void reportEndingRoute(std::string routeId, std::string startEdgeId, std::string endEdgeId, double travelTime);

    std::map<std::string,int> & getVehiclesOnRoute();
	std::map<std::string,double> & getTravelTimesOnRoute();
	std::map<std::string,double> & getTravelTimeDateOnRoute();

	void vehicleOnRoadsInitialize(std::string routeId);

	void initializeStaticTravelTimes(map<string, Route> routes);
	map<string, double> getCosts(map<string, Route> routes, string startEdgeId, string endEdgeId);
	double computeStaticCostExcludingMargins(string routeId, string startEdgeId, string endEdgeId);
	std::map<std::string, EdgeInfo> & getStaticRecords();

	std::string chooseMinTravelTimeRoute(std::map<std::string,double> costs);
	std::string chooseProbTravelTimeRoute(std::map<std::string,double> costs);
	std::string chooseFlowAwareRoute(double flow, std::map<std::string,double> costs);
	std::string chooseRandomRoute();
	string getEvent(vector<pair<string, double> > probabilities);

//	void DetectJam(double currentSpeed, double maxSpeed, std::string currentEdge);

private:

    TIS();
    TIS(TIS const&);           	// Don't Implement
	void operator=(TIS const&); 						// Don't implement
	static TIS * instance;

	std::map<std::string, EdgeInfo> staticRecords; // info about expected travel times on routes (whith max speed)

    std::map<std::string,Route> staticRoutes;
    std::map<std::string,int> vehiclesOnRoute;
    std::map<std::string,double> travelTimesOnRoute;
    std::map<std::string,double> travelTimeDateOnRoute;

//		        bool m_jam_state;
//		        double m_time_jammed;
//		        double flow;
	bool comp_prob(const pair<string,double>& v1, const pair<string,double>& v2);
	UniformVariable  rando;
	const static double eps = 1e-9;
	const static double alfa = 0.5;
};

}
#endif /* TRAFFICINFORMATIONSYSTEM_H_ */
