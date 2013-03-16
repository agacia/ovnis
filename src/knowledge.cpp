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
	maxInformationAge = PACKET_TTL;
}

Knowledge::~Knowledge() {
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

bool Knowledge::record(Data data) {
	// record only information that is fresher than the last heard
	if (travelTimes.find(data.edgeId) == travelTimes.end() ||
			(travelTimes.find(data.edgeId) != travelTimes.end() && travelTimes[data.edgeId].getLatestTime() < data.date)) {
//		if (data.edgeId == "main_3b") {
//			Log::getInstance().getStream("main_3b_recording") << data.travelTime << "\tfrom date\t" << data.date;
//		}
		travelTimes[data.edgeId].add(0, "", data.date, data.travelTime);
		return true;
	}
//	if (data.edgeId == "main_3b") {
//		Log::getInstance().getStream("main_3b_recording") << data.travelTime << "\tfrom date\t" << data.date << "\t there is fresher:\t" << travelTimes[data.edgeId].getLatestTime() << "\t" << travelTimes[data.edgeId].getLatestValue();
//	}
	return false;
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
	map<string,int> numberOfUpdatedEdges;
	double now = Simulator::Now().GetSeconds();
	map<string,int> vehicles;

	// get static costs
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		costs[it->first] = TIS::getInstance().computeStaticCostExcludingMargins(it->first, startEdgeId, endEdgeId);
		packetAges[it->first] = 0;
		vehicles[it->first] = 0;
		numberOfUpdatedEdges[it->first] = 0;
	}

	int totalNumberOfUpdatedEdges = 0;
	// update costs from vanets records
	for (map<string, RecordEntry>::iterator it = travelTimes.begin(); it != travelTimes.end(); ++it) {
		string edgeId = it->first;
		double travelTime = it->second.getLatestValue();
		double packetDate = it->second.getLatestTime();
		double packetAge = packetDate == 0 ? 0 : now - packetDate;
		int vehs = numberOfVehicles[it->first];
//		double travelTime = it->second.getAverageValue();
//		double packetDate = it->second.getAverageTime();

		double staticCost = 0;
		for (map<string, Route>::iterator itRoutes = routes.begin(); itRoutes != routes.end(); ++itRoutes) {
			if (itRoutes->second.containsEdgeExcludedMargins(edgeId, startEdgeId, endEdgeId)) { // if the edge belongs to the part of the future route
				if (travelTime > 0 && packetAge < maxInformationAge) {// information was recorded about this edge and is fresh enough
					staticCost = TIS::getInstance().getEdgeStaticCost(edgeId);
					if (staticCost != 0) {
						it->second.setCapacity(staticCost);
					}
					costs[itRoutes->first] = costs[itRoutes->first] - staticCost + travelTime;
					packetAges[itRoutes->first] += packetAge;
					++numberOfUpdatedEdges[itRoutes->first];
					++totalNumberOfUpdatedEdges;
					Log::getInstance().getStream("vanets_knowledge") << edgeId << "," << packetDate << "," << travelTime << "\t";
				}
			}
		}
	}

	// calculate the average information age
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		packetAges[it->first] = numberOfUpdatedEdges[it->first] == 0 ? 0 : packetAges[it->first] / numberOfUpdatedEdges[it->first];
		if (totalNumberOfUpdatedEdges > 0) {
			Log::getInstance().getStream("vanets_knowledge") << it->first << "," << numberOfUpdatedEdges[it->first] << "," << it->second.countEdgesExcludedMargins(startEdgeId, endEdgeId) << "\t";
		}
	}

	Log::getInstance().getStream("vanets_knowledge") << endl;

	Log::getInstance().getStream("vanet_costs") <<  now << "\t";
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		Log::getInstance().getStream("vanet_costs") << it->first << "," << costs[it->first] << "," << packetAges[it->first] << "," << TIS::getInstance().getVehiclesOnRoute(it->first) << "\t";

	}
	Log::getInstance().getStream("vanet_costs") << endl;

	return costs;
}

