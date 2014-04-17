/*
 * sumoTraciConnection.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */
#include <iostream>
#include <sstream>
#include <fstream>

#include "traci/sumoTraciConnection.h"
#include "traci/query/simStepQuery.h"
#include "traci/query/subscribeQuery.h"
#include "traci/query/vehicleQuery.h"
#include "traci/query/edgeQuery.h"
#include "traci/query/laneQuery.h"
#include "traci/query/closeQuery.h"
#include "xml-sumo-conf-parser.h"
#include "log.h"

using namespace std;
using namespace tcpip;

namespace ovnis {

SumoTraciConnection::SumoTraciConnection()
 :host("localhost"), port(0), config(""), socket(host, port), currentTime(0), stepQuery(&socket, currentTime) {
	boundaries[0] = 0;
	boundaries[1] = 0;
}

SumoTraciConnection::~SumoTraciConnection() {
}

void SumoTraciConnection::RunServer(string sumoConfig = "", string host = "", string sumoPath = "", int port = 0, string outputFolder="") {
	currentTime = 0;
	if (!host.empty()) {
		this->host = host;
	}
	this->port = port;
	if (!sumoConfig.empty()) {
		this->port = StartSumo(sumoConfig, sumoPath, outputFolder);
	}
	socket = Socket(this->host, this->port);
	sleep(2);
	try {
		socket.connect();
		Log::getInstance().getStream("") << "starting SUMO with config " << sumoConfig << " on " << this->host << ":" << this->port << endl;
	}
	catch (SocketException & e) {
		cerr << e.what();
		return;
	}
	stepQuery = SimStepQuery(&socket, currentTime);
}

int SumoTraciConnection::StartSumo(string config, string sumoPath, string outputFolder) {
	this->sumoPath = sumoPath;
	this->config = config;
	if (config.empty()) {
		cerr << "#Error: given port is different that read from sumo config file.";
		throw;
	}
	if (sumoPath.empty()) {
		sumoPath = SUMO_PATH;
	}
	// retrieve SUMO network port number and simulation boundaries from config files
	int sumoPort;
	string configPath = outputFolder+config;
	cout << "Reading config file " << configPath << endl;
	XMLSumoConfParser::parseConfiguration(configPath, &sumoPort, boundaries);
	this->port = sumoPort;
	if (this->port == 0) {
		this->port = SUMO_PORT;
	}
	if ((pid = fork()) == 0) {
		char buff[512];
		ofstream out;
		out.open((outputFolder+"/sumo_output.log").c_str());
		out << "Output log file from sumo's execution.";
		FILE * fp;
		string args =  " --summary-output=" + outputFolder+"summary.xml "
//				+ "--fcd-output=" + outputFolder + "fcd.xml "
//				+ " --netstate="   + outputFolder + "netsate.xml"
				+ "--tripinfo-output=" + outputFolder + "tripinfo.xml "
//				+ "--full-output=" + outputFolder + "/full.xml";
				;
		if ((fp = popen((sumoPath + " -c " + outputFolder+"/"+config + " " + args + " 2>&1").c_str(), "r")) == NULL) {
			cerr <<  "#Error: Sumo processes cannot be created" << endl;
			throw;
		}
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			out << buff;
			out.flush();
		}
		pclose(fp);
		exit(0);
	}
	return port;
}


void SumoTraciConnection::SubscribeSimulation(int startTime, int stopTime) {
	stepQuery.SetCurrentTime(startTime);
	vector<int> variables;
	variables.push_back(VAR_TIME_STEP);
	variables.push_back(VAR_DEPARTED_VEHICLES_IDS);
	variables.push_back(VAR_ARRIVED_VEHICLES_IDS);
	SubscribeQuery subscribeQuery(&socket, CMD_SUBSCRIBE_SIM_VARIABLE, variables, startTime, stopTime);
	subscribeQuery.DoCommand();
}

void SumoTraciConnection::NextSimStep(vector<string> & departedVehicles, vector<string> & arrivedVehicles) {
	stepQuery.DoCommand();
	currentTime = stepQuery.GetCurrentTime();
	departedVehicles = stepQuery.getDepartedVehicles();
	arrivedVehicles = stepQuery.getArrivedVehicles();
}

double SumoTraciConnection::GetVehicleSpeed(string vehicleId) {
	VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, VAR_SPEED);
	vehicleQuery.DoCommand();
	return vehicleQuery.getDoubleResponse();
}

string SumoTraciConnection::GetVehicleEdge(string vehicleId) {
	try {
        VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, VAR_ROAD_ID);
        if (vehicleQuery.DoCommand() != 0) {
        	return "";
		}
		return vehicleQuery.getStringResponse();
	}
	catch (TraciException &e) {
		cout << vehicleId << " " <<  CMD_GET_VEHICLE_VARIABLE << " " <<  VAR_ROAD_ID << endl;
		return "";
		throw e;
	}
	catch (exception &e) {
		return "";
		throw e;
	}

}

