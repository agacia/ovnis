/*
 * SimulationQuery.cpp
 *
 *  Created on: Jul 31, 2012
 *      Author: agata
 */

#include "simulationQuery.h"

namespace ovnis {

SimulationQuery::SimulationQuery() :
		Query() {
}
SimulationQuery::SimulationQuery(tcpip::Socket * socket, int commandId, int variableId) :
				Query(socket, "", commandId, variableId) {
}

SimulationQuery::~SimulationQuery() {
}

void SimulationQuery::InitializeCommand(Command & command) {
	if (commandId == CMD_GET_SIM_VARIABLE) {
		InitializeGetCommand(command);
	}
}

void SimulationQuery::ReadResponse(tcpip::Storage & content) {
	if (commandId == CMD_GET_SIM_VARIABLE) {
		ReadGetResponse(content);
	}
}

} /* namespace ovnis */
