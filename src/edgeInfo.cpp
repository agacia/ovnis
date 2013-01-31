/*
 * edgeInfo.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: agata
 */

#include "edgeInfo.h"
#include <sstream>
#include <string>

namespace ovnis {

using namespace std;
using namespace ns3;

EdgeInfo::EdgeInfo() :
	id(""), laneId(""), length(0), maxSpeed(0) {
	traci = Names::Find<ovnis::SumoTraciConnection>("SumoTraci");
}

EdgeInfo::EdgeInfo(string edgeId) :
	id(edgeId) {
	traci = Names::Find<ovnis::SumoTraciConnection>("SumoTraci");
	string laneId = edgeId + "_0";
	try {
		maxSpeed = traci->GetLaneMaxSpeed(laneId);
		length = traci->GetLaneLength(laneId);
		currentTravelTime = traci->GetEdgeTravelTime(edgeId);
		if (maxSpeed > 0) {
			staticCost = length / maxSpeed;
		}
		else {
			// XXX If maxSpeed is set to 0 (simulated accident), then return the expected travel time with the avg speed of 25m/s for calculating expected costs f the whole route (we assume we don't know any information)
			staticCost = length / 25;
		}
//		cout << edgeId << ": " << staticCost << "(" << length <<"/" << maxSpeed << ")" << endl;
	}
	catch (TraciException &e) { }
}

EdgeInfo::~EdgeInfo() {
}

string EdgeInfo::print() {
	stringstream out;
	out << id << "\t" << laneId << "\t" << length << "\t" << maxSpeed << "\t" << (length/maxSpeed) << "\t" << currentTravelTime << "\t";
	return out.str();
}

std::string EdgeInfo::getId() {
	return id;
}

std::string EdgeInfo::getLaneId() {
	return laneId;
}

double EdgeInfo::getLength() {
	return length;
}

double EdgeInfo::getMaxSpeed() {
	return maxSpeed;
}

double EdgeInfo::getStaticCost() {
	return staticCost;
}

double EdgeInfo::requestCurrentTravelTime() {
	currentTravelTime = traci->GetEdgeTravelTime(id);
	return currentTravelTime;
}

double EdgeInfo::getCurrentTravelTime() {
	return currentTravelTime;
}


} /* namespace ovnis */
