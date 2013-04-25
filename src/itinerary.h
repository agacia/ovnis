/*
 * Itinerary.h
 *
 *  Created on: Jan 7, 2013
 *      Author: agata
 */

#ifndef ITINERARY_H_
#define ITINERARY_H_

#include "route.h"

using namespace std;

namespace ovnis {

class Itinerary: public Route {
public:
	Itinerary();
	Itinerary(std::string id, std::string strRoute);
	Itinerary(std::string, std::vector<std::string> route);
	virtual ~Itinerary();

	virtual std::string printRoute();
	std::string printEdges();
	Edge & getCurrentEdge();
	void setCurrentEdge(std::string edgeId);
	Edge & getEdge(std::string edgeId);
	Edge & getLastEdge();
	std::map<std::string,Edge> & getEdges();
	void addEdge(std::string edgeId);
	double computeTravelTime(std::string startEdge, std::string endEdge);
	void fixTravelTime(std::string edgeId);
	double getStartTime();
private:

	std::map<std::string, Edge> edges;
	Edge * currentEdge;
    std::string lastEdgeId;

	void initializeEdges(std::vector<std::string> route);
};

} /* namespace ovnis */
#endif /* ITINERARY_H_ */