map<string,double> Knowledge::getCongestedLengthOnRoutes() {
	return congestedLengthOnRoutes;
}

/**
 * Tries to assess the severity of congestion.
 * Look up in recorded database and compute the actual capacity of each edge (the lower capacity (0-1) the more congested edge
 */
bool Knowledge::isCapacityDrop(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	bool isCapacityDrop = false;
	double now = Simulator::Now().GetSeconds();
	double sumLength = 0;
	double congestedLength = 0;
	congestedLengthOnRoutes = map<string,double>();
	for (map<string, Route>::iterator itRoutes = routes.begin(); itRoutes != routes.end(); ++itRoutes) {
		congestedLengthOnRoutes[itRoutes->first] = 0;
	}
	// assess capacity drops
	Log::getInstance().getStream("capacity") << now << "\t";
	for (map<string, RecordEntry>::iterator it = travelTimes.begin(); it != travelTimes.end(); ++it) {
		string edgeId = it->first;
		double travelTime = it->second.getLatestValue();
		double packetDate = it->second.getLatestTime();
		double packetAge = packetDate == 0 ? 0 : now - packetDate;
		for (map<string, Route>::iterator itRoutes = routes.begin(); itRoutes != routes.end(); ++itRoutes) {
			if (itRoutes->second.containsEdgeExcludedMargins(edgeId, startEdgeId, endEdgeId) && // if the edge belongs to the part of the future route
				travelTime > 0 && // no information was ever recorded about this edge
				packetAge < maxInformationAge) { // if the information is fresh enough
				double edgeLength = TIS::getInstance().getEdgeLength(edgeId);
				sumLength += edgeLength;
				if (it->second.getActualCapacity() < CAPACITY_THRESHOLD) {
					congestedLength += edgeLength;
					isCapacityDrop = true;
					congestedLengthOnRoutes[itRoutes->first] += edgeLength;
				}
				Log::getInstance().getStream("capacity") << edgeId << "," << it->second.getExpectedValue() << "," << travelTime << "," << it->second.getActualCapacity() << "\t";
			}
		}
	}
	Log::getInstance().getStream("congestion_scale") << now << "\t" << congestedLength << "\t" << sumLength << "\t";
	for (map<string, double>::iterator itRoutes = congestedLengthOnRoutes.begin(); itRoutes != congestedLengthOnRoutes.end(); ++itRoutes) {
		Log::getInstance().getStream("congestion_scale") << itRoutes->first << "," << itRoutes->second << "\t";
	}
	Log::getInstance().getStream("congestion_scale") << endl;

	return isCapacityDrop;
}


map<std::string,double> Knowledge::getEdgesCosts(map<string, Route> routes, string startEdgeId, string endEdgeId)
{
	map<std::string, double> edgesCosts;//edgesCosts = map<string, double> ();
	for (map<string, EdgeInfo>::iterator it = TIS::getInstance().getStaticRecords().begin(); it != TIS::getInstance().getStaticRecords().end(); ++it) {
		for (map<string, Route>::iterator itRoutes = routes.begin(); itRoutes != routes.end(); ++itRoutes) {
			if (itRoutes->second.containsEdgeExcludedMargins(it->first, startEdgeId, endEdgeId)) {
				if (edgesCosts.find(it->first) == edgesCosts.end()) {
					double travelTime = travelTimes[it->first].getLatestValue();
					double packetAge = travelTimes[it->first].getLatestTime() == 0 ? 0 : Simulator::Now().GetSeconds() - travelTimes[it->first].getLatestTime();
					if (travelTime > 0 && packetAge < maxInformationAge) { // if the information is fresh enough
						edgesCosts[it->first] = travelTime;
					}
					else {
						edgesCosts[it->first] = it->second.getStaticCost();
					}
				}
			}
		}
	}
    return edgesCosts;
}

}
