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
	congestion = false;
	timeEstimationMethod = "last";
	traci = Names::Find<ovnis::SumoTraciConnection>("SumoTraci");
}

TIS::~TIS() {
}

std::string TIS::getTimeEstimationMethod() {
	return timeEstimationMethod;
}

void TIS::setTimeEstimationMethod(string method) {
	timeEstimationMethod = method;
}

double TIS::getDecayFactor() {
	return decayFactor;
}

void TIS::setDecayFactor(double factor) {
	decayFactor = factor;
}

void TIS::reportStartingRoute(string vehicleId, string currentEdgeId, string currentRouteId, string newEdgeId, string newRouteId,
		string originEdgeId, string destinationEdgeId, bool isCheater, bool isCongested,
		double expectedTravelTime, double shortestExpectedTravelTime) {
	// xxx only one entering on route - no need for subtract
	++vehiclesOnRoute[newRouteId];
	// time	vehicleId	currentEdgeId	currentRouteId	newEdgeId	newRouteId	originEdgeId	destinationEdgeId	isCheater	isCongested	expectedTravelTime	shortestExpectedTravelTime	vehiclesOnRoute[newRouteId]
//	Log::getInstance().getStream("routing_start") << Simulator::Now().GetSeconds() << "\t" << vehicleId << "\t" << currentEdgeId << "\t"
//			<< currentRouteId << "\t"<< newEdgeId << "\t"<< newRouteId << "\t"<< originEdgeId << "\t" << destinationEdgeId << "\t"
//			<< isCheater << "\t" << isCongested << "\t" << "\t" << expectedTravelTime << "\t" << shortestExpectedTravelTime << "\t" << vehiclesOnRoute[newRouteId];;
//	Log::getInstance().getStream("routing_start") << endl;
}

void TIS::reportEndingEdge(string vehicleId, string edgeId, double travelTime) {
	perfectTravelTimes[edgeId].add(0, "", Simulator::Now().GetSeconds(), travelTime);
//	cout << "Perfect ttdb\t" << edgeId << "\t" << Simulator::Now().GetSeconds() << "\t" << travelTime << "\tperfectTTDB\t" << perfectTravelTimes.size() << endl;
}

std::map<std::string, RecordEntry> TIS::getPerfectTravelTimes() {
	return perfectTravelTimes;
}

void TIS::reportEndingRoute(string vehicleId, string routeId, string startEdgeId, string endEdgeId,
		double startReroute, double travelTime, bool isCheater, double selfishExpectedTravelTime, double expectedTravelTime, bool wasCongested,
		string routingStrategy, double start, double staticCost) {
	--vehiclesOnRoute[routeId];
	travelTimeDateOnRoute[routeId] = Simulator::Now().GetSeconds();
	travelTimesOnRoute[routeId] = travelTime;
//	travelTimesOnRoute[routeId] = alfa*travelTime + (1-alfa)*travelTimesOnRoute[routeId]; // smooth
	double delayTime = travelTime - computeStaticCostExcludingMargins(routeId, startEdgeId, endEdgeId);
//	cout << "logging" << endl;
	double now = Simulator::Now().GetSeconds();
	NS_LOG_INFO ("aaaaaCreating Topology");
	//step	routeId	vehicleId	startReroute	travelTime	startEdgeId	endEdgeId	vehiclesOnRoute[routeId]	isCheater	selfishExpectedTravelTime	expectedTravelTime	wasCongested	delayTime	routingStrategy	start	(now-start)
	Log::getInstance().getStream("routing_end") << now << "\t" << routeId << "\t" << vehicleId << "\t" << startReroute << "\t"
			<< travelTime << "\t" << startEdgeId << "\t" << endEdgeId << "\t" << vehiclesOnRoute[routeId] << "\t" << isCheater << "\t"
			<< selfishExpectedTravelTime << "\t" << expectedTravelTime << "\t" << wasCongested << "\t" << delayTime
			<< "\t" << routingStrategy << "\t" << start << "\t" << (now-start) << "\t" << staticCost<< endl;
}

int TIS::getVehiclesOnRoute(string routeId) {
	map<string,int>::iterator it = vehiclesOnRoute.find(routeId);
	if (it != vehiclesOnRoute.end()) {
		return vehiclesOnRoute[routeId];
	}
	return 0;
}

