/*
 *
 * Copyright (c) 2010-2011 University of Luxembourg
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * @file TraciClientClient.cpp
 *
 * @date Mar 10, 2010
 *
 * @author Yoann Pigné <yoann@pigne.org>
 *
 */
#include <unistd.h>
#include "traci-client.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include "ns3/log.h"
#define BUILD_TCPIP
#include <foreign/tcpip/storage.h>
#include <foreign/tcpip/socket.h>
#include <limits.h>
NS_LOG_COMPONENT_DEFINE("TraciClient");

using namespace std;
using namespace tcpip;

namespace traciclient {

TraciClient::TraciClient()
//:	socket("localhost", 0)
{
	socket = new Socket("localhost", 0);
}

TraciClient::~TraciClient() {
	//delete socket;
}

bool TraciClient::connect(std::string host, int port) {
	std::stringstream msg;
	//socket = new tcpip::Socket(host, port);

	socket = new Socket(host, port);
	//    socket->set_blocking(true);
	sleep(2);
	try {
		socket->connect();
	} catch (SocketException &e) {
		cout << "#Error while connecting: " << e.what();
		errorMsg(msg);
		return false;
	}

	return true;
}

void TraciClient::errorMsg(std::stringstream & msg) {
	cerr << msg.str() << endl;

}

bool TraciClient::close() {
	socket->close();
	return true;
}

void TraciClient::commandClose() {
	tcpip::Storage outMsg;
	tcpip::Storage inMsg;
	std::stringstream msg;

	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return;
	}

	// command length
	outMsg.writeUnsignedByte(1 + 1);
	// command id
	outMsg.writeUnsignedByte(CMD_CLOSE);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// validate result state
	if (!reportResultState(inMsg, CMD_CLOSE)) {
		return;
	}
}

bool TraciClient::reportExtendedResultState(tcpip::Storage & inMsg, int command, bool ignoreCommandId) {
	uint32_t cmdLength;
	int cmdLengthExt;
	int cmdId;
	int resultType;
	uint32_t cmdStart;
	std::string msg;

	try {
		cmdStart = inMsg.position();
		cmdLength = inMsg.readUnsignedByte();
		cmdLengthExt = inMsg.readInt();
		cmdId = inMsg.readUnsignedByte();
		if (cmdId != command && !ignoreCommandId) {
			return false;
		}
		resultType = inMsg.readUnsignedByte();
		msg = inMsg.readString();

	}
	catch (std::invalid_argument & e) {
		int p = inMsg.position();
		NS_LOG_DEBUG( "#Error: an exception was thrown while reading result state message"<< endl << "---" << p << endl);
		return false;
	}
	switch (resultType) {
	case RTYPE_ERR:
		NS_LOG_DEBUG( ".. Answered with error to command (" << cmdId
				<< "), [description: " << msg << "]" << endl);
		return false;
	case RTYPE_NOTIMPLEMENTED:
		NS_LOG_DEBUG(".. Sent command is not implemented (" << cmdId << "), [description: " << msg << "]" << endl);
		return false;
	case RTYPE_OK:
		break;
	default:
		NS_LOG_DEBUG(".. Answered with unknown result code(" << resultType << ") to command(" << cmdId << "), [description: " << msg << "]" << endl);
		return false;
	}
	if ((cmdStart + cmdLength) != inMsg.position()) {
		NS_LOG_DEBUG("#Error: command at position " << cmdStart << " has wrong length" << endl);
		return false;
	}
	return true;
}

bool TraciClient::reportResultState(tcpip::Storage & inMsg, int command, bool ignoreCommandId) {
	uint32_t cmdLength;
	int cmdId;
	int resultType;
	uint32_t cmdStart;
	std::string msg;

	try {
		cmdStart = inMsg.position();
		cmdLength = inMsg.readUnsignedByte();
		cmdId = inMsg.readUnsignedByte();
		if (cmdId != command && !ignoreCommandId) {
			return false;
		}
		resultType = inMsg.readUnsignedByte();
		msg = inMsg.readString();

	} catch (std::invalid_argument & e) {
		int p = inMsg.position();
		NS_LOG_DEBUG( "#Error: an exception was thrown while reading result state message"<< endl << "---" << p << endl);
		return false;
	}
	switch (resultType) {
	case RTYPE_ERR:
		NS_LOG_DEBUG( ".. Answered with error to command (" << cmdId
				<< "), [description: " << msg << "]" << endl);
		return false;
	case RTYPE_NOTIMPLEMENTED:
		NS_LOG_DEBUG(".. Sent command is not implemented (" << cmdId << "), [description: " << msg << "]" << endl);
		return false;
	case RTYPE_OK:
		break;
	default:
		NS_LOG_DEBUG(".. Answered with unknown result code(" << resultType << ") to command(" << cmdId << "), [description: " << msg << "]" << endl);
		return false;
	}
	if ((cmdStart + cmdLength) != inMsg.position()) {
		NS_LOG_DEBUG("#Error: command at position " << cmdStart << " has wrong length" << endl);
		return false;
	}
	return true;
}

