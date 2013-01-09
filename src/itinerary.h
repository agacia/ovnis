/*
 * Itinerary.h
 *
 *  Created on: Jan 7, 2013
 *      Author: agata
 */

#ifndef ITINERARY_H_
#define ITINERARY_H_

#include "route.h"

namespace ovnis {

class Itinerary: public ovnis::Route {
public:
	Itinerary();
	Itinerary(std::string id, std::string strRoute);
	Itinerary(std::string, std::vector<std::string> route);
	virtual ~Itinerary();

	std::string print();
	virtual std::string printRoute();
	std::string printEdges();

	Edge & getCurrentEdge();
	void setCurrentEdge(std::string edgeId);
	Edge & getEdge(std::string edgeId);
	std::map<std::string,Edge> & getEdges();
	void addEdge(std::string edgeId);

	virtual double computeLength();
	virtual double computeStaticCost();
	double computeTravelTime();
	double computeTravelTime(std::string startEdge, std::string endEdge);
	double getTravelTime();

	std::string getDepartedEdge();
	std::string getArrivalEdge();
	void setDepartedEdge(std::string edgeId);
	void setArrivalEdge(std::string edgeId);
	double getDepartedTime();
	double getDepartedEdgeEnterTime();
	double getArrivalEdgeLeftTime();
	double getArrivalTime();
	void setDepartedTime(double time);
	void setArrivalTime(double time);

private:

	std::map<std::string, Edge> edges;
	Edge * currentEdge;
	std::string currentEdgeId;
	double travelTime;
	std::string departureEdgeId;
	std::string arrivalEdgeId;
	double departureTime;
	double arrivalTime;

	void initializeEdges(std::vector<std::string> route);
	void fixTravelTime(std::string edgeId);
};

} /* namespace ovnis */
#endif /* ITINERARY_H_ */
