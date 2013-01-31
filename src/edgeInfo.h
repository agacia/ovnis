/*
 * edge.h
 *
 *  Created on: Aug 28, 2012
 *      Author: agata
 */

#ifndef EDGE_INFO_H_
#define EDGE_INFO_H_

#include <cstdlib>
#include <iostream>

#include "ns3/ptr.h"
#include "traci/sumoTraciConnection.h"
#include <traci-server/TraCIConstants.h>

namespace ovnis {

class EdgeInfo {
public:
	EdgeInfo();
	EdgeInfo(std::string id);
	virtual ~EdgeInfo();
	std::string print();
	std::string getId();
	std::string getLaneId();
	double getLength();
	double getMaxSpeed();
//	double getLeftTime();
	void setLaneId(std::string laneId);
	void setLength(double length);
	void setMaxSpeed(double maxSpeed);
	double getCurrentTravelTime();
	double requestCurrentTravelTime();
	double getStaticCost();

private:
    ns3::Ptr<ovnis::SumoTraciConnection> traci;
	std::string id;
	std::string laneId;
	double length;
	double maxSpeed;
	double staticCost;
	double currentTravelTime;
};

} /* namespace ovnis */
#endif /* EDGE_INFO_H_ */
