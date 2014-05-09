/*
 * vehicleQuery.cpp
 *
 *  Created on: Mar 27, 2012
 *      Author: agata
 */

/**
 * string:
 * VAR_ROAD_ID 0x50 road id (get: vehicles)
 * LANE_EDGE_ID 0x31 id of parent edge (get: lanes)
 * VAR_LANE_ID 0x51 lane id (get: vehicles)
 * VAR_ROUTE_ID 0x53 route id (get & set: vehicles)
 * string list:
 * VAR_EDGES 0x54 edges (get: routes)
 * double:
 * VAR_SPEED 0x40 speed (get: vehicle)
 */
/**
 * CMD_GET_VEHICLE_VARIABLE 0xa4
 * CMD_SET_VEHICLE_VARIABLE 0xc4
 */


#include "vehicleQuery.h"
#include <iomanip>

namespace ovnis {

VehicleQuery::VehicleQuery() :
		Query() {
}

VehicleQuery::VehicleQuery(tcpip::Socket * socket, string objectId, int commandId, int variableId) :
		Query(socket, objectId, commandId, variableId) {
}

VehicleQuery::~VehicleQuery() {
}

void VehicleQuery::InitializeCommand(Command & command) {
	if (commandId == CMD_SET_VEHICLE_VARIABLE) {
		if (variableId == VAR_ROUTE) {
			InitializeChangeEdgesCommand(command, TYPE_STRINGLIST, stringListValue);
		}
		else {
			//InitializeGetCommand(command);
		}
	}
	else if (commandId == CMD_GET_VEHICLE_VARIABLE) {
		InitializeGetCommand(command);
	}
}

void VehicleQuery::InitializeChangeEdgesCommand(Command & command, int type, vector<string> value) {
	int sizeEdges = 0;
	for (vector<string>::iterator it = value.begin(); it < value.end(); ++it) {
		sizeEdges += it->size() + sizeof(int);
	}
	int contentSize = sizeof(unsigned char) + (sizeof(int) + objectId.size()) + sizeof(unsigned char) + sizeof(int) + sizeEdges;
	command = Command(commandId);
	command.WriteHeader(contentSize);
	command.Content().writeUnsignedByte(variableId);
	command.Content().writeString(objectId);
	command.Content().writeUnsignedByte(type); //type of value
	command.Content().writeStringList(value);
}

void VehicleQuery::ReadResponse(tcpip::Storage & content) {
	if (commandId == CMD_GET_VEHICLE_VARIABLE) {
		ReadGetResponse(content);
	}
	else if (commandId == CMD_SET_VEHICLE_VARIABLE) {
	}
}

} /* namespace ovnis */
