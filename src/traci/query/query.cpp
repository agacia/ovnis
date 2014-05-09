/*
 * query.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#include "query.h"
#include <sstream>
#include <string>
#include <iostream>

using namespace std;
using namespace tcpip;

namespace ovnis {

Query::Query() {
}

Query::Query(Socket * socket) {
	this->socket = socket;
}

Query::Query(tcpip::Socket * socket, string objectId, int commandId, int variableId) :
	socket(socket), objectId(objectId), commandId(commandId), variableId(variableId) {
}

Query::~Query() {}

int Query::getCommandId() const {
	return commandId;
}

int Query::getVariableId() const {
	return variableId;
}

string Query::getObjectId() const
{
	return objectId;
}

void Query::setCommandId(int commandId) {
	this->commandId = commandId;
}

void Query::setVariableId(int variableId) {
	this->variableId = variableId;
}

void Query::setObjectId(string objectId) {
	this->objectId = objectId;
}

void Query::SetIntValue(int intValue) {
	this->intValue = intValue;
}

void Query::SetDoubleValue(double doubleValue) {
	this->doubleValue = doubleValue;
}

void Query::SetStringValue(string stringValue) {
	this->stringValue = stringValue;
}

void Query::SetStringListValue(vector<string> stringList) {
	stringListValue = stringList;
}

int Query::DoCommand() {
	doubleValue = 0;
	stringValue.clear();
	intValue = 0;
	positionValue.x = 0;
	positionValue.y = 0;
	vectorValue.clear();

	InitializeCommand(requestCommand);
	SendRequestAndReceiveResponse(requestCommand);
	if (ValidateResponse(responseStream)) {
		ReadResponse(responseStream);
		return 0;
	}
	else {
		stringstream ss;
		ss << "Wrong response to command " << commandId << ", variable " << variableId << " " << responseStatus.getDescription();
		string msg = ss.str();
//		throw TraciException(msg);
		return -1;
	}
}

void Query::InitializeGetCommand(Command & command) {
	command = Command(commandId);
	int contentSize = sizeof(unsigned char) + (sizeof(int) + objectId.size());
	command.WriteHeader(contentSize);
	command.Content().writeUnsignedByte(variableId);
	command.Content().writeString(objectId);
}

void Query::SendRequestAndReceiveResponse(Command & command) {
	try {
		socket->sendExact(command.Content());
	}
	catch (SocketException & e) {
		cout << "--Error while sending command: " << e.what();
	}
	catch (exception & e) {
		cout << "--Error while sending command: " << e.what();
	}
	try {
		responseStream.reset();
		socket->receiveExact(responseStream);
	}
	catch (SocketException & e) {
		cout << "Error while receiving command: " << e.what();
	}
}

bool Query::ValidateResponse(Storage & responseMessage) {
	responseStatus = Status();
	try {
		responseStatus.setLength(responseMessage.readUnsignedByte());
		responseStatus.setId(responseMessage.readUnsignedByte());
		responseStatus.setStatus(responseMessage.readUnsignedByte());
		responseStatus.setDescription(responseMessage.readString());
	}
	catch (invalid_argument & e) {
		cout << "Error while reading response: " << e.what();
	}
	if (responseStatus.getStatus() != RTYPE_OK) {
		return false;
	}
	return true;
}

void Query::ReadRawResponse(tcpip::Storage & content) {
	cout  << "Response content:" << endl;
	for (int i = content.position(); i < content.size(); ++i) {
		cout << "byte" << i << ": " << content.readUnsignedByte() << endl;
	}
}

void Query::ReadGetResponse(tcpip::Storage & content) {
	int position = 0;
	int length2 = 0;
	int extLength = 0;
	int length = 0;
	int commandId = 0;
	int varId = 0;
	string objectId = "";
	int variableType = 0;
	try {
		position = content.position();
		length2 = content.size();
		extLength = content.readUnsignedByte();
		if (extLength == 0) {
			length = content.readInt();
		}
		commandId = content.readUnsignedByte();
		varId = content.readUnsignedByte();
		objectId = content.readString();
		variableType = content.readUnsignedByte();
		int listLen = 0;
		int count = 0;
		switch (variableType) {
		case TYPE_STRING:
			stringValue = content.readString();
			break;
		case TYPE_INTEGER:
			intValue = content.readInt();
			break;
		case TYPE_DOUBLE:
			doubleValue = content.readDouble();
			break;
		case TYPE_STRINGLIST:
			listLen = content.readInt();
			stringListValue.clear();
			for (int i=0; i<listLen; i++) {
				stringListValue.push_back(content.readString());
			}
			break;
		case POSITION_2D:
			positionValue.x = content.readDouble();
			positionValue.y = content.readDouble();
			break;
		case TYPE_BOUNDINGBOX:
			count = content.size()/sizeof(double);
			for (int i = 0; i < count-1; i++) {
				vectorValue.push_back(content.readDouble());
			}
			break;
		default:
			break;
		}
	}
	catch (exception & ex) {
			cout << "can't read response for object " << objectId << ex.what() << endl;
			cout<<position<<" "<<length2 << " "<<extLength<<" "<<length<<" "<<commandId<<" "<<varId<<" "<<objectId<<" "<<variableType<< "\n";
		}
}

void Query::ReadSetResponse(tcpip::Storage & content) {
//	cout << endl << "Response to vehicle query: " << endl;
	int pos = content.position();
	cout << "pos: " << pos << endl;
	int length = content.size();
	cout << "length: " << length << endl;
//	int length2 = content.readUnsignedByte();
//	cout << "length: " << length2 << endl;
//	int commandId = content.readUnsignedByte();
//	cout << "command id : " << hex << commandId << endl;
//	int varId = content.readUnsignedByte();
//	cout << "variable id: " << hex << varId << endl;
//	string vehicleId = content.readString();
//	cout << "vehicle id: " << vehicleId << endl;
//	int variableType = content.readUnsignedByte();
//	cout << "type: " << variableType << endl;
//	string variableStr;
//	int variableInt;
//	cout << "cmdid " << commandId;
//	double variableDouble;
//	switch (variableType) {
//		case TYPE_STRING:
//			variableStr = content.readString();
//			break;
//		case TYPE_INTEGER:
//			variableInt = content.readInt();
//			break;
//		case TYPE_DOUBLE:
//			variableDouble = content.readDouble();
//			break;
//	}
//	// remember the result
//	switch (varId) {
//		case VAR_CURRENT_TRAVELTIME:
//			travelTime = variableDouble;
//			break;
//		case VAR_EDGE_TRAVELTIME:
//			travelTime = variableDouble;
//			break;
//	}
}

string Query::getStringResponse() const
{
	return stringValue;
}

vector<string> Query::getStringListResponse() const
{
	return stringListValue;
}

double Query::getDoubleResponse() const
{
	return doubleValue;
}

Position2D Query::getPositionResponse() const
{
	return positionValue;
}

int Query::getIntResponse() const
{
	return intValue;
}

vector<double> Query::getVectorResponse() const {
	return vectorValue;
}

//void Query::ValidateGetVersionResponse(tcpip::Storage & inMsg) {
//	uint32_t pos = inMsg.position();
//	cout << "pos: " << pos << endl;
//	int lengthExt = inMsg.readUnsignedByte();
//	cout << "lengthExt: " << lengthExt << endl;
//	int varNo2 = inMsg.readUnsignedByte();
//	cout << "cmdID: " << varNo2 << endl;
//	int b1 = inMsg.readUnsignedByte();
//	cout << "status: " << b1 << endl;
//	string objectId = inMsg.readString();
//	cout << "objectId: " << objectId << endl;
//	int varNo7 = inMsg.readInt();
//	cout << "api version : " << varNo7 << endl;
//	string varNo8 = inMsg.readString();
//	cout << "identifier: " << varNo8 << endl;
//}

/* namespace ovnis */
}

