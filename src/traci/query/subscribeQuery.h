/*
 * subscribeQuery.h
 *
 *  Created on: Mar 26, 2012
 *      Author: agata
 */

#ifndef SUBSCRIBEQUERY_H_
#define SUBSCRIBEQUERY_H_

#include "query.h"

namespace ovnis {

class SubscribeQuery : public Query {

public:
	SubscribeQuery();
	SubscribeQuery(tcpip::Socket * socket, int commandId, std::vector<int> variables, int startTime, int stopTime);
	virtual ~SubscribeQuery();

	virtual void InitializeCommand(Command & command);
	virtual void ReadResponse(tcpip::Storage & content);

private:
	int startTime;
	int stopTime;
	std::vector<int> variables;
};

} /* namespace ovnis */

#endif /* SUBSCRIBEQUERY_H_ */
