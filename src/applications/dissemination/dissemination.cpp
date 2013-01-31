/*
 * dissemination.cpp
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#include "dissemination.h"

using namespace std;

namespace ovnis
{
Dissemination::Dissemination() {
	// TODO Auto-generated constructor stub

}

Dissemination::~Dissemination() {
	// TODO Auto-generated destructor stub
}

/**
 * Ahead
 */
map<string,RecordEntry> Dissemination::getTrafficInformation() {
	map<string,RecordEntry> records;
	bool isAhead = false;
//	for (vector<string>::iterator it = itinerary.getEdgeIds(); it != itinerary.getEdgeIds().end(); ++it) {
//		if (*it == itinerary.getCurrentEdge().getId()) {
//			isAhead = true;
//		}
//		map<string,RecordEntry>::iterator entry = localKnowledge.find(*it);
//		if (entry != localKnowledge.end()) {
//			records[*it] = *entry;
//		}
//	}
	return records;
}

void Dissemination::TryRebroadcast(OvnisPacket packet, double packetDate, string vehicleId) {
	double now = Simulator::Now().GetSeconds();
	// drop if was already forwarded or packet is old
//	if (vehicle.getPacketCount(packet.getPacketId()) > 1 || (now - packetDate) > PACKET_TTL) {
//		return;
//	}
//	Log::getInstance().packetForwarded();
//	Vector position = mobilityModel->GetPosition();
//	if (Log::getInstance().forwardedPackets.count(packet.getPacketId())== 0) {
//		Log::getInstance().forwardedPackets[packet.getPacketId()] = 0;
//	}
////	Log::getInstance().getStream("broadcasting") << Simulator::Now().GetSeconds() << "\t" << packet.getPacketId() << "\t" << vehicle.getId() << "\t forwarding from \t" << packet.getSenderId() << "\t vehicleId: " << packet.getVehicleId() << "\t decisionTaken: " << decisionTaken << "\t" << packet.getStringValue() <<","<< packet.getDoubleValue() << endl;
//	++Log::getInstance().forwardedPackets[packet.getPacketId()];
//	Ptr<Packet> p = OvnisPacket::BuildPacket(now, vehicle.getId(), position.x, position.y, packet.getPacketType() , packet.getPacketId(), packetDate, vehicleId, packet.getStringValue(), packet.getDoubleValue());
//	SendPacket(p);
}
}