void TIS::initializeStaticTravelTimes(map<string, Route> routes) {
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		if (this->staticRoutes.find(it->first) == this->staticRoutes.end()) {
			this->staticRoutes[it->first] = it->second;
			Route route = it->second;
			vehiclesOnRoute[it->first] = 0;
			travelTimeDateOnRoute[it->first] = 0;
			travelTimesOnRoute[it->first] = 0;
			for (vector<string>::iterator it2 = route.getEdgeIds().begin(); it2 != route.getEdgeIds().end(); ++it2) {
				if (staticRecords.find(*it2) == staticRecords.end()) {
					// add info about the edge
					staticRecords[*it2] = route.getEdgeInfo(*it2);
				}
			}
			// print route to file
			Log::getInstance().getStream("routes_info") << it->first << "\t";
			for (vector<EdgeInfo>::iterator edges_it = it->second.getEdgeInfos().begin(); edges_it != it->second.getEdgeInfos().end(); ++edges_it) {
				Log::getInstance().getStream("routes_info") << edges_it->getId() << "\t" << edges_it->getLength() << "\t" << edges_it->getMaxSpeed() << "\t";
			}
			Log::getInstance().getStream("routes_info") << endl;
		};
	}
}

std::map<std::string, EdgeInfo> & TIS::getStaticRecords() {
	return staticRecords;
}

double TIS::computeStaticCostExcludingMargins(string routeId, string startEdgeId, string endEdgeId) {
	double staticCost = 0;
		if (staticRoutes.find(routeId) != staticRoutes.end()) {
			bool isMonitored = false;
			vector<string> edgeIds = staticRoutes[routeId].getEdgeIds();
			for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
				if (*it == endEdgeId) {
					isMonitored = false;
				}
				if (isMonitored) {
					staticCost += staticRecords[*it].getStaticCost();
				}
				if (*it == startEdgeId) {
					isMonitored = true;
				}
			}
		}
		return staticCost;
}

map<string, double> TIS::getCosts(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	map<string, double> costs;
	map<string, double> packetAges;
	double now = Simulator::Now().GetSeconds();
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		costs[it->first] = computeStaticCostExcludingMargins(it->first, startEdgeId, endEdgeId);
	}
	for (map<string,double>::iterator it = travelTimesOnRoute.begin(); it != travelTimesOnRoute.end(); ++it) {
		double informationAge = 0;
		if (it->second != 0) {
//			double maxInformationAge = costs[it->first];
			double maxInformationAge = CENTRALISED_INFORMATION_TTL;
			informationAge = travelTimeDateOnRoute[it->first] == 0 ? 0 : now-travelTimeDateOnRoute[it->first];
			if (informationAge < maxInformationAge) {
				costs[it->first] = it->second;
			}
			else {
				informationAge = 0;
			}
		}
		packetAges[it->first] = informationAge;
	}
//	Log::getInstance().getStream("global_costs") << now << "\t";
//	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
//		Log::getInstance().getStream("global_costs") << it->first << "," << costs[it->first] << "," << packetAges[it->first] << "," << vehiclesOnRoute[it->first] << "\t";
//	}
//	Log::getInstance().getStream("global_costs") << endl;
	return costs;
}

string TIS::getEvent(map<string, double> probabilities) {
	// sort probabilities
	vector<pair<string, double> > sortedProbabilities = vector<pair<string, double> >();
	for (map<string, double>::iterator it = probabilities.begin(); it != probabilities.end(); ++it) {
		sortedProbabilities.push_back(pair<string,double>(it->first, it->second));
	}
//	Log::getInstance().getStream("probabilities") << Simulator::Now().GetSeconds();
//	for (vector<pair<string, double> >::iterator it = sortedProbabilities.begin(); it != sortedProbabilities.end(); ++it) {
//		Log::getInstance().getStream("probabilities") << "\t" << it->first << "\t" << it->second << "\t";
//	}
//	Log::getInstance().getStream("probabilities") << endl;

	double shifting_weight = 0;
    double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX;
    string lastChoice = "";

    vector<pair<string, double> >::iterator it;
    for (it = sortedProbabilities.begin(); it != sortedProbabilities.end(); ++it) {
    	r -= it->second;
    	r -= shifting_weight;
    	lastChoice = it->first;
    	if (r < eps) {
			return lastChoice;
		}
    }
    return lastChoice;
}

string TIS::chooseMinCostRoute(map<string,double> costs) {
	double minCost = numeric_limits<double>::max();
	string chosenRouteId = "";
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		double value = it->second;
		if (value >= 0 && value < minCost) {
			minCost = value;
			chosenRouteId = it->first;
		}
	}
	if (minCost == 0) {
		chosenRouteId = "";
	}
	return chosenRouteId;
}

