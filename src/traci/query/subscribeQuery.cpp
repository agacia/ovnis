/*
 * subscribeQuery.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: agata
 */

#include "subscribeQuery.h"

namespace ovnis {

using namespace std;
using namespace tcpip;

SubscribeQuery::SubscribeQuery() :
Query(socket, "", commandId, variableId), variables(std::vector<int>()), startTime(0), stopTime(0) {
}

SubscribeQuery::SubscribeQuery(tcpip::Socket * socket, int commandId, std::vector<int> variables, int startTime, int stopTime) :
Query(socket, "", commandId, 0), variables(variables), startTime(startTime), stopTime(stopTime) {
}

SubscribeQuery::~SubscribeQuery() {
}

void SubscribeQuery::InitializeCommand(Command & command) {
	command = Command(commandId);
	string s = "*";
	int variablesCount = variables.size();
	command.WriteHeader(sizeof(int) + sizeof(int) + (sizeof(int) + (int)s.length()) + sizeof(unsigned char) + variablesCount);
	command.Content().writeInt(startTime); // Time: the subscription is executed only in time steps >= this value; in ms
	command.Content().writeInt(stopTime); // Time: the subscription is executed in time steps <= this value; the subscription is removed if the simulation has reached a higher time step; in ms
	command.Content().writeString(s); // objectId -> unused
	command.Content().writeUnsignedByte(variablesCount); // number of subscribed variables
	for (vector<int>::iterator i = variables.begin(); i != variables.end(); ++i) {
		command.Content().writeUnsignedByte((int)*i);
	}
}

void SubscribeQuery::ReadResponse(Storage & content) {
//	cout << endl << "Response: " << endl;
//	int pos = content.position();
//	cout << "pos: " << pos << endl;
//	int length = content.size();
//	cout << "length: " << length << endl;
//	cout << "additional byte: " << content.readUnsignedByte() << endl;
//	cout << "length: " << content.readInt() << endl;
//	cout << "0xdb subscribe simulation value command id: " << content.readUnsignedByte() << endl;
//	cout << "object id: " << content.readString() << endl;
//	cout << "variable count: " << content.readUnsignedByte() << endl;
//	cout << "var1:" << endl;
//	cout << "0x70 simulation time command id: " << content.readUnsignedByte() << endl;
//	cout << "status: " << content.readUnsignedByte() << endl;
//	cout << "data type: " << content.readUnsignedByte() << endl;
//	cout << "value: " << content.readInt() << endl;
//	cout << "var2:" << endl;
//	cout << "0x74 departed vehicles: " << content.readUnsignedByte() << endl;
//	cout << "status: " << content.readUnsignedByte() << endl;
//	cout << "data type: " << content.readUnsignedByte() << endl;
//	cout << "number: " << content.readInt() << endl;
//	cout << "var3:" << endl;
//	cout << "0x7a arrived vehicles: " << content.readUnsignedByte() << endl;
//	cout << "status: " << content.readUnsignedByte() << endl;
//	cout << "data type: " << content.readUnsignedByte() << endl;
//	cout << "number: " << content.readInt() << endl;
}

} /* namespace ovnis */
