/*
 * Itinerary.cpp
 *
 *  Created on: Jan 7, 2013
 *      Author: agata
 */

#include "itinerary.h"


using namespace std;

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

string Itinerary::printRoute() {
	stringstream out;
	for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
			out << (*it) << "\t";
	}
	out << endl;
	return out.str();
}

void Itinerary::addEdge(string edgeId) {
	lastEdgeId = currentEdge->getId();
	edgeIds.push_back(edgeId);
	edges[edgeId] = Edge(edgeId);
	edgeInfos.push_back(EdgeInfo(edgeId));
}

string Itinerary::printEdges() {
	stringstream out;
	bool isMonitored = false;
	for (vector<EdgeInfo>::iterator it = edgeInfos.begin(); it != edgeInfos.end(); ++it) {
		out << it->print() << endl;
	}
	return out.str();
}

void Itinerary::initializeEdges(vector<string> routeVector) {
	this->edgeIds.clear();
	this->edgeInfos = vector<EdgeInfo>();
	currentEdge = new Edge();
	for (int i = 0; i < routeVector.size(); ++i) {
		edges[routeVector[i]] = Edge(routeVector[i]);
		this->edgeIds.push_back(routeVector[i]);
		edgeInfos.push_back(EdgeInfo(routeVector[i]));
	}
	if (routeVector.size() > 0) {
		currentEdge = &edges.find(edgeIds[0])->second;
		computeLength();
		computeStaticCost();
	}
}

Edge & Itinerary::getCurrentEdge() {
	return *currentEdge;
}

Edge & Itinerary::getLastEdge() {
	return edges.find(lastEdgeId)->second;
}

void Itinerary::setCurrentEdge(string edgeId) {
	if (edgeId.length() != 0) {
		this->currentEdge = &edges.find(edgeId)->second;
	}
}

std::map<std::string,Edge> & Itinerary::getEdges() {
	return edges;
}

Edge & Itinerary::getEdge(string edgeId) {
	Edge edge;
	map<string,Edge>::iterator it = edges.find(edgeId);
	if (it != edges.end()) {
		edge = (*it).second;
	}
	return edge;
}

double Itinerary::computeTravelTime(string startEdgeId, string endEdgeId) {
	double travelTime = 0;
	if (edgeIds.size() > 0) {
		Edge & startEdge = edges.find(startEdgeId)->second;
		Edge & endEdge = edges.find(endEdgeId)->second;
		if (startEdge.getLeftTime() == 0) {
			fixTravelTime(startEdgeId);
		}
		if (endEdge.getEnteredTime() == 0) {
			fixTravelTime(endEdgeId);
		}
		travelTime = edges.find(endEdgeId)->second.getEnteredTime() - edges.find(startEdgeId)->second.getLeftTime();
//		cout << "travel time on " << id << " " <<  travelTime << " [" << startEdgeId << " " << "," << endEdgeId << "]" << endl;
	}
	return travelTime;
}

/**
 * find the untraversed edge in the route (in case edge is too short for SUMO to record
 */
void Itinerary::fixTravelTime(string edgeId) {
	int index = 0;
	int i = 0;
	for (vector<string>::iterator it = edgeIds.begin(); it != edgeIds.end(); ++it) {
		if (*it == edgeId) {
			index = i;
			break;
		}
		++i;
	}
	// if edge is found
	if (i != edgeIds.size()) {
		// if untraversed edge is first
		if (i == 0 && edgeIds.size() > 1) {
			cerr << "fixing first edge on route " << id << " edge:" << edgeId << endl;
//			edges.find(edgeId)->second.setEnteredTime(departureTime); // normal that it's 0
			edges.find(edgeId)->second.setLeftTime(edges.find(edgeIds[i+1])->second.getEnteredTime());
		}
		// if untraversed edge is the last
		if (i == edgeIds.size() -1) {
			if (edges.find(edgeIds[i-1])->second.getLeftTime() != 0) { // fix only when the length of the edge was too short and the edge before before has been traversed
				cerr << "the last edge was not traversed -- too short";
//				edges.find(edgeId)->second.setLeftTime(arrivalTime);
			}
			if (edges.find(edgeId)->second.getEnteredTime() == 0) {
				cerr << " nie moze byc 0 " << endl;
				cerr << "dajemy wiec z poprzedniej krawedzi " << edges.find(edgeIds[i-1])->second.getLeftTime();
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

} /* namespace ovnis */
