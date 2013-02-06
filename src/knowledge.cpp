/*
 * knowledge.cpp
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#include "knowledge.h"

using namespace std;

namespace ovnis {

Knowledge::Knowledge() {
	// TODO Auto-generated constructor stub

}

Knowledge::~Knowledge() {
	// TODO Auto-generated destructor stub
}

int Knowledge::getNumberOfVehicles(std::string edgeId) {
	int vehs = 0;
	if (numberOfVehicles.find(edgeId) != numberOfVehicles.end()) {
		vehs = numberOfVehicles[edgeId];
	}
	return vehs;
}

int Knowledge::addNumberOfVehicles(std::string edgeId) {
	if (numberOfVehicles.find(edgeId) != numberOfVehicles.end()) {
		++numberOfVehicles[edgeId];
	}
	else {
		numberOfVehicles[edgeId] = 1;
	}
	return numberOfVehicles[edgeId];
}

int Knowledge::substractNumberOfVehicles(std::string edgeId) {
	if (numberOfVehicles.find(edgeId) != numberOfVehicles.end()) {
		numberOfVehicles[edgeId] = numberOfVehicles[edgeId]==0 ? 0 : --numberOfVehicles[edgeId];
	}
	return numberOfVehicles[edgeId];
}

void Knowledge::record(Data data) {
	// check if the time of hearing the packet is not too long
	if (Simulator::Now().GetSeconds() - data.date > PACKET_TTL) {
		return;
	}
	if (travelTimes.find(data.edgeId) == travelTimes.end() ||
			(travelTimes.find(data.edgeId) != travelTimes.end() && travelTimes[data.edgeId].getLatestTime() < data.date)) {
		if (data.date == 0) {
			cerr << "date ==0 !" << endl;
		}
		travelTimes[data.edgeId].add(0, "", data.date, data.travelTime);
	}
}

void Knowledge::record(vector<Data> data) {
	for (vector<Data>::iterator it = data.begin(); it != data.end(); ++it) {
		record(*it);
	}
}

map<string,RecordEntry> & Knowledge::getRecords()
{
	return travelTimes;
}

map<string, double> Knowledge::getCosts(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	map<string, double> costs;
	map<string, double> packetAges;

	double vehsOnRoute = 0;
	double now = Simulator::Now().GetSeconds();

	// update localKnowledge by reseting outdated packets
	int size = travelTimes.size();
	if (size > 0) {
		map<string,RecordEntry> localrecords = travelTimes;
		for (map<string, RecordEntry>::iterator it = localrecords.begin(); it != localrecords.end(); ++it) {
			double packetAge = it->second.getLatestTime() == 0 ? 0 : now - it->second.getLatestTime();
			if (packetAge > PACKET_TTL) {
				packetAge = 0;
				travelTimes[it->first].reset();
			}
		}
	}

	costs = computeTravelTimesOnRoutes(routes, startEdgeId, endEdgeId);
	return costs;
}

map<string, double> Knowledge::computeTravelTimesOnRoutes(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	map<string, double> costs;
	map<string, double> packetAges;
	map<string,int> vehicles;
	map<string,int> numberOfUpdatedEdges;
	double now = Simulator::Now().GetSeconds();

	// get static costs
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		costs[it->first] = TIS::getInstance().computeStaticCostExcludingMargins(it->first, startEdgeId, endEdgeId);
		packetAges[it->first] = 0;
		vehicles[it->first] = 0;
		numberOfUpdatedEdges[it->first] = 0;
	}

	// update costs from vanets records
	for (map<string, RecordEntry>::iterator it = travelTimes.begin(); it != travelTimes.end(); ++it) {
		string edgeId = it->first;
		double travelTime = it->second.getLatestValue();
		double packetDate = it->second.getLatestTime();
		int vehs = numberOfVehicles[it->first];
//		double travelTime = it->second.getAverageValue();
//		double packetDate = it->second.getAverageTime();

		double staticCost = 0;
		for (map<string, Route>::iterator itRoutes = routes.begin(); itRoutes != routes.end(); ++itRoutes) {
			if (itRoutes->second.containsEdgeExcludedMargins(edgeId, startEdgeId, endEdgeId)
					&& travelTime > 0
					& TIS::getInstance().getStaticRecords().find(edgeId) != TIS::getInstance().getStaticRecords().end()) {
				staticCost = TIS::getInstance().getStaticRecords()[edgeId].getStaticCost();
				staticCost = itRoutes->second.getEdgeInfo(edgeId).getStaticCost();
				costs[itRoutes->first] = costs[itRoutes->first] - staticCost + travelTime;
				packetAges[itRoutes->first] += (Simulator::Now().GetSeconds() - packetDate);
				++numberOfUpdatedEdges[itRoutes->first];
//				cout << "updating cost of route " << itRoutes->first << " on edge: " << it->first << ": staticCost:" << staticCost << ", actualTravelTime " << travelTime << ", " << (Simulator::Now().GetSeconds() - packetDate) << "," << vehs << endl;
			}
		}
	}
	// calculate the average information age
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		packetAges[it->first] = numberOfUpdatedEdges[it->first] == 0 ? 0 : packetAges[it->first] / numberOfUpdatedEdges[it->first];
	}

	Log::getInstance().getStream("vanet_costs") <<  now << "\t";
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		Log::getInstance().getStream("vanet_costs") << it->first << "," << costs[it->first] << "," << packetAges[it->first] << "," << numberOfUpdatedEdges[it->first] << "\t";
	}
	Log::getInstance().getStream("vanet_costs") << endl;

	return costs;
}




}