void TraciClient::submission(int start, int stop, u_int8_t dom, std::vector<u_int8_t> variables) {

	std::stringstream msg;

	if (variables.empty() && dom != CMD_SUBSCRIBE_SIM_VARIABLE) {
		msg << "TraciClient::submission. Incorrect parameters. dom: " << (u_int8_t)dom << ", variables size: " << variables.size();
		errorMsg(msg);
		return;
	}

	tcpip::Storage outMsg, inMsg;
	std::string s = "*";

	if (variables.empty()) {
		variables = vector<u_int8_t>();
		variables.push_back(VAR_TIME_STEP); // Returns the current simulation time (in ms)
		variables.push_back(VAR_DEPARTED_VEHICLES_IDS); // A list of ids of vehicles which departed (were inserted into the road network) in this time step.
		variables.push_back(VAR_ARRIVED_VEHICLES_IDS); // A list of ids of vehicles which arrived (have reached their destination and are removed from the road network) in this time step
		//variables.push_back(VAR_LOADED_VEHICLES_NUMBER); // The number of vehicles which were loaded in this time step.
		//variables.push_back(VAR_WAITING_VEHICLES_NUMBER);
		//variables.push_back(VAR_DEPARTED_VEHICLES_NUMBER); // The number of vehicles which depared in this time step.
		//variables.push_back(VAR_ARRIVED_VEHICLES_NUMBER); // The number of vehicles which arrived in this time step.

	}
	uint8_t variablesCount = variables.size();
	outMsg.writeUnsignedByte(0);
	outMsg.writeInt(1 + 4 + 1 + 4 + 4 + 4 + (int) s.length() + 1 + variablesCount);
	//outMsg.writeUnsignedByte(1 + 1 + 4 + 4 + 4 + (int) s.length() + 1 + variablesCount);

	outMsg.writeUnsignedByte(dom);
	outMsg.writeInt((start)); // Time: the subscription is executed only in time steps >= this value; in ms
	outMsg.writeInt((stop)); // Time: the subscription is executed in time steps <= this value; the subscription is removed if the simulation has reached a higher time step; in ms
	outMsg.writeString(s);
	outMsg.writeUnsignedByte(variablesCount);
	for (vector<u_int8_t>::iterator i = variables.begin(); i != variables.end(); ++i) {
		outMsg.writeUnsignedByte((int)*i);
	}
	// send request message
	try {
		socket->sendExact(outMsg);
	}
	catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
		if (!reportResultState(inMsg, dom)) {
			return;
		}
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// don't validate anything from the answer
	//validateSubscribeSimulationResponse(inMsg);
}

void TraciClient::validateSubscribeSimulationResponse(tcpip::Storage & inMsg) {
	uint8_t cmdLength;
	uint8_t command = CMD_SUBSCRIBE_SIM_VARIABLE;
	uint8_t cmdId;
	uint8_t resultType;
	uint8_t cmdStart;
	string statusMsg;
	uint32_t pos = inMsg.position();
	cout << "pos: " << pos << endl;
	int length = inMsg.readInt();
	cout << "length: " << length << endl;
	int lengthExt = inMsg.readUnsignedByte();
	cout << "lengthExt: " << lengthExt << endl;
	int varNo2 = inMsg.readUnsignedByte();
	cout << "cmdID: " << varNo2 << endl;
	string objectId = inMsg.readString();
	cout << "objectId: " << objectId << endl;
	int varNo7 = inMsg.readUnsignedByte();
	cout << "varNo : " << varNo7 << endl;
	int varNo8 = inMsg.readUnsignedByte();
	cout << "var1 id: " << varNo8 << endl;
	int varNo3 = inMsg.readUnsignedByte();
	cout << "var1 status: " << varNo3 << endl;
	int varNo4 = inMsg.readUnsignedByte();
	cout << "var1 type: " << varNo4 << endl;
	int intNo2 = inMsg.readInt();
	cout << "var1: " << intNo2 << endl;
	int varNo10 = inMsg.readUnsignedByte();
	cout << "var2 id: " << varNo10 << endl;
	int varNo11 = inMsg.readUnsignedByte();
	cout << "var2 status: " << varNo11 << endl;
	int varNo12 = inMsg.readUnsignedByte();
	cout << "var2 type: " << varNo12 << endl;
}


/**
 * If TargetTime is 0 (zero), SUMO performs exactly one time step.
 * Otherwise SUMO performs the simulation until the given time step is reached.
 * If the given time step is smaller than the simulation step then SUMO performs one simulation step
 */
bool TraciClient::simulationStep(int targetTime, int & time, std::vector<std::string> & in, std::vector<std::string> & out) {
	tcpip::Storage outMsg;
	tcpip::Storage inMsg;
	std::stringstream msg;

	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return false;
	}
	// command length
	outMsg.writeUnsignedByte(1 + 1 + 4);
	// extended command length
