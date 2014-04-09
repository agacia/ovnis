/*
 * sumoTraciConnection.h
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#ifndef SUMOTRACICONNECTION_H_
#define SUMOTRACICONNECTION_H_

#include <iostream>

#define BUILD_TCPIP
//#include <foreign/tcpip/storage.h>
//#include <foreign/tcpip/socket.h>
#include "traci/storage.h"
#include "traci/socket.h"
#include "traci/query/simStepQuery.h"
#include "traci/query/simulationQuery.h"
#include <limits.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include "ns3/core-module.h"
#include "ns3/object.h"

namespace ovnis {

class SumoTraciConnection : public ns3::Object {

public:
	SumoTraciConnection();
	virtual ~SumoTraciConnection();

	// simulation
	void RunServer(std::string sumoConfig, std::string host, std::string sumoPath, int port, std::string outputFolder);
	void SubscribeSimulation(int startTime, int stopTime);
	void NextSimStep(std::vector<std::string> & departedVehicles, std::vector<std::string> & arrivedVehicles);
	const int GetCurrentTime() const;
	void Close();

	// vehicle
	double GetVehicleSpeed(std::string vehicleId);
    std::string GetVehicleEdge(std::string vehicleId);
    std::string GetVehicleLane(std::string vehicleId);
    double GetLaneLength(std::string laneId);
    double GetLaneMaxSpeed(std::string laneId);
    std::string GetVehicleRoute(std::string vehicleId);
    std::string GetVehicleRouteId(std::string vehicleId);
    std::vector<std::string> GetVehicleEdges(std::string vehicleId);
    void ChangeVehicleEdges(std::string vehicleId, std::vector<std::string> edges);
    double GetVehicleAngle(std::string vehicleId);
    Position2D GetVehiclePosition(std::string vehicleId);
    int GetVehicleCount(std::string vehicleId);
    // edge
    const double GetEdgeTravelTime(std::string edgeId);
	const double GetEdgeGlobalTravelTime(std::string edgeId);
	void CloseLane(std::string laneId);
	void CloseEdge(std::string edgeId);
	std::vector<double> GetSimulationBoundaries();
    pid_t pid;

protected:
    /**
	 * The configuration file for running SUMO
	 */
	std::string config;
    /**
	 * The host machine on which SUMO will run
	 */
    std::string host;
    /**
	 * The port number (network) on the host machine SUMO will run on
	 */
    int port;
    /**
	 * The system path where the SUMO executable is located
	 */
    std::string sumoPath;
    /**
	 *
	 */
    double boundaries[2];

    tcpip::Socket socket;

    int currentTime;

    SimStepQuery stepQuery;

    int StartSumo(std::string config, std::string sumoPath, std::string outputFolder);

};

} /* namespace ovnis */

#endif /* SUMOTRACICONNECTION_H_ */
