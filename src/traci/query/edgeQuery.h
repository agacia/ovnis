/*
 * edgeQuery.h
 *
 *  Created on: Mar 30, 2012
 *      Author: agata
 */

#ifndef EDGEQUERY_H_
#define EDGEQUERY_H_

#include "query.h"

namespace ovnis {

class EdgeQuery : public Query {
public:
	EdgeQuery();
	EdgeQuery(tcpip::Socket * socket, string nodeId, int commandId = CMD_GET_EDGE_VARIABLE, int variableId = VAR_CURRENT_TRAVELTIME);
	virtual ~EdgeQuery();


protected:
	virtual void InitializeCommand(Command & command);
	virtual void ReadResponse(tcpip::Storage & content);

	void InitializeRequestGlobalTimeCommand(Command & command);
	void InitializeChangeEdgeTravelTimeCommand(Command & command, int beginTime, int endTime, double travelTime);
	void InitializeChangeCommand(Command & command, int type, int value);

};

} /* namespace ovnis */
#endif /* EDGEQUERY_H_ */