//	outMsg.writeUnsignedByte(0);
//	outMsg.writeInt(1 + 4 + 1+ 4);
	// command id
	outMsg.writeUnsignedByte(CMD_SIMSTEP2);
	outMsg.writeInt(targetTime);
	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "--Error while sending command: " << e.what();
		errorMsg(msg);
		return false;
	}
	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	uint8_t command = CMD_SIMSTEP2;

	// validate result state
	if (!reportResultState(inMsg, command)) {
		return false;
	}
	try {
		int noSubscriptions = inMsg.readInt();
		cout << "received response to " << (int)command << ", noSubscriptions: " << noSubscriptions << endl;
		for (int s = 0; s < noSubscriptions; ++s) {
			try {
				uint32_t respStart = inMsg.position();
				int extLength = inMsg.readUnsignedByte();
				int respLength = inMsg.readInt();
				int cmdId = inMsg.readUnsignedByte();
				if (cmdId < 0xe0 || cmdId > 0xef) {
					NS_LOG_DEBUG( "#Error: received response with command id: " << cmdId << " but expected a subscription response (0xe0-0xef)" << endl);
					return false;
				}
				NS_LOG_DEBUG( "  CommandID=" << cmdId);
				string oId = inMsg.readString();
				NS_LOG_DEBUG("  ObjectID=" << oId);
				unsigned int varNo = inMsg.readUnsignedByte();
				NS_LOG_DEBUG( "  #variables=" << varNo << endl);

				if (cmdId == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE) {
					for (int i = 0; i < varNo; ++i) {
						responseVehicleSubscriptionHandle(inMsg, time);
					}
				}
				else if (cmdId == RESPONSE_SUBSCRIBE_SIM_VARIABLE) {
					for (int i = 0; i < varNo; ++i) {
						responseSimulationSubscriptionHandle(inMsg, time, in, out);
					}
				}
				else {
					for (uint32_t i = 0; i < varNo; ++i) {
						int vId = inMsg.readUnsignedByte();
						NS_LOG_DEBUG( "      VariableID=" << vId);
						bool ok = inMsg.readUnsignedByte() == RTYPE_OK;
						NS_LOG_DEBUG( "      ok=" << ok);
						int valueDataType = inMsg.readUnsignedByte();
						NS_LOG_DEBUG( " valueDataType=" << valueDataType);
						readAndReportTypeDependent(inMsg, valueDataType);
					}
				}
			}
			catch (exception  e) {
				NS_LOG_DEBUG( "#Error while reading message:" << e.what() << endl);
				return false;
			}
		}
	}
	catch (exception & e) {
		NS_LOG_DEBUG( "#Error while reading message:" << e.what() << endl);
		return false;
	}
	return true;
}

void TraciClient::responseVehicleSubscriptionHandle(tcpip::Storage & inMsg, int time) {
	int varId = inMsg.readUnsignedByte();
	NS_LOG_DEBUG( "      VariableID=" << varId);
	if (inMsg.readUnsignedByte() != RTYPE_OK)
	{
		return;
	}
	int valueDataType = inMsg.readUnsignedByte();
	if (varId == VAR_ROAD_ID) {

	}
	else if (varId == ID_LIST) {
//		vector<string> s = inMsg.readStringList();
//		NS_LOG_DEBUG( " string list value: [ " << endl);
//		for (vector<string>::iterator i = s.begin(); i != s.end(); ++i) {
//			if (i != s.begin()) {
//				NS_LOG_DEBUG(", ");
//			}
//			NS_LOG_DEBUG( "'" << *i);
//		}
//		NS_LOG_DEBUG( " ]" << endl);
//		int runningVehiclesCount = s.size();
//		//ovnis::Log::getInstance().logIn(VEHICLES_RUNNING, runningVehiclesCount, time);
	}
	// DOMVAR_COUNT never is a variable here
//	else if (varId == DOMVAR_COUNT) {
//		int runningVehiclesCount = inMsg.readInt();
//		//ovnis::Log::getInstance().logIn(VEHICLES_RUNNING, runningVehiclesCount, time);
//	}
}

void TraciClient::responseSimulationSubscriptionHandle(tcpip::Storage & inMsg, int & time, std::vector<std::string> & in, std::vector<std::string> & out) {
	int varId = inMsg.readUnsignedByte();
	NS_LOG_DEBUG( "      VariableID=" << varId);
	if (inMsg.readUnsignedByte() != RTYPE_OK)
	{
		return;
	}
	int valueDataType = inMsg.readUnsignedByte();

	//----------  Get the list of vehicle that entered the simulation.
	if (varId == VAR_DEPARTED_VEHICLES_IDS) {
		vector<string> s = inMsg.readStringList();
		NS_LOG_DEBUG( " string list value: [ " << endl);
		for (vector<string>::iterator i = s.begin(); i != s.end(); ++i) {
			if (i != s.begin()) { NS_LOG_DEBUG( ", "); }
			NS_LOG_DEBUG( "'" << *i);
			in.push_back((*i));
//			if (rand() % 100 > 80) {
//				silentChangeRoute(*i, "middle", (double) INT_MAX);
//				NS_LOG_DEBUG( "(changed)");
//			}
		}
		NS_LOG_DEBUG( " ]" << endl);
	}
	//-------------- Get the list of vehicles that finished their trip and got out of the simulation.
	else if (varId == VAR_ARRIVED_VEHICLES_IDS) {
		vector<string> s = inMsg.readStringList();
		NS_LOG_DEBUG( " string list value: [ " << endl);
		for (vector<string>::iterator i = s.begin(); i != s.end(); ++i) {
			if (i != s.begin()) { NS_LOG_DEBUG( ", "); }
			NS_LOG_DEBUG( "'" << *i);
			out.push_back((*i));
			//			if (rand() % 100 > 80) {
			//				silentChangeRoute(*i, "middle", (double) INT_MAX);
			//				NS_LOG_DEBUG( "(changed)");
			//			}
		}
		NS_LOG_DEBUG( " ]" << endl);
	}
	//----------- Get The simulation time step.
	else if (varId == VAR_TIME_STEP) {
		time = inMsg.readInt();
		//cout << " Time value (s): " << time << endl;
		NS_LOG_DEBUG( " Time value (s): " << time << endl);
	}
	else if (varId == VAR_LOADED_VEHICLES_NUMBER) {
		//ovnis::Log::getInstance().logIn(VEHICLES_LOADED, inMsg.readInt(), time);
	}
	else if (varId == VAR_WAITING_VEHICLES_NUMBER) {
		//ovnis::Log::getInstance().logIn(VEHICLES_WAITING, inMsg.readInt(), time);
	}
	else if (varId == VAR_DEPARTED_VEHICLES_NUMBER) {
		//ovnis::Log::getInstance().logIn(VEHICLES_DEPARTURED, inMsg.readInt(), time);
	}
	else if (varId == VAR_ARRIVED_VEHICLES_NUMBER) {
		//ovnis::Log::getInstance().logIn(VEHICLES_ARRIVED, inMsg.readInt(), time);
	}
	else {
		readAndReportTypeDependent(inMsg, valueDataType);
	}
}