string SumoTraciConnection::GetVehicleLane(string vehicleId) {
	VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, VAR_LANE_ID);
	vehicleQuery.DoCommand();
	return vehicleQuery.getStringResponse();
}

double SumoTraciConnection::GetLaneLength(string laneId) {
	LaneQuery laneQuery(&socket, laneId, CMD_GET_LANE_VARIABLE, VAR_LENGTH);
	laneQuery.DoCommand();
	return laneQuery.getDoubleResponse();
}

double SumoTraciConnection::GetLaneMaxSpeed(string laneId) {
	LaneQuery laneQuery(&socket, laneId, CMD_GET_LANE_VARIABLE, VAR_MAXSPEED);
	laneQuery.DoCommand();
	return laneQuery.getDoubleResponse();
}

string SumoTraciConnection::GetVehicleRoute(string vehicleId) {
	VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_ID);
	vehicleQuery.DoCommand();
	return vehicleQuery.getStringResponse();
}

int SumoTraciConnection::GetVehicleCount(string vehicleId) {
	VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, 1);
	vehicleQuery.DoCommand();
	return vehicleQuery.getIntResponse();
}

string SumoTraciConnection::GetVehicleRouteId(string vehicleId) {
	try {
		VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, VAR_ROUTE_ID);
		vehicleQuery.DoCommand();
		return vehicleQuery.getStringResponse();
	}
	catch (exception &e) {
		throw e;
		return "";
	}
}

vector<string> SumoTraciConnection::GetVehicleEdges(string vehicleId) {
	try {
		VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, VAR_EDGES);
		vehicleQuery.DoCommand();
		return vehicleQuery.getStringListResponse();
	}
	catch (exception &e) {
		throw e;
		return vector<string>();
	}
}

void SumoTraciConnection::ChangeVehicleEdges(string vehicleId, vector<string> edges) {
	VehicleQuery vehicleQuery(&socket, vehicleId, CMD_SET_VEHICLE_VARIABLE, VAR_ROUTE);
	vehicleQuery.SetStringListValue(edges);
	vehicleQuery.DoCommand();
}

double SumoTraciConnection::GetVehicleAngle(string vehicleId) {
	VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, VAR_ANGLE);
	vehicleQuery.DoCommand();
	return vehicleQuery.getDoubleResponse();
}

Position2D SumoTraciConnection::GetVehiclePosition(string vehicleId) {
	VehicleQuery vehicleQuery(&socket, vehicleId, CMD_GET_VEHICLE_VARIABLE, VAR_POSITION);
	vehicleQuery.DoCommand();
	return vehicleQuery.getPositionResponse();
}

const double SumoTraciConnection::GetEdgeTravelTime(string edgeId) {
	EdgeQuery edgeQuery(&socket, edgeId, CMD_GET_EDGE_VARIABLE, VAR_CURRENT_TRAVELTIME);
	edgeQuery.DoCommand();
	return edgeQuery.getDoubleResponse();
}

const double SumoTraciConnection::GetEdgeGlobalTravelTime(string edgeId) {
	EdgeQuery edgeQuery(&socket, edgeId, CMD_GET_EDGE_VARIABLE, VAR_EDGE_TRAVELTIME);
	edgeQuery.DoCommand();
	return edgeQuery.getDoubleResponse();
}

void SumoTraciConnection::CloseLane(string laneId) {
	EdgeQuery edgeQuery(&socket, laneId, CMD_SET_LANE_VARIABLE, VAR_MAXSPEED);
	edgeQuery.DoCommand();
}

void SumoTraciConnection::CloseEdge(string edgeId) {
	EdgeQuery edgeQuery(&socket, edgeId, CMD_SET_EDGE_VARIABLE, VAR_EDGE_TRAVELTIME);
	edgeQuery.DoCommand();
}

vector<double> SumoTraciConnection::GetSimulationBoundaries() {
	SimulationQuery simQuery(&socket, CMD_GET_SIM_VARIABLE, VAR_NET_BOUNDING_BOX);
	simQuery.DoCommand();
	return simQuery.getVectorResponse();
}

/**
 * Closes the connection, quits the simulator, frees any stale
 * resource and makes all Vehicle instances inactive.
 */
void SumoTraciConnection::Close() {
	CloseQuery query(&socket);
	query.DoCommand();
	socket.close();
}

const int SumoTraciConnection::GetCurrentTime() const
{
    return currentTime;
}

} /* namespace ovnis */
