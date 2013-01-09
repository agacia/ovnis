/*
 * route.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: agata
 */

#include "route.h"
#include <sstream>
#include <vector>
#include <map>

namespace ovnis {

using namespace std;

Route::Route() :
	edgeIds(), edgeInfos(), staticCost(0) {
}

Route::Route(string id, string strRoute) :
	id(id), edgeIds(), edgeInfos(), staticCost(0) {
	vector<string> readRoute;
	if (strRoute.size() > 0) {
		stringstream ss(strRoute);
		string edgeId;
		while (ss >> edgeId) {
			readRoute.push_back(edgeId);
		}
		initializeEdges(readRoute);
	}
}

Route::Route(string id, vector<string> route) :
		id(id), edgeIds(), edgeInfos(), staticCost(0) {
	initializeEdges(route);
}

void Route::initializeEdges(vector<string> routeVector) {
	this->edgeIds.clear();
	this->edgeInfos = vector<EdgeInfo>();
	for (int i = 0; i < routeVector.size(); ++i) {
		edgeIds.push_back(routeVector[i]);
		edgeInfos.push_back(EdgeInfo(routeVector[i]));
	}
	if (routeVector.size() > 0) {
		computeLength();
		computeStaticCost();
	}
}

void Route::updateCurrentTravelTime() {
	for (vector<EdgeInfo>::iterator it = edgeInfos.begin(); it != edgeInfos.end(); ++it) {
		it->requestCurrentTravelTime();
	}
}

Route::~Route() {
}

string Route::getId() {
	return this->id;
}

void Route::setId(string id) {
	this->id = id;
}

string Route::print() {
	stringstream out;
	out << "route " << id << endl;
	out << "length\t" << length << endl;
	out << "staticCost\t" << staticCost << endl;
	return out.str();
}

string Route::printRoute() {
	stringstream out;
	bool isMonitored = false;
	for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
		out << (*it) << "\t";
	}
	out << endl;
	return out.str();
}

vector<string> & Route::getRoute() {
	return edgeIds;
}

double Route::getCapacity() {
	return capacity;
}

void Route::setCapacity(double capacity) {
	this->capacity = capacity;
}

double Route::getStaticCost() {
	return this->staticCost;
}
double Route::getLength() {
	return this->length;
}

bool Route::containsEdge(string edgeId) {
	for (vector<EdgeInfo>::iterator it = edgeInfos.begin(); it != edgeInfos.end(); ++it) {
		if (it->getId() == edgeId) {
			return true;
		}
	}
	return false;
}

double Route::computeLength() {
	if (edgeIds.size() > 0) {
		return computeLength(edgeIds[0], edgeIds[edgeIds.size()-1]);
	}
	else {
		return 0;
	}
}

double Route::computeLength(string startEdgeId, string endEdgeId) {
	length = 0;
	if (edgeIds.size() > 0) {
		bool isMonitored = false;
		for (vector<EdgeInfo>::iterator it = edgeInfos.begin(); it != edgeInfos.end(); ++it) {
			if (it->getId() == startEdgeId) {
				isMonitored = true;
			}
			if (it->getId() == endEdgeId) {
				isMonitored = false;
			}
			if (isMonitored) {
				length += it->getLength();
			}
		}
	}
	return length;
}

EdgeInfo Route::getEdgeInfo(std::string edgeId) {
	for (vector<EdgeInfo>::iterator it = edgeInfos.begin(); it != edgeInfos.end(); ++it) {
		if (it->getId() == edgeId) {
			return *it;
		}
	}
	return EdgeInfo();
}

double Route::getEdgeMaxSpeed(std::string edgeId) {
	double speed = 0;
	for (vector<EdgeInfo>::iterator it = edgeInfos.begin(); it != edgeInfos.end(); ++it) {
		if (it->getId() == edgeId) {
			speed = it->getMaxSpeed();
		}
	}
	return speed;
}
double Route::computeStaticCost() {
	if (edgeIds.size() > 0) {
			return computeStaticCost(edgeIds[0], edgeIds[edgeIds.size()-1]);
		}
		else {
			return 0;
		}
}

double Route::computeStaticCost(string startEdgeId, string endEdgeId) {
	staticCost = 0;
	if (edgeIds.size() > 0) {
		bool isMonitored = false;
//		cout <<  endl;
		for (vector<EdgeInfo>::iterator it = edgeInfos.begin(); it != edgeInfos.end(); ++it) {
			if (it->getId() == startEdgeId) {
				isMonitored = true;
			}
			if (it->getId() == endEdgeId) {
				isMonitored = false;
			}
			if (isMonitored) {
//				cout << it->getId() << ":" << it->getLength() << "," << it->getMaxSpeed() << " ";
				double maxSpeed = it->getMaxSpeed();
				if (maxSpeed > 0) {
					staticCost += it->getLength() / it->getMaxSpeed();
				}
			}
		}
	}
//	cout << " static cost: " << staticCost << endl;
	return staticCost;
}

} /* namespace ovnis */