double TraciClient::getDouble(u_int8_t dom, u_int8_t cmd, const std::string & nodeId) {
	tcpip::Storage outMsg, inMsg;
	std::stringstream msg;
	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + (int) nodeId.length()));
	outMsg.writeUnsignedByte(dom);
	outMsg.writeUnsignedByte(cmd);
	outMsg.writeString(nodeId);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "+++Error while sending command: " << e.what();
		errorMsg(msg);
		return false;
	}
	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	// validate result state
	if (!reportResultState(inMsg, dom)) {
		return false;
	}
	// validate answer message
	try {
		uint32_t respStart = inMsg.position();
		uint32_t extLength = inMsg.readUnsignedByte();
		//uint32_t respLength = inMsg.readInt();
		uint32_t cmdId = inMsg.readUnsignedByte();
		if (cmdId != (dom + 0x10)) {
			NS_LOG_DEBUG( "#Error: received response with command id: " << cmdId << "but expected: " << (int) (dom + 0x10) << endl);
			return false;
		} NS_LOG_DEBUG( "  CommandID=" << cmdId);
		int vId = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( "  VariableID=" << vId);
		string oId = inMsg.readString();
		NS_LOG_DEBUG( "  ObjectID=" << oId);
		int valueDataType = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( " valueDataType=" << valueDataType);
		if (valueDataType == TYPE_DOUBLE) {
			double result = inMsg.readDouble();
			NS_LOG_DEBUG( " double value:  " << result << endl);
			return result;
		} else {
			return 0;
		}
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	return 0;

}

//int TraciClient::getInt2(u_int8_t dom, u_int8_t cmd, const std::string & node) {
//	tcpip::Storage outMsg, inMsg;
//	std::stringstream msg;
//	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + (int) node.length()));
//	outMsg.writeUnsignedByte(dom);
//	outMsg.writeUnsignedByte(cmd);
//	outMsg.writeString(node);
//	// send request message
//	try {
//		socket->sendExact(outMsg);
//	} catch (SocketException &e) {
//		msg << "+++Error while sending command: " << e.what();
//		errorMsg(msg);
//		return false;
//	}
//	// receive answer message
//	try {
//		socket->receiveExact(inMsg);
//	} catch (SocketException &e) {
//		msg << "Error while receiving command: " << e.what();
//		errorMsg(msg);
//		return false;
//	}
//	// validate result state
//	if (!reportResultState(inMsg, dom)) {
//		return false;
//	}
//	// validate answer message
//	try {
//		uint32_t respStart = inMsg.position();
//		uint32_t extLength = inMsg.readUnsignedByte();
//		uint32_t respLength = inMsg.readInt();
//		uint32_t cmdId = inMsg.readUnsignedByte();
//		if (cmdId != (dom + 0x10)) {
//			NS_LOG_DEBUG( "#Error: received response with command id: " << cmdId << "but expected: " << (int) (dom + 0x10) << endl);
//			return false;
//		}
//		NS_LOG_DEBUG( "  CommandID=" << cmdId);
//		int vId = inMsg.readUnsignedByte();
//		NS_LOG_DEBUG( "  VariableID=" << vId);
//		string oId = inMsg.readString();
//		NS_LOG_DEBUG( "  ObjectID=" << oId);
//		int valueDataType = inMsg.readUnsignedByte();
//		NS_LOG_DEBUG( " valueDataType=" << valueDataType);
//		if (valueDataType == TYPE_INTEGER) {
//			int result = inMsg.readInt();
//			NS_LOG_DEBUG( " float value:  " << result << endl);
//			return result;
//		} else {
//			return 0;
//		}
//	} catch (SocketException &e) {
//		msg << "Error while receiving command: " << e.what();
//		errorMsg(msg);
//		return false;
//	}
//	return 0;
//}

int TraciClient::getInt(u_int8_t dom, u_int8_t cmd, const std::string & node) {
	// build command
	tcpip::Storage out;
	// Length of command
	out.writeUnsignedByte(16);
	out.writeUnsignedByte(dom);
	out.writeUnsignedByte(cmd);
	out.writeString(node);
	// Send command
	try
	{
		socket->sendExact(out);
	}
	catch (SocketException & e)
	{
		cerr << "Error in method MobilityInterfaceClient::commandGetRoadID while sending: " << e.what() << endl;
		abort();
	}
	tcpip::Storage in;
	try
	{
		socket->receiveExact(in);
	}
	catch (SocketException & e)
	{
		cerr << "Error in method MobilityInterfaceClient::commandGetRoadID while receiving: " << e.what() << endl;
		abort();
	}
	// First result command serves as status information
	string description;
	bool nodeExist = false;
	nodeExist = extractCommandStatus(in, dom, description);
	if ( nodeExist == true ) {
		int UnusedData = in.readInt();
		int comLength = in.readUnsignedByte();
		int comID = in.readUnsignedByte();
		int dataIdentifier  = in.readUnsignedByte();
		string nodeID = in.readString();
		int returnDataIdentifier = in.readUnsignedByte();
		//string roadID = in.readString();
		int result = in.readInt();
		cout << " node: " << node << " running vehicles: " << result << endl;
	}
	return true;
}


