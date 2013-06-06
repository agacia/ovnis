/*
 * query.h
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#ifndef QUERY_H_
#define QUERY_H_

#include <iostream>
//#include <foreign/tcpip/storage.h>
//#include <foreign/tcpip/socket.h>
#include "traci/storage.h"
#include "traci/socket.h"

#include <traci-server/TraCIConstants.h>

#include "log.h"
#include "traci/protocol/command.h"
#include "traci/protocol/status.h"
#include "traci/structs.h"

namespace ovnis {


class Query {
public:
	Query();
	Query(tcpip::Socket * socket);
	Query(tcpip::Socket * socket, string nodeId, int commandId, int variableId);
	virtual ~Query();

	int getCommandId() const;
	int getVariableId() const;
	string getObjectId() const;
	void setVariableId(int variableId);
	void setCommandId(int commandId);
	void setObjectId(string vehicleId);
	void SetIntValue(int intValue);
	void SetDoubleValue(double doubleValue);
	void SetStringValue(string stringValue);
	void SetStringListValue(vector<string> stringList);

	int DoCommand();

	int getIntResponse() const;
	double getDoubleResponse() const;
    std::string getStringResponse() const;
    std::vector<std::string> getStringListResponse() const;
    Position2D getPositionResponse() const;
    vector<double> getVectorResponse() const;

protected:
    int commandId;
    int variableId;
    string objectId;

	tcpip::Socket * socket;
	Command requestCommand;

	tcpip::Storage responseStream;
	Status responseStatus;

	int intValue;
	double doubleValue;
	string stringValue;
	vector<string> stringListValue;
	Position2D positionValue;
	vector<double> vectorValue;

	virtual void InitializeCommand(Command & command) = 0;
	void InitializeGetCommand(Command & command);
	void SendRequestAndReceiveResponse(Command & command);
	bool ValidateResponse(tcpip::Storage & responseMessage);

	virtual void ReadRawResponse(tcpip::Storage & responseMessage);
	virtual void ReadResponse(tcpip::Storage & responseMessage) = 0;

	virtual void ReadGetResponse(tcpip::Storage & content);
	virtual void ReadSetResponse(tcpip::Storage & content);
};

} /* namespace ovnis */

#endif /* QUERY_H_ */
