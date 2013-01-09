/*
 * Itinerary.cpp
 *
 *  Created on: Jan 7, 2013
 *      Author: agata
 */

#include "itinerary.h"

namespace ovnis {

Itinerary::Itinerary():
		Route(), edges() {
	initializeEdges(vector<string>());
}

Itinerary::Itinerary(string id, string strRoute) :
	Route(id, strRoute), edges() {

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

Itinerary::Itinerary(string id, vector<string> route) :
		Route(id, route), edges() {

	initializeEdges(route);
}

Itinerary::~Itinerary() {
}

string Itinerary::print() {
	stringstream out;
	out << "route " << id << endl;
	out << "length\t" << length << endl;
	out << "staticCost\t" << staticCost << endl;
	out << "travelTime\t" << travelTime << endl;
	return out.str();
}


string Itinerary::printRoute() {
	stringstream out;
	bool isMonitored = false;
	for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
//		out << "current" << (*it) << "departedEdge" << departedEdge << "arrivalEdge" << arrivalEdge << endl;
		if ((*it) == departureEdgeId) {
			isMonitored = true;
		}
		if (isMonitored) {
					out << (*it) << "\t";
				}
		if (*it == getArrivalEdge()) {
			isMonitored = false;
		}
	}
	out << endl;
	return out.str();
}

void Itinerary::addEdge(string edgeId) {
	if (departureEdgeId == "") {
		departureEdgeId = edgeId;
	}
	edgeIds.push_back(edgeId);
	edges[edgeId] = Edge(edgeId);
	edgeInfos.push_back(EdgeInfo(edgeId));
	computeLength();
	computeStaticCost();
}

string Itinerary::printEdges() {
	stringstream out;
	bool isMonitored = false;
	for (vector<EdgeInfo>::iterator it = edgeInfos.begin(); it != edgeInfos.end(); ++it) {
		if (it->getId() == departureEdgeId) {
			isMonitored = true;
		}
		if (it->getId().compare(getArrivalEdge())==0) {
			isMonitored = false;
		}
		if (isMonitored) {
			out << it->print() << endl;
		}
	}
	return out.str();
}

void Itinerary::initializeEdges(vector<string> routeVector) {
	this->edgeIds.clear();
	this->edgeInfos = vector<EdgeInfo>();
	departureEdgeId = "";
	currentEdge = new Edge();
	arrivalEdgeId = "";
	for (int i = 0; i < routeVector.size(); ++i) {
		edges[routeVector[i]] = Edge(routeVector[i]);
		this->edgeIds.push_back(routeVector[i]);
		edgeInfos.push_back(EdgeInfo(routeVector[i]));
	}
	if (routeVector.size() > 0) {
		departureEdgeId = routeVector[0];
		currentEdge = &edges.find(departureEdgeId)->second;
		arrivalEdgeId = routeVector[routeVector.size()-1];
		computeLength();
		computeStaticCost();
	}
}

Edge & Itinerary::getCurrentEdge() {
	return *currentEdge;
}

void Itinerary::setCurrentEdge(string edgeId) {
	if (edgeId.length() != 0) {
		this->currentEdge = &edges.find(edgeId)->second;
	}
}

Edge & Itinerary::getEdge(string edgeId) {
	map<string,Edge>::iterator it = edges.find(edgeId);
	if (it != edges.end()) {
		return (*it).second;
	}
	Edge edge = Edge();
	return edge;
}

double Itinerary::computeLength() {
	return Route::computeLength(departureEdgeId, arrivalEdgeId);
}

double Itinerary::computeStaticCost() {
	return Route::computeStaticCost(departureEdgeId, arrivalEdgeId);
}

double Itinerary::computeTravelTime() {
	return computeTravelTime(departureEdgeId, arrivalEdgeId);
}

double Itinerary::computeTravelTime(string startEdgeId, string endEdgeId) {
	travelTime = 0;
	if (edgeIds.size() > 0) {
		Edge & startEdge = edges.find(startEdgeId)->second;
		Edge & endEdge = edges.find(endEdgeId)->second;
		if (startEdge.getEnteredTime() == 0) {
			fixTravelTime(startEdgeId);
			cout << "fixing startEdge time" << endl;
		}
		if (endEdge.getEnteredTime() == 0) {
			fixTravelTime(endEdgeId);
		}
		travelTime = edges.find(endEdgeId)->second.getEnteredTime() - edges.find(startEdgeId)->second.getEnteredTime();
//		cout << printEdges();
//		cout << id << " " <<  travelTime << " start edge: " << startEdgeId << " " << startEdge.getEnteredTime() << ", end edge: " << endEdgeId << " " << endEdge.getEnteredTime() << endl;
	}
	return travelTime;
}

void Itinerary::fixTravelTime(string edgeId) {
	// find the untraversed edge in the route
	int index = 0;
	int i = 0;
	for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
		if (*it == edgeId) {
			index = i;
			break;
		}
		++i;
	}
//	cout << "fixing on route " << id << " edge:" << edgeId << endl;
	// if edge is found
	if (i != edgeIds.size()) {
		// if untraversed edge is first
		if (i == 0) {
			edges.find(edgeId)->second.setEnteredTime(departureTime); // normal that it's 0
			edges.find(edgeId)->second.setLeftTime(edges.find(edgeIds[i+1])->second.getEnteredTime());
		}
		// if untraversed edge is the last
		if (i == edgeIds.size() -1) {
			if (edges.find(edgeIds[i-1])->second.getLeftTime() != 0) { // fix only when the length of the edge was too short and the edge before before has been traversed
				edges.find(edgeId)->second.setLeftTime(arrivalTime);
			}
			edges.find(edgeId)->second.setEnteredTime(edges.find(edgeIds[i-1])->second.getLeftTime());
		}

		if (i > 0 && i < edgeIds.size() -1) {
			double enterTime = edges.find(edgeIds[i-1])->second.getLeftTime();
			double leftTime = edges.find(edgeIds[i+1])->second.getEnteredTime();
			edges.find(edgeId)->second.setLeftTime(leftTime);
			edges.find(edgeId)->second.setEnteredTime(enterTime);
		}
	}
}

double Itinerary::getTravelTime() {
	return this->travelTime;
}

void Itinerary::setDepartedEdge(std::string edgeId) {
	this->departureEdgeId = edgeId;
}
void Itinerary::setArrivalEdge(std::string edgeId) {
	this->arrivalEdgeId = edgeId;
	//computeLength();
	//computeStaticCost();
}

string Itinerary::getDepartedEdge() {
	return edges.find(departureEdgeId)->second.getId();
}

string Itinerary::getArrivalEdge() {
	map<string,Edge>::iterator it = edges.find(arrivalEdgeId);
	if (it == edges.end() && edgeIds.size() > 0) {
		return edgeIds[edgeIds.size()-1];
	}
	else {
		return arrivalEdgeId;
	}
}

double Itinerary::getDepartedEdgeEnterTime() {
	return edges.find(departureEdgeId)->second.getEnteredTime();
}

double Itinerary::getArrivalEdgeLeftTime() {
	return edges.find(arrivalEdgeId)->second.getLeftTime();
}

double Itinerary::getDepartedTime() {
	return this->departureTime;
}

double Itinerary::getArrivalTime() {
	return this->arrivalTime;
}

void Itinerary::setDepartedTime(double time) {
	this->departureTime = time;
}

void Itinerary::setArrivalTime(double time) {
	this->arrivalTime = time;
}


} /* namespace ovnis */