bool TraciClient::getString(u_int8_t dom, u_int8_t cmd, const std::string & node, std::string & value) {
	tcpip::Storage outMsg, inMsg;
	std::stringstream msg;
	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + (int) node.length()));
	outMsg.writeUnsignedByte(dom);
	outMsg.writeUnsignedByte(cmd);
	outMsg.writeString(node);
	// send request message

	if (socket->port() == 0) {

		std::cerr << "Error while sending command: no connection to server";
		std::flush(std::cerr);

	}
	try {
		socket->sendExact(outMsg);

	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return false;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	// validate result state
	if (!reportResultState(inMsg, dom)) {
		return false;
	}

	// validate answer message
	try {
		int respStart = inMsg.position();
		int extLength = inMsg.readUnsignedByte();
		int respLength = inMsg.readInt();
		int cmdId = inMsg.readUnsignedByte();
		if (cmdId != (dom + 0x10)) {
			NS_LOG_DEBUG( "#Error: received response with command id: " << cmdId << "but expected: " << (int) (dom
							+ 0x10) << endl);
			return false;
		} NS_LOG_DEBUG( "  CommandID=" << cmdId);
		int vId = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( "  VariableID=" << vId);
		string oId = inMsg.readString();
		NS_LOG_DEBUG( "  ObjectID=" << oId);
		int valueDataType = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( " valueDataType=" << valueDataType);

		//data should be string list
		//          readAndReportTypeDependent(inMsg, valueDataType);
		if (valueDataType == TYPE_STRING) {
			value.assign(inMsg.readString());
			NS_LOG_DEBUG( " string value:  " <<value<< std::endl);
			return true;
		} else {

			return 0;
		}

	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	return 0;

}

bool TraciClient::extractCommandStatus(tcpip::Storage & inMsg, u_int8_t cmd, string & description)
{
	u_int8_t commandStart = inMsg.position();
	u_int8_t commandLength = inMsg.readUnsignedByte();
	u_int8_t rcvdCommandId = inMsg.readUnsignedByte();
	// CommandID needs to fit
	if (rcvdCommandId != cmd)
	{
      cerr << ": Error in method TraCIClient::extractCommandStatus, Server answered to command: " << rcvdCommandId << ". Expected command: " << cmd << std::endl;
		abort();
	}
	// Get result and description
	unsigned int result = inMsg.readUnsignedByte();
	description = inMsg.readString();
	if (result != RTYPE_OK)
	{
		return false;
	}
	// Right length?
	cout << "commandLength: " << (int)commandLength << endl;
	if ((commandStart + commandLength) != inMsg.position())
	{
		// Last read command has wrong length
      cerr << ": Error in method TraCIClient::extractCommandStatus, command at position "
			 << commandStart << " was read with wrong length." << endl;
      return false;
	}
	return true;
}

bool TraciClient::getStringList(u_int8_t dom, u_int8_t cmd, const std::string & node, std::vector<std::string>& list) {
	tcpip::Storage outMsg, inMsg;
	std::stringstream msg;
	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + (int) node.length()));
	outMsg.writeUnsignedByte(dom);
	outMsg.writeUnsignedByte(cmd);
	outMsg.writeString(node);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return false;
	}
	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	// validate result state
	if (!reportResultState(inMsg, dom)) {
		return false;
	}
	// validate answer message
	try {
		int respStart = inMsg.position();
		int extLength = inMsg.readUnsignedByte();
		int respLength = inMsg.readInt();
		int cmdId = inMsg.readUnsignedByte();
		if (cmdId != (dom + 0x10)) {
			NS_LOG_DEBUG( "#Error: received response with command id: " << cmdId << "but expected: " << (int) (dom
							+ 0x10) << endl);
			return false;
		} NS_LOG_DEBUG( "  CommandID=" << cmdId);
		int vId = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( "  VariableID=" << vId);
		string oId = inMsg.readString();
		NS_LOG_DEBUG( "  ObjectID=" << oId);
		int valueDataType = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( " valueDataType=" << valueDataType);

		//data should be string list
		//          readAndReportTypeDependent(inMsg, valueDataType);
		if (valueDataType == TYPE_STRINGLIST) {
//			vector<string> s = inMsg.readStringList();
//			NS_LOG_DEBUG( " string list value: [ " << endl);
//			for (vector<string>::iterator i = s.begin(); i != s.end(); ++i) {
//				if (i != s.begin()) {
//					NS_LOG_DEBUG( ", ");
//				}
//				//                          std::cout<<(*i)<<endl;
//				list.push_back((*i));
//				NS_LOG_DEBUG( '"' << *i << '"');
//			} NS_LOG_DEBUG( " ]" << endl);
		} else {

			return false;
		}

	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	return true;
}

