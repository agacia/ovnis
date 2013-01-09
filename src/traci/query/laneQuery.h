/*
 * edgeQuery.h
 *
 *  Created on: Mar 30, 2012
 *      Author: agata
 */

#ifndef LANEQUERY_H_
#define LANEQUERY_H_

#include "query.h"

namespace ovnis {

class LaneQuery : public Query {
public:
	LaneQuery();
	LaneQuery(tcpip::Socket * socket, string nodeId, int commandId = CMD_GET_LANE_VARIABLE, int variableId = VAR_LENGTH);
	virtual ~LaneQuery();


protected:
	virtual void InitializeCommand(Command & command);
	virtual void ReadResponse(tcpip::Storage & content);

	void InitializeRequestGlobalTimeCommand(Command & command);
	void InitializeChangeCommand(Command & command, int type, int value);

};

} /* namespace ovnis */
#endif /* LANEQUERY_H_ */