string TIS::chooseRandomRoute() {
	string chosenRouteId = "";
	int chosenIndex = rand() % staticRoutes.size();
//	int chosenIndex = 0;
	int i = 0;
	for (map<string,Route>::iterator it = staticRoutes.begin(); it != staticRoutes.end(); ++it) {
		if (i == chosenIndex) {
			chosenRouteId = it->second.getId();
		}
		++i;
	}
	return chosenRouteId;
}

/**
 * If flow exceeds the capacity of the fastest road, then split the rest of the flow proportionally to other alternative routes
 */
//string TIS::chooseFlowAwareRoute(double flow, map<string,double> costs) {
//	string chosenRouteId = chooseMinCostRoute(costs);
//	double capacity = staticRoutes[chosenRouteId].getCapacity();
//	double flowRatioNeededToUseOtherAlternatives = (flow - capacity) / flow;
//	if (flowRatioNeededToUseOtherAlternatives <= 0) {
//		flowRatioNeededToUseOtherAlternatives = 0;
//	}
//	else if (flowRatioNeededToUseOtherAlternatives >= 1) {
//		flowRatioNeededToUseOtherAlternatives = 1;
//	}
//	//double random = rando.GetValue(0, 1);
//	double random = (double)(rand()%RAND_MAX)/(double)RAND_MAX;
//	if (random < flowRatioNeededToUseOtherAlternatives) {
//		map<string, double>::iterator it = costs.find(chosenRouteId);
//		if (it != costs.end()) {
//			costs.erase(it);
//		}
//		chosenRouteId = chooseProbTravelTimeRoute(costs);
//	}
//	return chosenRouteId;
//}

bool comp_prob(const pair<string,double>& v1, const pair<string,double>& v2)
{
	return v1.second < v2.second;
}

string TIS::chooseProbTravelTimeRoute(map<string,double> costs) {
	map<string, double> correlated;
	return chooseProbTravelTimeRoute(costs, correlated);
}

string TIS::chooseProbTravelTimeRoute(map<string,double> costs, map<string, double> correlated) {
	double minCost = numeric_limits<double>::max();
	double sumCost = 0;
	string chosenRouteId = "";
	int costsSize = 0;

	for (map<string, double>::iterator it = correlated.begin(); it != correlated.end(); ++it) {
		costs[it->first] = costs[it->first] + costs[it->first]*it->second;
	}

	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		sumCost += it->second;
		costsSize += it->second > 0 ? 1 : 0;
	}
	if (sumCost == 0 || costsSize == 0) {
		return "";
	}

	map<string, double> probabilities;
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		double probability = 0;
		if (it->second == 0) { // there are routes with no cost!
			// take only them into probabilities, skip the rest of routes
			probability = 1 / costsSize;
		}
		else {
			if (costsSize == costs.size()) {
				probability = (sumCost-it->second)/((costsSize-1)*sumCost);
			}
			else {
				probability = 0;
			}
		}

		if (probability != 0) {
			probabilities[it->first] = probability;
		}
	}
	// todo remove
//	probabilities = map<string, double>();
//	probabilities["main"] = 0.52;
//	probabilities["bypass"] = 0.48;
//	Log::getInstance().getStream("probabilities2") << Simulator::Now().GetSeconds() << "\t" << probabilities["bypass"] << "\t" << probabilities["main"]<< "\n";

	chosenRouteId = getEvent(probabilities);

	return chosenRouteId;
}

double TIS::getEdgeLength(std::string edgeId) {
	if (this->staticRecords.find(edgeId) != this->staticRecords.end()) {
		return this->staticRecords[edgeId].getLength();
	}
	return 0;
}

double TIS::getEdgeStaticCost(std::string edgeId) {
	if (this->staticRecords.find(edgeId) != this->staticRecords.end()) {
			return this->staticRecords[edgeId].getStaticCost();
		}
	return 0;
}

double TIS::getEdgePerfectCost(std::string edgeId) {
	return traci->GetEdgeTravelTime(edgeId);
}

bool TIS::isCongestion()
{
	return congestion;
}

void TIS::setCongestion(bool congestion, bool ifDense, bool ifCongested)
{
	if (this->congestion != congestion) {
//		Log::getInstance().getStream("congestion_changes") <<  Simulator::Now().GetSeconds() << "\t" << this->congestion << "\t->\t" << congestion <<  "\t" << ifDense << "\t" << ifCongested << endl;
		this->congestion = congestion;
	}
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