bool TraciClient::readAndReportTypeDependent(tcpip::Storage &inMsg,int valueDataType) {
	if (valueDataType == TYPE_UBYTE) {
		int ubyte = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( " Unsigned Byte Value: " << ubyte << endl);
	} else if (valueDataType == TYPE_BYTE) {
		int byte = inMsg.readByte();
		NS_LOG_DEBUG( " Byte value: " << byte << endl);
	} else if (valueDataType == TYPE_INTEGER) {
		int integer = inMsg.readInt();
		NS_LOG_DEBUG( " Int value: " << integer << endl);
	} else if (valueDataType == TYPE_FLOAT) {
		float floatv = inMsg.readFloat();
		//        if (floatv < 0.1 && floatv > 0)
		//          {
		//            answerLog.setf(std::ios::scientific, std::ios::floatfield);
		//          }
		NS_LOG_DEBUG( " float value: " << floatv << endl);
	} else if (valueDataType == TYPE_DOUBLE) {
		double doublev = inMsg.readDouble();
		NS_LOG_DEBUG( " Double value: " << doublev << endl);
	} else if (valueDataType == TYPE_BOUNDINGBOX) {
		BoundingBox box;
		box.lowerLeft.x = inMsg.readFloat();
		box.lowerLeft.y = inMsg.readFloat();
		box.upperRight.x = inMsg.readFloat();
		box.upperRight.y = inMsg.readFloat();
		NS_LOG_DEBUG( " BoundaryBoxValue: lowerLeft x=" << box.lowerLeft.x
				<< " y=" << box.lowerLeft.y << " upperRight x="
				<< box.upperRight.x << " y=" << box.upperRight.y << endl);
	} else if (valueDataType == TYPE_POLYGON) {
		int length = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( " PolygonValue: ");
		for (int i = 0; i < length; i++) {
			float x = inMsg.readFloat();
			float y = inMsg.readFloat();
			NS_LOG_DEBUG( "(" << x << "," << y << ") ");
		} NS_LOG_DEBUG( endl);
	} else if (valueDataType == POSITION_3D) {
		float x = inMsg.readFloat();
		float y = inMsg.readFloat();
		float z = inMsg.readFloat();
		NS_LOG_DEBUG( " Position3DValue: " << std::endl); NS_LOG_DEBUG( " x: " << x << " y: " << y << " z: " << z << std::endl);
	} else if (valueDataType == POSITION_ROADMAP) {
		std::string roadId = inMsg.readString();
		float pos = inMsg.readFloat();
		int laneId = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( " RoadMapPositionValue: roadId=" << roadId << " pos="
				<< pos << " laneId=" << laneId << std::endl);
	} else if (valueDataType == TYPE_TLPHASELIST) {
		int length = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( " TLPhaseListValue: length=" << length << endl);
		for (int i = 0; i < length; i++) {
			string pred = inMsg.readString();
			string succ = inMsg.readString();
			int phase = inMsg.readUnsignedByte();
			NS_LOG_DEBUG( " precRoad=" << pred << " succRoad=" << succ
					<< " phase=");
			switch (phase) {
			case TLPHASE_RED:
				NS_LOG_DEBUG( "red" << endl);
				break;
			case TLPHASE_YELLOW:
				NS_LOG_DEBUG("yellow" << endl);
				break;
			case TLPHASE_GREEN:
				NS_LOG_DEBUG( "green" << endl);
				break;
			default:
				NS_LOG_DEBUG( "#Error: unknown phase value" << (int) phase
						<< endl);
				return false;
			}
		}
	} else if (valueDataType == TYPE_STRING) {
		string s = inMsg.readString();
		NS_LOG_DEBUG( " string value: " << s << endl);
	} else if (valueDataType == TYPE_STRINGLIST) {
//		vector<string> s = inMsg.readStringList();
//		NS_LOG_DEBUG( " string list value: [ " << endl);
//		for (vector<string>::iterator i = s.begin(); i != s.end(); ++i) {
//			if (i != s.begin()) {
//				NS_LOG_DEBUG( ", ");
//			} NS_LOG_DEBUG( '"' << *i << '"');
//		} NS_LOG_DEBUG( " ]" << endl);
	} else if (valueDataType == TYPE_COMPOUND) {
		int no = inMsg.readInt();
		NS_LOG_DEBUG( " compound value with " << no << " members: [ " << endl);
		for (int i = 0; i < no; ++i) {
			int currentValueDataType = inMsg.readUnsignedByte();
			NS_LOG_DEBUG( " valueDataType=" << currentValueDataType);
			readAndReportTypeDependent(inMsg, currentValueDataType);
		} NS_LOG_DEBUG( " ]" << endl);
	} else if (valueDataType == POSITION_2D) {
		float xv = inMsg.readFloat();
		float yv = inMsg.readFloat();
		NS_LOG_DEBUG( " position value: (" << xv << "," << yv << ")" << endl);
	} else if (valueDataType == TYPE_COLOR) {
		int r = inMsg.readUnsignedByte();
		int g = inMsg.readUnsignedByte();
		int b = inMsg.readUnsignedByte();
		int a = inMsg.readUnsignedByte();
		NS_LOG_DEBUG(" color value: (" << r << "," << g << "," << b << "," << a
				<< ")" << endl);
	} else {
		NS_LOG_DEBUG( "#Error: unknown valueDataType!" << endl);
		return false;
	}
	return true;
}

Position2D TraciClient::getPosition2D(std::string &veh)

{
	Position2D p;
	tcpip::Storage outMsg, inMsg;
	std::stringstream msg;
	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return p;
	}
	// command length
	outMsg.writeUnsignedByte(1 + 1 + 1 + 4 + (int) veh.length());
	// command id
	outMsg.writeUnsignedByte(CMD_GET_VEHICLE_VARIABLE);
	// variable id
	outMsg.writeUnsignedByte(VAR_POSITION);
	// object id
	outMsg.writeString(veh);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return p;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
		if (!reportResultState(inMsg, CMD_GET_VEHICLE_VARIABLE)) {
			return p;
		}
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return p;
	}
	// validate result state
	try {
		uint32_t respStart = inMsg.position();
		int extLength = inMsg.readUnsignedByte();
		int respLength = inMsg.readInt();
		int cmdId = inMsg.readUnsignedByte();
		if (cmdId != (CMD_GET_VEHICLE_VARIABLE + 0x10)) {
			NS_LOG_DEBUG( "#Error: received response with command id: " << cmdId
					<< "but expected: " << (int) (CMD_GET_VEHICLE_VARIABLE
							+ 0x10) << endl);
			return p;
		}
		//  VariableID=" <<
		inMsg.readUnsignedByte();
		//answerLog << "  ObjectID=" <<
		inMsg.readString();
		//int valueDataType =
		inMsg.readUnsignedByte();

		p.x = inMsg.readFloat();
		p.y = inMsg.readFloat();
		//answerLog << xv << "," << yv << endl;
		return p;
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return p;
	}

}

