/*
 * SimulationQuery.h
 *
 *  Created on: Jul 31, 2012
 *      Author: agata
 */

#ifndef SIMULATIONQUERY_H_
#define SIMULATIONQUERY_H_

#include "query.h"

namespace ovnis {

class SimulationQuery : public Query {

public:
	SimulationQuery();
	SimulationQuery(tcpip::Socket * socket, int commandId = CMD_GET_SIM_VARIABLE, int variableId = VAR_NET_BOUNDING_BOX);
	virtual ~SimulationQuery();

private:

    virtual void InitializeCommand(Command & command);
	virtual void ReadResponse(tcpip::Storage & content);

};

} /* namespace ovnis */

#endif /* SIMULATIONQUERY_H_ */
