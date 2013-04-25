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
	congestionThreshold = CONGESTION_THRESHOLD;
	densityThreshold = DENSITY_THRESHOLD;
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

void Knowledge::analyseLocalDatabase(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	ifCongestedFlow = false;
	ifDenseFlow = false;
	double sumLength = 0;
	double congestedLength = 0;
	double denseLength = 0;
	double sumDelay = 0;
	int totalNumberOfUpdatedEdges = 0;
	map<string,int> vehicles;
	travelTimesOnRoutes = map<string,double>();
	packetAgesOnRoutes = map<string,double>();
	delayOnRoutes = map<string,double>();
	congestedLengthOnRoutes = map<string,double>();
	denseLengthOnRoutes = map<string,double>();
	// initialize
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		travelTimesOnRoutes[it->first] = TIS::getInstance().computeStaticCostExcludingMargins(it->first, startEdgeId, endEdgeId);
		packetAgesOnRoutes[it->first] = 0;
		vehicles[it->first] = 0;
		numberOfUpdatedEdges[it->first] = 0;
		congestedLengthOnRoutes[it->first] = 0;
		denseLengthOnRoutes[it->first] = 0;
		delayOnRoutes[it->first] = 0;
	}
	// read vanets records
	for (map<string, RecordEntry>::iterator it = travelTimes.begin(); it != travelTimes.end(); ++it) {
		string edgeId = it->first;
		double travelTime = it->second.getLatestValue();
		double packetDate = it->second.getLatestTime();
		double packetAge = packetDate == 0 ? 0 : Simulator::Now().GetSeconds() - packetDate;
		int vehs = numberOfVehicles[it->first];
//		double travelTime = it->second.getAverageValue();
//		double packetDate = it->second.getAverageTime();
		double staticCost = 0;
		for (map<string, Route>::iterator itRoutes = routes.begin(); itRoutes != routes.end(); ++itRoutes) {
			if (itRoutes->second.containsEdgeExcludedMargins(edgeId, startEdgeId, endEdgeId)) { // if the edge belongs to the part of the future route
				if (travelTime > 0 && packetAge < maxInformationAge) { // information was recorded about this edge and is fresh enough
					staticCost = TIS::getInstance().getEdgeStaticCost(edgeId);
					if (staticCost != 0) {
						it->second.setCapacity(staticCost);
					}
					travelTimesOnRoutes[itRoutes->first] = travelTimesOnRoutes[itRoutes->first] - staticCost + travelTime;
					packetAgesOnRoutes[itRoutes->first] += packetAge;
					++numberOfUpdatedEdges[itRoutes->first];
					++totalNumberOfUpdatedEdges;
					double edgeLength = TIS::getInstance().getEdgeLength(edgeId);
					sumLength += edgeLength;

					Log::getInstance().getStream("capacity") << Simulator::Now().GetSeconds() << "\t";

					// sumo bug fix
					if (travelTime != SIMULATION_STEP_INTERVAL || it->second.getExpectedValue() > SIMULATION_STEP_INTERVAL) { // travelTime > 1 means that it was recorded in sumo!
						double delay = travelTime - it->second.getExpectedValue();
						if (delay > 0) {
							delayOnRoutes[itRoutes->first] += delay;
							sumDelay += delay;
						}
						Log::getInstance().getStream("vanets_knowledge") << edgeId << "," << packetDate << "," << travelTime << "\t";
						Log::getInstance().getStream("delay") << Simulator::Now().GetSeconds() << "\t" << edgeId << "\t" << travelTime << "\t-\t" << it->second.getExpectedValue() << "\t=\t" << delay << "\t";
						Log::getInstance().getStream("capacity") << edgeId << "," << it->second.getExpectedValue() << "," << travelTime << "," << it->second.getActualCapacity() << "\t";

						if (it->second.getActualCapacity() < congestionThreshold) {
							congestedLength += edgeLength;
							ifCongestedFlow = true;
							congestedLengthOnRoutes[itRoutes->first] += edgeLength;
							Log::getInstance().getStream("congestion") << Simulator::Now().GetSeconds() << "\t" << edgeId << "\t" << it->second.getActualCapacity() << "<" << congestionThreshold << "\t" << travelTime << "\t" << it->second.getExpectedValue() << endl;
						}
						if (it->second.getActualCapacity() > congestionThreshold && it->second.getActualCapacity() < densityThreshold) {
							ifDenseFlow = true;
							denseLengthOnRoutes[itRoutes->first] += edgeLength;
							Log::getInstance().getStream("dense") << Simulator::Now().GetSeconds() << "\t" << edgeId << "\t" << congestionThreshold << "<" << it->second.getActualCapacity() << "<" << densityThreshold << "\t" << travelTime << "\t" << it->second.getExpectedValue() << endl;
						}
					}
				}
			}
		}
	}
	// calculate the average information age
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		packetAgesOnRoutes[it->first] = numberOfUpdatedEdges[it->first] == 0 ? 0 : packetAgesOnRoutes[it->first] / numberOfUpdatedEdges[it->first];
	}
	// print knowledge
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		packetAgesOnRoutes[it->first] = numberOfUpdatedEdges[it->first] == 0 ? 0 : packetAgesOnRoutes[it->first] / numberOfUpdatedEdges[it->first];
		if (totalNumberOfUpdatedEdges > 0) {
			Log::getInstance().getStream("vanets_knowledge") << it->first << "," << numberOfUpdatedEdges[it->first] << "," << it->second.countEdgesExcludedMargins(startEdgeId, endEdgeId) << "\t";
		}
	}
	Log::getInstance().getStream("vanets_knowledge") << endl;
}

map<string,double> Knowledge::getTravelTimesOnRoutes() {
	return travelTimesOnRoutes;
}

map<string,double> Knowledge::getDelayOnRoutes() {
	return delayOnRoutes;
}

map<string,double> Knowledge::getCongestedLengthOnRoutes() {
	return congestedLengthOnRoutes;
}

map<string,double> Knowledge::getDenseLengthOnRoutes() {
	return denseLengthOnRoutes;
}

//double Knowledge::getSumDelay() {
//	return sumDelay;
//}

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

    bool Knowledge::isCongestedFlow() const
    {
        return ifCongestedFlow;
    }

    bool Knowledge::isDenseFlow() const
    {
        return ifDenseFlow;
    }

}