// XXXXXXXXXXXXXXXXXXX
void TraciClient::changeRoute(std::string nodeId,
		std::vector<std::string> stringList) {
	tcpip::Storage outMsg;
	tcpip::Storage inMsg;
	std::stringstream msg;

	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return;
	}

	int size_of_strings = 0;
	for (std::vector<std::string>::iterator i = stringList.begin();
			i != stringList.end(); ++i) {
		size_of_strings += (4 + (*i).size());
	}
	// command length
	outMsg.writeUnsignedByte(
			1 + 1 + 1 + (4 + (int) nodeId.length()) + 1 + 4 + size_of_strings);
	// command id
	outMsg.writeUnsignedByte(CMD_SET_VEHICLE_VARIABLE);

	outMsg.writeUnsignedByte(VAR_ROUTE);
	// vehicle id
	outMsg.writeString(nodeId);
	//type of value (string list) : byte
	outMsg.writeUnsignedByte(TYPE_STRINGLIST);

	//number of edges in the route : int
	outMsg.writeInt(stringList.size());

	for (std::vector<std::string>::iterator i = stringList.begin();
			i != stringList.end(); ++i) {
		outMsg.writeString((*i));
	}

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// validate result state
	if (!reportResultState(inMsg, CMD_SET_VEHICLE_VARIABLE)) {
		return;
	}
}

void TraciClient::changeRoad(std::string nodeId, std::string roadId, double travelTime) {
	tcpip::Storage outMsg;
	tcpip::Storage inMsg;
	std::stringstream msg;

	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return;
	}

	// command length
	outMsg.writeUnsignedByte(
			1 + 1 + 1 + (4 + (int) nodeId.length()) + 1 + 4 + (1 + 4) + (1 + 4)
					+ (1 + (4 + (int) roadId.length())) + (1 + 4));
	// command id
	outMsg.writeUnsignedByte(CMD_SET_VEHICLE_VARIABLE);
	// var id VAR_EDGE_TRAVELTIME
	outMsg.writeUnsignedByte(VAR_EDGE_TRAVELTIME);
	// vehicle id
	outMsg.writeString(nodeId);
	//type of value (compound)
	outMsg.writeUnsignedByte(TYPE_COMPOUND);

	// compoung value for edge travael time;
	//number of elements (always=4)
	outMsg.writeInt(4);

	//int begin
	outMsg.writeUnsignedByte(TYPE_INTEGER);
	outMsg.writeInt(0);
	//int end
	outMsg.writeUnsignedByte(TYPE_INTEGER);
	outMsg.writeInt(INT_MAX);
	//string edge
	outMsg.writeUnsignedByte(TYPE_STRING);
	outMsg.writeString(roadId);
	//float value
	outMsg.writeUnsignedByte(TYPE_FLOAT);
	outMsg.writeFloat(travelTime);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// validate result state
	if (!reportResultState(inMsg, CMD_SET_VEHICLE_VARIABLE)) {
		return;
	}

	outMsg.reset();
	inMsg.reset();

	// command length
	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + (int) nodeId.length()) + 1 + 4);
	// command id
	outMsg.writeUnsignedByte(CMD_SET_VEHICLE_VARIABLE);
	// var id VAR_EDGE_TRAVELTIME
	outMsg.writeUnsignedByte(CMD_REROUTE_TRAVELTIME);
	// vehicle id
	outMsg.writeString(nodeId);
	//type of value (compound)
	outMsg.writeUnsignedByte(TYPE_COMPOUND);

	// Compound value for reroute on  travel time;
	//number of elements (always=4)
	outMsg.writeInt(0);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// validate result state
	if (!reportResultState(inMsg, CMD_SET_VEHICLE_VARIABLE)) {
		return;
	}

}

void TraciClient::crash(std::string &nodeId) {
	tcpip::Storage outMsg;
	tcpip::Storage inMsg;
	std::stringstream msg;

	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return;
	}

	// command length
	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + (int) nodeId.length()) + 1 + 8);
	// command id
	outMsg.writeUnsignedByte(CMD_SET_VEHICLE_VARIABLE);
	// var id VAR_EDGE_TRAVELTIME
	outMsg.writeUnsignedByte(VAR_SPEED);
	// vehicle id
	outMsg.writeString(nodeId);
	// Type of value (double)
	outMsg.writeUnsignedByte(TYPE_DOUBLE);
	outMsg.writeDouble(0.0001);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// validate result state
	if (!reportResultState(inMsg, CMD_SET_VEHICLE_VARIABLE)) {
		return;
	}

	outMsg.reset();
	inMsg.reset();

	changeColor(nodeId, 0, 0, 0);

}

