/*
 * edgeQuery.cpp
 *
 *  Created on: Mar 30, 2012
 *      Author: agata
 */

/**
 * CMD_GET_VEHICLE_VARIABLE 0xa4
 * CMD_SET_VEHICLE_VARIABLE 0xc4
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

#include "edgeQuery.h"
#include <limits>

namespace ovnis {

EdgeQuery::EdgeQuery() :
	Query() {
}

EdgeQuery::EdgeQuery(tcpip::Socket * socket, string objectId, int commandId, int variableId) :
		Query(socket, objectId, commandId, variableId) {
}

EdgeQuery::~EdgeQuery() {
}

void EdgeQuery::InitializeCommand(Command & command) {
	if (commandId == CMD_SET_LANE_VARIABLE) {
		// max speed
		InitializeChangeCommand(command, TYPE_DOUBLE, 0);
	}
	else if (commandId == CMD_SET_EDGE_VARIABLE) {
		InitializeChangeEdgeTravelTimeCommand(command, 0, 50000, numeric_limits<double>::max());
	}
	else if (commandId == CMD_GET_EDGE_VARIABLE) {
		InitializeGetCommand(command);
		if (variableId == VAR_EDGE_TRAVELTIME) {
			InitializeRequestGlobalTimeCommand(command);
		}
		else {
			//InitializeGetCommand(command);
		}
	}
}

void EdgeQuery::InitializeChangeCommand(Command & command, int type, int value) {
	command = Command(commandId);
	int contentSize = sizeof(unsigned char) + (sizeof(int) + objectId.size()) + sizeof(unsigned char) + sizeof(double);
	command.WriteHeader(contentSize);
	command.Content().writeUnsignedByte(VAR_MAXSPEED); // var id change edge travel time information (0x58)
	command.Content().writeString(objectId); //string laneId
	command.Content().writeUnsignedByte(TYPE_DOUBLE); //type of value (double)
	command.Content().writeDouble(value); // maxspeed
}

/**
 * change global travel time information (0x58)
 * Inserts the information about the travel time of the named edge valid from begin time to end time into the global edge weights times container.
 * byte	 value type compound
 * int	 number of elements (always=3)
 * byte	 value type integer
 * int	 begin time (in s)
 * byte	 value type integer
 * int	 end time (in s)
 * byte	 value type double
 * double travel time value (in s)
 */
void EdgeQuery::InitializeChangeEdgeTravelTimeCommand(Command & command, int beginTime, int endTime, double travelTime) {
	int contentSize = sizeof(unsigned char) + (sizeof(int) + objectId.size()) + // variable + objectId
				+ sizeof(unsigned char) + sizeof(int) + sizeof(unsigned char) + sizeof(int) + sizeof(unsigned char) + sizeof(int) + sizeof(unsigned char) + sizeof(double) ;
	command = Command(commandId);
	command.WriteHeader(contentSize);
	command.Content().writeUnsignedByte(variableId);
	command.Content().writeString(objectId); //string objectId
	command.Content().writeUnsignedByte(TYPE_COMPOUND); // type compound 0x0F
	command.Content().writeInt(3); // number of elements
	command.Content().writeUnsignedByte(TYPE_INTEGER); // 0x09
	command.Content().writeInt(beginTime);
	command.Content().writeUnsignedByte(TYPE_INTEGER);
	command.Content().writeInt(endTime);
	command.Content().writeUnsignedByte(TYPE_DOUBLE);
	command.Content().writeDouble(travelTime);
}

void EdgeQuery::InitializeRequestGlobalTimeCommand(Command & command) {
	command = Command(commandId);
	int contentSize = sizeof(unsigned char) + (sizeof(int) + objectId.size()) + sizeof(unsigned char) + sizeof(int);
	command.WriteHeader(contentSize);
	command.Content().writeUnsignedByte(variableId);

	command.Content().writeString(objectId);

	command.Content().writeUnsignedByte(TYPE_INTEGER);
	command.Content().writeInt(2700);

}

void EdgeQuery::ReadResponse(tcpip::Storage & content) {
	if (commandId == CMD_SET_LANE_VARIABLE) {
	}
	else if (commandId == CMD_SET_EDGE_VARIABLE) {
	}
	else if (commandId == CMD_GET_EDGE_VARIABLE) {
		ReadGetResponse(content);
	}
}

} /* namespace ovnis */
