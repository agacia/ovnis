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

class TrafficInformationSystem {
public:
	TrafficInformationSystem();
	virtual ~TrafficInformationSystem();
    void ReportRoute(std::string routeId, std::string startEdgeId, std::string endEdgeId, double travelTime, int numberOfVehicles);

	std::string ChooseMinTravelTimeRoute(std::map<std::string,double> costs);
	    std::string ChooseProbTravelTimeRoute(std::map<std::string,double> costs);
	    std::string ChooseFlowAwareRoute(double flow, std::map<std::string,double> costs);
		std::string ChooseRandomRoute();
		    std::string TakeDecision(Knowledge knowledge, std::string currentEdge, std::string destinationEdge);
		    void DetectJam(double currentSpeed, double maxSpeed, std::string currentEdge);
		    string GetEvent(vector<pair<string, double> > probabilities);
		    /**
			 * Sets m_jam_state according whether condition for traffic jam are met for the second time in a row.
			 * If there is traffic jam then broadcasts alert message containing egde's name.
			 */
		    void SendTrafficConditions(std::string currentEdge);
		    void setScenario(Scenario scenario);

private:
		    bool comp_prob(const pair<string,double>& v1, const pair<string,double>& v2);
		    /**
		         * True if this vehicle is considered in a traffic jam.
		         */
		        bool m_jam_state;
		        double m_time_jammed;

		        Scenario scenario;
		        UniformVariable  rando;
		        double flow;
		        const static double eps = 1e-9;
};

}
#endif /* TRAFFICINFORMATIONSYSTEM_H_ */
