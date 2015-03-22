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
	++vehiclesOnRoute[newRouteId];
	// time	vehicleId	currentEdgeId	currentRouteId	newEdgeId	newRouteId	originEdgeId	destinationEdgeId	isCheater	isCongested	expectedTravelTime	shortestExpectedTravelTime	vehiclesOnRoute[newRouteId]
//	Log::getInstance().getStream("routing_start") << Simulator::Now().GetSeconds() << "\t" << vehicleId << "\t" << currentEdgeId << "\t"
//			<< currentRouteId << "\t"<< newEdgeId << "\t"<< newRouteId << "\t"<< originEdgeId << "\t" << destinationEdgeId << "\t"
//			<< isCheater << "\t" << isCongested << "\t" << "\t" << expectedTravelTime << "\t" << shortestExpectedTravelTime << "\t" << vehiclesOnRoute[newRouteId];;
//	Log::getInstance().getStream("routing_start") << endl;
}

void TIS::reportEndingEdge(string vehicleId, string edgeId, double travelTime) {
	perfectTravelTimes[edgeId].add(0, "", Simulator::Now().GetSeconds(), travelTime);
//	Log::getInstance().getStream("reports") << edgeId << " " << perfectTravelTimes[edgeId].getInfo();
}

std::map<std::string, RecordEntry> TIS::getPerfectTravelTimes() {
	return perfectTravelTimes;
}

std::map<std::string, RecordEntry> TIS::getTMCTravelTimes() {
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
					// initialize perfect travel times with large size
					perfectTravelTimes[*it2].setLocalMemorySize(100);
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

double TIS::computeLengthExcludingMargins(string routeId, string startEdgeId, string endEdgeId) {
	double length = 0.0;
		if (staticRoutes.find(routeId) != staticRoutes.end()) {
			bool isMonitored = false;
			vector<string> edgeIds = staticRoutes[routeId].getEdgeIds();
			for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
				if (*it == endEdgeId) {
					isMonitored = false;
				}
				if (isMonitored) {
					length += staticRecords[*it].getLength();
				}
				if (*it == startEdgeId) {
					isMonitored = true;
				}
			}
		}
		return length;
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

bool comp_prob(const pair<string,double>& v1, const pair<string,double>& v2)
{
	return v1.second < v2.second;
}

map<string, double> TIS::getProbabilities(map<string,double> costs) {
	map<string, double> correlated;
	return getProbabilities(costs, correlated);
}

map<string, double> TIS::getProbabilities(map<string,double> costs, map<string, double> correlated) {
	double minCost = numeric_limits<double>::max();
	double sumCost = 0;
	map<string, double> probabilities;
	int costsSize = 0;

	// add corellation penalty (cost * correlation_value)
	for (map<string, double>::iterator it = correlated.begin(); it != correlated.end(); ++it) {
		costs[it->first] += + costs[it->first]*it->second;
	}
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		sumCost += it->second;
		costsSize += it->second > 0 ? 1 : 0;
	}
	if (sumCost == 0 || costsSize == 0) {
		return probabilities;
	}
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
	return probabilities;
}


map<string, double> TIS::getMNLProbabilities(map<string,double> costs, Vehicle * vehicle) {
	return getCLogitProbabilities(costs, vehicle, 0.0, 1.0, 1.0);
}

