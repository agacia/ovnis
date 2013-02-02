/*
 * trafficInformationSystem.cpp
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#include "trafficInformationSystem.h"

using namespace std;

namespace ovnis
{

TIS & TIS::getInstance() {
	static TIS instance; // Guaranteed to be destroyed. Instantiated on first use.
	return instance;
}

TIS::TIS() {
}

TIS::~TIS() {
}


void TIS::reportStartingRoute(string routeId, string startEdgeId, string endEdgeId) {
	++vehiclesOnRoute[routeId];
	//travelTimeDateOnRoute[routeId] = Simulator::Now().GetSeconds();
}

void TIS::reportEndingRoute(string routeId, string startEdgeId, string endEdgeId, double travelTime) {
	--vehiclesOnRoute[routeId];
	travelTimeDateOnRoute[routeId] = Simulator::Now().GetSeconds();
	travelTimesOnRoute[routeId] = travelTime;
//	travelTimesOnRoute[routeId] = alfa*travelTime + (1-alfa)*travelTimesOnRoute[routeId]; // smooth
	Log::getInstance().getStream("fixedTIS") << Simulator::Now().GetSeconds() << "\t" << routeId << "\t" << travelTime << "\t" << startEdgeId << "\t" << endEdgeId << "\t" << vehiclesOnRoute[routeId];
	Log::getInstance().getStream("fixedTIS") << endl;
}

void TIS::initializeStaticTravelTimes(map<string, Route> routes) {
	if (this->routes.size() != 0) {
		return;
	}
	this->routes = routes;
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		Route route = it->second;
		vehiclesOnRoute[it->first] = 0;
		travelTimeDateOnRoute[it->first] = 0;
		travelTimesOnRoute[it->first] = 0;
		for (vector<string>::iterator it2 = route.getEdgeIds().begin(); it2 != route.getEdgeIds().end(); ++it2) {
			if (staticRecords.find(*it2) != staticRecords.end()) {
				// add info about the edge
				staticRecords[*it2] = route.getEdgeInfo(*it2);
			}
		}
	}
}

map<string, double> TIS::getCosts(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	map<string, double> costs;
	map<string, double> packetAges;
	double now = Simulator::Now().GetSeconds();

	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		costs[it->first] = it->second.computeStaticCostExcludingMargins(startEdgeId, endEdgeId);
	}
	for (map<string,double>::iterator it = travelTimesOnRoute.begin(); it != travelTimesOnRoute.end(); ++it) {
		double packetAge = 0;
		if (it->second != 0) {
			packetAge = travelTimeDateOnRoute[it->first] == 0 ? 0 : now-travelTimeDateOnRoute[it->first];
			if (packetAge < PACKET_TTL) {
				costs[it->first] = it->second;
			}
			else {
				packetAge = 0;
			}
		}
		packetAges[it->first] = packetAge;
	}

	Log::getInstance().getStream("global_costs") << now << "\t";
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		Log::getInstance().getStream("global_costs") << it->first << "," << costs[it->first] << "," << packetAges[it->first] << "," << vehiclesOnRoute[it->first] << "\t";
	}
	Log::getInstance().getStream("global_costs") << endl;

	return costs;
}

string TIS::getEvent(vector<pair<string, double> > probabilities) {
    double r = rando.GetValue(0, 1);
//    Log::getInstance().getStream("prob") << r << "\t" << endl;
//	for (vector<pair<string, double> >::iterator it = probabilities.begin(); it != probabilities.end(); ++it)
//	{
//		Log::getInstance().getStream("prob") << it->first << "," << it->second << " ";
//	}
//	Log::getInstance().getStream("prob") << endl;

    vector<pair<string, double> >::iterator it;
    //cout << r << endl;
    for (it = probabilities.begin(); it != probabilities.end(); ++it) {
    	r -= it->second;
//    	cout << "r: " << r << " " << it->first << "," << it->second << endl;
    	if (r < eps) {
			return it->first;
		}
    }
    return "";
}

string TIS::chooseMinTravelTimeRoute(map<string,double> costs) {
	double minCost = numeric_limits<double>::max();
	string chosenRouteId = "";
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		double value = it->second;
		if (value > 0 && value < minCost) {
			minCost = value;
			chosenRouteId = it->first;
		}
	}
	return chosenRouteId;
}

string TIS::chooseRandomRoute() {
	string chosenRouteId = "";
	int chosenIndex = rand() % routes.size();
	int i = 0;
	for (map<string,Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		if (i == chosenIndex) {
			chosenRouteId = it->second.getId();
		}
		++i;
	}
	return chosenRouteId;
}

string TIS::chooseFlowAwareRoute(double flow, map<string,double> costs) {
	string chosenRouteId = chooseMinTravelTimeRoute(costs);
	double capacity = routes[chosenRouteId].getCapacity();
	double flowRatioNeededToUseOtherAlternatives = (flow - capacity) / flow;
	if (flowRatioNeededToUseOtherAlternatives <= 0) {
		flowRatioNeededToUseOtherAlternatives = 0;
	}
	else if (flowRatioNeededToUseOtherAlternatives >= 1) {
		flowRatioNeededToUseOtherAlternatives = 1;
	}
	double random = rando.GetValue(0, 1);

	if (random < flowRatioNeededToUseOtherAlternatives) {
		map<string, double>::iterator it = costs.find(chosenRouteId);
		if (it != costs.end()) {
			costs.erase(it);
		}
		chosenRouteId = chooseProbTravelTimeRoute(costs);
	}
	return chosenRouteId;
}

bool comp_prob(const pair<string,double>& v1, const pair<string,double>& v2)
{
	return v1.second < v2.second;
}

string TIS::chooseProbTravelTimeRoute(map<string,double> costs) {
	double minCost = numeric_limits<double>::max();
	double sumCost = 0;

	string chosenRouteId = "";

	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		sumCost += it->second;
	}
	int costsSize = costs.size();
	if (sumCost == 0 || costsSize == 0) {
		return "";
	}
	if (costsSize == 1) {
		return costs.begin()->first;
	}
	vector<pair<string, double> > sortedProbabilities = vector<pair<string, double> >();
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		double probability = (sumCost-it->second)/((costsSize-1)*sumCost);
		sortedProbabilities.push_back(pair<string,double>(it->first, probability));
	}
//	sort(sortedProbabilities.begin(), sortedProbabilities.end(), comp_prob);

	chosenRouteId = getEvent(sortedProbabilities);
	return chosenRouteId;
}


//void TIS::DetectJam(double currentSpeed, double maxSpeed, string currentEdge) {
//
////	double maxLaneSpeed = vehicle.getItinerary().getEdgeMaxSpeed(currentEdge);
//	//decide if JAM or not according to a threshold
//	if (currentSpeed < (SPEED_THRESHOLD * maxSpeed)) {
//		if (m_time_jammed == 0) {
//			m_time_jammed = Simulator::Now().GetSeconds();
//		}
//		if (Simulator::Now().GetSeconds() - m_time_jammed > JAMMED_TIME_THRESHOLD) {
//			// I am in a jam for more than JAMMED_TIME_THRESHOLD send only ofr the first time
//			if (!m_jam_state) {
////				Ptr<Packet> packet = CreateWarningPacket();
////				SendPacket(packet);
//	//			m_sendEvent = Simulator::ScheduleNow(&TestApplication::SendMyState,this, Simulator::Now().GetSeconds(), vehicle.getCurrentEdge());
//			}
//			m_jam_state = true;
//			}
//	}
//	else {
//		// if ever I was in JAM state, then I am not anymore
//		if (m_jam_state) {
////			Ptr<Packet> packet = CreateWarningPacket();
////			SendPacket(packet);
//			m_jam_state = false;
//			m_time_jammed = 0;
//		}
//	}
//}


}
