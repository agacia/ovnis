/*
 * simStepQuery.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#include "simStepQuery.h"

using namespace std;
using namespace tcpip;

namespace ovnis {

SimStepQuery::SimStepQuery() :
		Query() {
}

SimStepQuery::SimStepQuery(Socket * socket, int time) :
		Query(socket), currentTime(time) {
}

SimStepQuery::~SimStepQuery() {
}


void SimStepQuery::InitializeCommand(Command & command) {
	command = Command(CMD_SIMSTEP2);
	command.WriteHeader(sizeof(int));
	command.Content().writeInt(currentTime);
}

int SimStepQuery::GetSubscriptionCount() {
	return subscriptionCount;
}

void SimStepQuery::ReadResponse(Storage & content) {
	try {
		subscriptionCount = content.readInt();
	}
	catch (invalid_argument & e) {
		cout << "#Error while reading message:" << e.what() << endl;
		return;
	}
	for (int s = 0; s < subscriptionCount; ++s) {
		try {
			int position = content.position();
			int extLength = content.readUnsignedByte();
			int length = content.readInt();
			int commandId = content.readUnsignedByte();
			if (commandId < 0xe0 || commandId > 0xef) {
				cout << "#Error: received response with command id: " << commandId << " but expected a subscription response (0xe0-0xef)" << endl;
				return;
			}
			string objectId = content.readString();
			int varCount = content.readUnsignedByte();
			if (commandId == RESPONSE_SUBSCRIBE_SIM_VARIABLE) {
				for (int i = 0; i < varCount; ++i) {
					ReadSimulationSubscriptionResponse(content);
				}
			} else if (commandId == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE) {
				for (int i = 0; i < varCount; ++i) {
					ReadVehicleSubscriptionResponse(content);
				}
			} else {
				cout << "Received unhandled response in SimStep: " << commandId << endl;
			}
		} catch (exception & e) {
			cout << "#Error while reading Simulation Subscription response:" << e.what() << endl;
		}
	}
}

void SimStepQuery::ReadSimulationSubscriptionResponse(Storage & content) {
	int varId = content.readUnsignedByte();
	int status = content.readUnsignedByte();
	if (status != RTYPE_OK) {
		cout << "Wrong Simulation Subscription response, variable id: " << varId << endl;
		return;
	}
	int valueDataType = content.readUnsignedByte();
	// Get the list of vehicle that entered the simulation
	if (varId == VAR_DEPARTED_VEHICLES_IDS) {
		departedVehicles = content.readStringList();
//		departedVehicles.clear();
//		vector<string> vehicles = content.readStringList();
//		for (vector<string>::iterator i = vehicles.begin(); i != vehicles.end(); ++i) {
//			cout << " " << *i << endl;
//			departedVehicles.push_back((*i));
//		}
	}
	// Get the list of vehicles that finished their trip and got out of the simulation
	else if (varId == VAR_ARRIVED_VEHICLES_IDS) {
		arrivedVehicles = content.readStringList();
	}
	// Get The simulation time step
	else if (varId == VAR_TIME_STEP) {
		currentTime = content.readInt();
	} else {
		cout << "Unhadled variable in Simulation subscription response: " << varId << endl;
	}
}

void SimStepQuery::ReadVehicleSubscriptionResponse(Storage & content) {

}

int SimStepQuery::GetCurrentTime() const {
	return currentTime;
}

void SimStepQuery::SimStepQuery::SetCurrentTime(const int currentTime) {
	this->currentTime = currentTime;
}

vector<string> SimStepQuery::getArrivedVehicles() const {
	return arrivedVehicles;
}

vector<string> SimStepQuery::getDepartedVehicles() const {
	return departedVehicles;
}

} /* namespace ovnis */