map<string, double> TIS::getCLogitProbabilities(map<string,double> costs, Vehicle * vehicle, double beta, double theta, double gamma) {
	map<string, double> probabilities;
	map<string, double> commonalities;
	string startEdgeId = (vehicle->getItinerary()).getCurrentEdge().getId();
	string endEdgeId = vehicle->getDestinationEdgeId();
	std::map<std::string,Route> alternatives = vehicle->getScenario().getAlternativeRoutes();
	Log::getInstance().getStream("mnl") << Simulator::Now().GetSeconds() << " vehicle " << vehicle->getId()
			<< " mnl choice of " << alternatives.size() << " alternatives: " << endl;
	Log::getInstance().getStream("c-logit") << Simulator::Now().GetSeconds() << " vehicle " << vehicle->getId()
					<< " edges: " << startEdgeId << "-" << endEdgeId
					<< " c-logit choice of " << alternatives.size() << " alternatives: " << endl;

	if (beta > 0) {
		// C-logit - calculate commonalities
		for (std::map<std::string, Route>::iterator itR = alternatives.begin(); itR != alternatives.end(); ++itR) {
			string r = (itR->second).getId();
			double lengthR = 0.0;
			// get edges of route r between start and end eges
			vector<string> edgesR;
			vector<string> edgeIds = staticRoutes[r].getEdgeIds();
			bool isMonitored = false;
			for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
				if (*it == endEdgeId) {
					isMonitored = false;
				}
				if (isMonitored) {
					lengthR += staticRecords[*it].getLength();
					edgesR.push_back(*it);
				}
				if (*it == startEdgeId) {
					isMonitored = true;
				}
			}

			double overlapSum = 0.0;
			for (std::map<std::string, Route>::iterator itS = alternatives.begin(); itS != alternatives.end(); ++itS) {
				string s = (itS->second).getId();
				if (staticRoutes.find(s) != staticRoutes.end()) {
					bool isMonitored = false;
					double lengthS = 0.0;
					double overlapLength = 0.0;
					vector<string> edgeIds = staticRoutes[s].getEdgeIds();
					for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
						if (*it == endEdgeId) {
							isMonitored = false;
						}
						if (isMonitored) {
							lengthS += staticRecords[*it].getLength();
							if (std::find(edgesR.begin(), edgesR.end(), *it) != edgesR.end()) {
								overlapLength += staticRecords[*it].getLength();
							}
						}
						if (*it == startEdgeId) {
							isMonitored = true;
						}
					}
					overlapSum += pow(overlapLength / sqrt(lengthR * lengthS), gamma);
				}
			}
			commonalities[r] = beta * log(overlapSum);
			Log::getInstance().getStream("c-logit") << "commonalities[" << r << "]=" << commonalities[r] << ", ";
		}
	}

	double probSum = 0;
	for (std::map<std::string, Route>::iterator itR = alternatives.begin(); itR != alternatives.end(); ++itR) {
		double weightedSum = 0.0;
		string r = (itR->second).getId();
		for (std::map<std::string, Route>::iterator itS = alternatives.begin(); itS != alternatives.end(); ++itS) {
			string s = (itS->second).getId();
			weightedSum += exp(theta * (costs[r] - costs[s] + commonalities[r] - commonalities[s]));
		}
		probabilities[r] = 1.0 / weightedSum;
		Log::getInstance().getStream("mnl") << "p[" << r << "]=" << probabilities[r] << ", ";
		probSum += probabilities[r];
	}
	Log::getInstance().getStream("mnl") << ". Sum: " << probSum << endl;

	return probabilities;
}

//vector<string> edgeList = vanetsKnowledge.getEdgesList(itRoutes->second, currentEdge, vehicle.getDestinationEdgeId());
//	map<string, double> vanetEdgesCosts = vanetsKnowledge.getEdgesCosts(edgeList, ttl, "vanet", "last"); // refer to the last call of vanetsKnowledge.getCosts !
//	map<string, double> perfectEdgesCosts = vanetsKnowledge.getEdgesCosts(edgeList, ttl, "perfect", "average"); // refer to the last call of vanetsKnowledge.getCosts !
//	map<string, double> tmcEdgesCosts = vanetsKnowledge.getEdgesCosts(edgeList, ttl, "perfect", "tmc"); // refer to the last call of vanetsKnowledge.getCosts !
//	map<string, double> staticEdgesCosts = vanetsKnowledge.getEdgesCosts(edgeList, ttl, "static", "last"); // refer to the last call of vanetsKnowledge.getCosts !
//
//	double error_vanet = 0;
//	double sumVanet = 0;
//	double sumPerfect = 0;
//	double sumTMC = 0;
//	double sumStatic = 0;
//	for (vector<string>::iterator it=edgeList.begin(); it!=edgeList.end(); ++it) {
//		error_vanet += abs(perfectEdgesCosts[*it] - vanetEdgesCosts[*it]);
//		sumPerfect += perfectEdgesCosts[*it];
//		sumVanet += vanetEdgesCosts[*it];
//		sumStatic += staticEdgesCosts[*it];
//		sumTMC += tmcEdgesCosts[*it];
//	}
//
//string endEdgeId = vehicle.getDestinationEdgeId();
//map<string,map<string,vector<string> > > correlated = vanetsKnowledge.analyseCorrelation(, currentEdgeId, endEdgeId);
////map<string,map<string,vector<string> > > correlated;
//map<string, double> correlatedValues;
//string defaultRoute = vehicle.getItinerary().getId();

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
		Log::getInstance().getStream("congestion_changes") <<  Simulator::Now().GetSeconds() << "\t" << this->congestion << "\t->\t" << congestion <<  "\t" << ifDense << "\t" << ifCongested << endl;
		this->congestion = congestion;
	}
}

}
