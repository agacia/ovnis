/*
 * route.h
 *
 *  Created on: Aug 28, 2012
 *      Author: agata
 */

#ifndef ROUTE_H_
#define ROUTE_H_

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "edge.h"
#include "edgeInfo.h"

using namespace std;

namespace ovnis {

class Route {

public:
	Route();
	Route(std::string id, std::string route);
	Route(std::string, std::vector<std::string> route);
	virtual ~Route();
	void initializeEdges(std::string routeStr);
	void initializeEdges(std::vector<std::string> routeVector);

	std::string getId();
	void setId(std::string id);

	std::string print();
	virtual std::string printRoute();

	std::vector<std::string> & getEdgeIds();
	std::vector<EdgeInfo> & getEdgeInfos();
	bool containsEdge(string edgeId);
	bool containsEdgeExcludedMargins(string edgeId, string startEdgeId, string endEdgeId);
	double getEdgeMaxSpeed(std::string edgeId);
	double getCapacity();
	void setCapacity(double capacity);
	virtual double computeLength();
	double computeLengthExcludingMargins(string startEdgeId, string endEdgeId);
	double computeLength(std::string startEdgeId, std::string endEdgeId);
	virtual double computeStaticCost();
	void updateCurrentTravelTime();
	double computeStaticCost(std::string startEdge, std::string endEdge);
	double computeStaticCostExcludingMargins(std::string startEdgeId, std::string endEdgeId);
	double getStaticCost();
	double getLength();
	EdgeInfo getEdgeInfo(std::string edgeId);

protected:
	std::string id;
	std::vector<std::string> edgeIds;
	std::vector<EdgeInfo> edgeInfos;
	double staticCost; // time = length / maxSpeed
	double length;
	double capacity;
};

} /* namespace ovnis */
#endif /* ROUTE_H_ */
