/*
 * laneQuery.cpp
 *
 *  Created on: Mar 30, 2012
 *      Author: agata
 */

#include "laneQuery.h"
#include <limits>

namespace ovnis {

LaneQuery::LaneQuery() :
	Query() {
}

LaneQuery::LaneQuery(tcpip::Socket * socket, string objectId, int commandId, int variableId) :
		Query(socket, objectId, commandId, variableId) {
}

LaneQuery::~LaneQuery() {
}

void LaneQuery::InitializeCommand(Command & command) {
	if (commandId == CMD_SET_LANE_VARIABLE) {
//		InitializeChangeCommand(command);
	}
	else if (commandId == CMD_GET_LANE_VARIABLE) {
		InitializeGetCommand(command);

	}
}

void LaneQuery::InitializeChangeCommand(Command & command, int type, int value) {
	// TODO
}


void LaneQuery::InitializeRequestGlobalTimeCommand(Command & command) {
	command = Command(commandId);
	int contentSize = sizeof(unsigned char) + (sizeof(int) + objectId.size()) + sizeof(unsigned char) + sizeof(int);
	command.WriteHeader(contentSize);
	command.Content().writeUnsignedByte(variableId);

	command.Content().writeString(objectId);

	command.Content().writeUnsignedByte(TYPE_INTEGER);
	command.Content().writeInt(2700);

}

void LaneQuery::ReadResponse(tcpip::Storage & content) {
	if (commandId == CMD_SET_LANE_VARIABLE) {
	}
	else if (commandId == CMD_SET_LANE_VARIABLE) {
	}
	else if (commandId == CMD_GET_LANE_VARIABLE) {
		ReadGetResponse(content);
	}
}

} /* namespace ovnis */
