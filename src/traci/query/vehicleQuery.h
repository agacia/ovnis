/*
 * vehicleQuery.h
 *
 *  Created on: Mar 27, 2012
 *      Author: agata
 */

#ifndef VEHICLEQUERY_H_
#define VEHICLEQUERY_H_

#include "query.h"

namespace ovnis {

class VehicleQuery : public Query {

public:
	VehicleQuery();
	VehicleQuery(tcpip::Socket * socket, string vehicleId, int commandId = CMD_GET_VEHICLE_VARIABLE, int variableId = VAR_ROAD_ID);
	virtual ~VehicleQuery();

private:

    virtual void InitializeCommand(Command & command);
	virtual void ReadResponse(tcpip::Storage & content);

	void InitializeChangeEdgesCommand(Command & command, int type, vector<string> edges);
};

} /* namespace ovnis */
#endif /* VEHICLEQUERY_H_ */