void TraciClient::changeColor(std::string &nodeId, int r, int g, int b) {
	tcpip::Storage outMsg;
	tcpip::Storage inMsg;
	std::stringstream msg;

	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return;
	}
	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + (int) nodeId.length()) + 1 + 4);
	// command id
	outMsg.writeUnsignedByte(CMD_SET_VEHICLE_VARIABLE);
	// var id VAR_EDGE_TRAVELTIME
	outMsg.writeUnsignedByte(VAR_COLOR);
	// vehicle id
	outMsg.writeString(nodeId);
	// Type of value (double)
	outMsg.writeUnsignedByte(TYPE_COLOR);
	outMsg.writeUnsignedByte(r);
	outMsg.writeUnsignedByte(g);
	outMsg.writeUnsignedByte(b);
	outMsg.writeUnsignedByte(1);
	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// validate result state
	if (!reportResultState(inMsg, CMD_SET_VEHICLE_VARIABLE)) {
		return;
	}
}

// agata

void TraciClient::closeRoad(std::string edgeId, int begin, int end) {
	tcpip::Storage outMsg;
	tcpip::Storage inMsg;
	std::stringstream msg;

	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return;
	}
	//std::cout << "edgeId: " << edgeId;
	// command length
	int rLength = (int) edgeId.length();
	outMsg.writeUnsignedByte(
			1 + 1 + 1 + (4 + rLength) + (1 + 4) + (1 + 4) + (1 + 4) + (1 + 4));

	// command id CMD_SET_EDGE_VARIABLE
	outMsg.writeUnsignedByte(CMD_SET_EDGE_VARIABLE);

	// var id change edge travel time information (0x58)
	outMsg.writeUnsignedByte(VAR_EDGE_TRAVELTIME);

	//string edgeid
	outMsg.writeString(edgeId);

	//type of value (compound)
	outMsg.writeUnsignedByte(TYPE_COMPOUND);

	//number of elements (always=3)
	outMsg.writeInt(3);

	//int begin
	outMsg.writeUnsignedByte(TYPE_INTEGER);
	outMsg.writeInt(begin);

	//int end
	outMsg.writeUnsignedByte(TYPE_INTEGER);
	outMsg.writeInt(end);

	//float value
	outMsg.writeUnsignedByte(TYPE_FLOAT);
	outMsg.writeFloat(1000);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// validate result state
	if (!reportResultState(inMsg, CMD_SET_EDGE_VARIABLE)) {
		return;
	}
}

void TraciClient::closeLane(std::string laneId) {
	tcpip::Storage outMsg;
	tcpip::Storage inMsg;
	std::stringstream msg;

	if (socket->port() == 0) {
		msg << "#Error while sending command: no connection to server";
		errorMsg(msg);
		return;
	}
	//std::cout << "laneId: " << laneId;
	// command length
	int rLength = (int) laneId.length();
	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + rLength) + (1 + 4));

	// command id CMD_SET_EDGE_VARIABLE
	outMsg.writeUnsignedByte(CMD_SET_LANE_VARIABLE);

	// var id change edge travel time information (0x58)
	outMsg.writeUnsignedByte(VAR_MAXSPEED);

	//string laneId
	outMsg.writeString(laneId);

	//type of value (float)
	outMsg.writeUnsignedByte(TYPE_FLOAT);

	//int maxspeed
	outMsg.writeFloat(0);

	// send request message
	try {
		socket->sendExact(outMsg);
	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return;
	}

	// validate result state
	if (!reportResultState(inMsg, CMD_SET_LANE_VARIABLE)) {
		return;
	}
}

bool TraciClient::getEdgeInfo(u_int8_t dom, u_int8_t cmd, const std::string & node, std::string & value) {
	tcpip::Storage outMsg, inMsg;
	std::stringstream msg;
	outMsg.writeUnsignedByte(1 + 1 + 1 + (4 + (int) node.length()));
	outMsg.writeUnsignedByte(dom);
	outMsg.writeUnsignedByte(cmd);
	outMsg.writeString(node);
	// send request message

	if (socket->port() == 0) {

		std::cerr << "Error while sending command: no connection to server";
		std::flush(std::cerr);

	}
	try {
		socket->sendExact(outMsg);

	} catch (SocketException &e) {
		msg << "Error while sending command: " << e.what();
		errorMsg(msg);
		return false;
	}

	// receive answer message
	try {
		socket->receiveExact(inMsg);
	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	// validate result state
	if (!reportResultState(inMsg, dom)) {
		return false;
	}

	// validate answer message
	try {
		int respStart = inMsg.position();
		int extLength = inMsg.readUnsignedByte();
		int respLength = inMsg.readInt();
		int cmdId = inMsg.readUnsignedByte();
		if (cmdId != (dom + 0x10)) {
			NS_LOG_DEBUG( "#Error: received response with command id: " << cmdId << "but expected: " << (int) (dom
							+ 0x10) << endl);
			return false;
		} NS_LOG_DEBUG( "  CommandID=" << cmdId);
		int vId = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( "  VariableID=" << vId);
		string oId = inMsg.readString();
		NS_LOG_DEBUG( "  ObjectID=" << oId);
		int valueDataType = inMsg.readUnsignedByte();
		NS_LOG_DEBUG( " valueDataType=" << valueDataType);

		//data should be string list
		//          readAndReportTypeDependent(inMsg, valueDataType);
		if (valueDataType == TYPE_STRING) {
			value.assign(inMsg.readString());
			NS_LOG_DEBUG( " string value:  " <<value<< std::endl);
			return true;
		} else {

			return 0;
		}

	} catch (SocketException &e) {
		msg << "Error while receiving command: " << e.what();
		errorMsg(msg);
		return false;
	}
	return 0;

}

}
