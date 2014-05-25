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
 * Takes only the information about requested edges
 */
vector<Data> Dissemination::getTrafficInformationToSend(Knowledge & knowledge, vector<string> edges, double ttl, double valueTh) {
	vector<Data> trafficData;
	if (knowledge.getRecords().size() > 0) {
		double now =  Simulator::Now().GetSeconds();
		for (vector<string>::iterator it = edges.begin(); it != edges.end(); ++it) {
			if (knowledge.getRecords().find(*it) != knowledge.getRecords().end()) {
				double packetDate =  knowledge.getRecords()[*it].getTime();
				if (packetDate == 0) {
//					if ((now-packetDate) > ttl) {
//						cerr << "Discarding packet about " <<*it << " packetDate " << packetDate << " now-packetDate " << (now-packetDate) << endl;
//						continue;
//					}
				}

				double packetValue = knowledge.getRecords()[*it].getValue();
//				double staticValue = TIS::getInstance().getStaticRecords()[*it].getStaticCost();

//				Log::getInstance().getStream("aaa") <<  packetValue  << "-" << staticValue << "= " <<packetValue - staticValue  << " , valueTh " << valueTh << endl;
				// with a significant change in value
//				double diff = abs(packetValue - staticValue);
//				if ( diff < 1 ||  diff < valueTh*staticValue) {
//					//Log::getInstance().getStream("aaa") <<  "discard  " << abs(packetValue - staticValue) << endl;
//					continue;
//				}
//				cerr << "Adding to packet information about edge " <<*it << " packetDate " << packetDate << " now-packetDate " << (now-packetDate) << endl;
				Data data;
				data.edgeId = *it;
				data.date = packetDate;
				data.travelTime = packetValue;
				trafficData.push_back(data);
			}
		}
//		cerr << "Sending packet about " << trafficData.size() << " edges (of " << edges.size() << " requested) , knowledge " << knowledge.getRecords().size() << endl;
	}
	return trafficData;
}

vector<Data> Dissemination::getTrafficInformationToSend(map<string,RecordEntry> edges, double ttl, double valueTh) {
	vector<Data> trafficData;
	double now =  Simulator::Now().GetSeconds();
	for (map<string, RecordEntry>::iterator it = edges.begin(); it != edges.end(); ++it) {
		double packetDate =  it->second.getTime();
		// fresher than ttl
//		if (packetDate == 0 or (now-packetDate) > ttl) {
////			cerr << "Discarding packet about " << it->first << " packetDate " << packetDate << " now-packetDate " << (now-packetDate) << ", " << trafficData.size() << " (of " << edges.size() << " requested) " << endl;
//			continue;
//		}
		double packetValue = it->second.getValue();
		double staticValue = TIS::getInstance().getStaticRecords()[it->first].getStaticCost();
		// with a significant change in value
		if (packetValue - staticValue > valueTh*staticValue) {
			continue;
		}
		Data data;
		data.edgeId = it->first;
		data.date = packetDate;
		data.travelTime = packetValue;
		trafficData.push_back(data);
	}
	return trafficData;
}

//void FceApplication::ReceiveTrafficInfoPacket(OvnisPacket ovnisPacket) {
//
//	double now =  Simulator::Now().GetSeconds();
//
//	// if heard for the first time
//	if (knowledge.getPacketCount(ovnisPacket.getPacketId()) == 1 && vehicle.getId() != vehicleId) {
//		if (!decisionTaken && (now - packetDate) < PACKET_TTL ) {
//			Log::getInstance().getStream("hearing") << ovnisPacket.getPacketType() << ", hearing about " << objectId << " " << ovnisPacket.getSenderId() <<" from " << vehicleId << " " << packetDate << " " << travelTime << " " << now - packetDate << endl;
//		}
//		knowledge.recordDouble(objectId, ovnisPacket.getPacketId(), vehicleId, packetDate, travelTime, 0);
//		if (distance > BROADCASTING_DISTANCE_THRESHOLD) {
////			double waitingTime = ovnisPacket.computeWaitingTime(position.x, position.y);
////			double waitingTime = FceApplication::rando.GetValue(0, RESEND_INTERVAL);
////			Simulator::Schedule(Seconds(waitingTime), &FceApplication::TryRebroadcast, this, ovnisPacket, packetDate, vehicleId);
//		}
//	}
//}
//
//void FceApplication::ReceiveTravelTimeEdgePacket(OvnisPacket ovnisPacket) {
//
//	double now =  Simulator::Now().GetSeconds();
//
//	Vector position = mobilityModel->GetPosition();
//	double distance = ovnisPacket.computeDistance(position.x, position.y);
//
//	//double date, string vehicleId, string objectId, double objectValue
//	double packetDate = ovnisPacket.readDouble();
//	string vehicleId = ovnisPacket.readString();
//	string objectId = ovnisPacket.readString();
//	double travelTime = ovnisPacket.readDouble();
//
//	knowledge.recordPacket(ovnisPacket.getPacketId());
//
//	Log::getInstance().packetReceived();
//	Log::getInstance().addDistance(distance);
//
//	// if heard for the first time
//	if (knowledge.getPacketCount(ovnisPacket.getPacketId()) == 1 && vehicle.getId() != vehicleId) {
//		knowledge.recordEdge(objectId, ovnisPacket.getPacketId(), vehicleId, packetDate, travelTime, 0);
//		// if average:
////		vehicle.recordEdge(objectId, ovnisPacket.getPacketId(), vehicleId, ovnisPacket.getSendingDate(), travelTime);
////		if (vehicle.getId() == "0.1") {
////			//<< ", packetDate: " << packetDate << ", packetAge: " << now - packetDate << endl;
////		}
//
//	}
//}

//void Dissemination::TryRebroadcast(OvnisPacket packet, double packetDate, string vehicleId) {
//	double now = Simulator::Now().GetSeconds();
//	// drop if was already forwarded or packet is old
////	if (vehicle.getPacketCount(packet.getPacketId()) > 1 || (now - packetDate) > PACKET_TTL) {
////		return;
////	}
////	Log::getInstance().packetForwarded();
////	Vector position = mobilityModel->GetPosition();
////	if (Log::getInstance().forwardedPackets.count(packet.getPacketId())== 0) {
////		Log::getInstance().forwardedPackets[packet.getPacketId()] = 0;
////	}
//////	Log::getInstance().getStream("broadcasting") << Simulator::Now().GetSeconds() << "\t" << packet.getPacketId() << "\t" << vehicle.getId() << "\t forwarding from \t" << packet.getSenderId() << "\t vehicleId: " << packet.getVehicleId() << "\t decisionTaken: " << decisionTaken << "\t" << packet.getStringValue() <<","<< packet.getDoubleValue() << endl;
////	++Log::getInstance().forwardedPackets[packet.getPacketId()];
////	Ptr<Packet> p = OvnisPacket::BuildPacket(now, vehicle.getId(), position.x, position.y, packet.getPacketType() , packet.getPacketId(), packetDate, vehicleId, packet.getStringValue(), packet.getDoubleValue());
////	SendPacket(p);
//}
}
