/**
 *
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
 * @file FceApplication.cpp
 * 
 * @author Yoann Pigné <yoann@pigne.org>
 *
 */

#include "applications/FceApplication.h"
#include <vector>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iterator>

#include "ns3/application.h"
#include "ns3/boolean.h"
#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/global-value.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/node-list.h"
#include "ns3/mac48-address.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tag-buffer.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/wifi-net-device.h"
#include "ns3/config.h"
#include "ns3/integer.h"
#include "ns3/assert.h"
#include "ns3/callback.h"

#include "applications/ovnis-application.h"
#include "ovnisPacket.h"
#include "traci/structs.h"
#include "vehicle.h"
#include "common/commonHelper.h";

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("FceApplication");
NS_OBJECT_ENSURE_REGISTERED(FceApplication);

TypeId FceApplication::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::FceApplication").SetParent<OvnisApplication>().AddConstructor<FceApplication>();
	return tid;
}

FceApplication::FceApplication() {
	m_port = 2000;
	m_socket = 0;
	decisionTaken = false;
	notificationSent = false;

//	 scenario
	scenario.setDecisionEdges(CommonHelper::split("main_1b"));
	scenario.setNotificationEdges(CommonHelper::split("main_2d bypass_2b"));
	map<string, Route> alternativeRoutes = map<string, Route>();
	alternativeRoutes["main"] = Route("main", "main_1 main_1b main_2 main_2a main_2b main_2c main_2d");
	alternativeRoutes["main"].setCapacity(1300);
	alternativeRoutes["bypass"] = Route("bypass", "main_1 main_1b bypass_1 bypass_2 bypass_2b");
	alternativeRoutes["bypass"].setCapacity(600);
	scenario.setAlternativeRoutes(alternativeRoutes);

//	scenario.setDecisionEdges(split("56640729#4 56640729#5"));
//	scenario.setNotificationEdges(split("53349130#1"));
//	map<string, Route> alternativeRoutes = map<string, Route>();
//	alternativeRoutes["kennedy"] = Route("kennedy", "56640729#0 56640729#1 56640729#2 56640729#3 56640729#4 56640729#5 56640728#0 56640728#1 56640728#2 56640728#3 56640728#4 56640728#5 56640728#6 56640728#7 56640728#8 55444662 23595095#0 23595095#1 53349130#0 53349130#1");
//	alternativeRoutes["kennedy"].setCapacity(1300);
//	alternativeRoutes["adenauer"] = Route("adenauer", "56640729#0 56640729#1 56640729#2 56640729#3 56640729#4 56640729#5 56640724#0 56640724#1 56640724#2 56640724#3 56640724#4 48977754#0 48977754#1 48977754#2 48977754#3 48977754#4 48977754#5 95511865#0 95511865#1 126603964 -149693909#2 -149693909#1 -149693909#0 -149693907 49248917#0 49248917#1 149693908 126603969 53349130#0 53349130#1");
//	alternativeRoutes["adenauer"].setCapacity(600);
//	alternativeRoutes["thuengen"] = Route("thuengen", "56640729#0 56640729#1 56640729#2 56640729#3 56640729#4 56640729#5 95511899 95511885#0 95511885#1 95511885#2 95511885#3 95511885#4 95511885#5 -50649897 -37847306#1 56640728#8 55444662 23595095#0 23595095#1 53349130#0 53349130#1");
//	alternativeRoutes["thuengen"].setCapacity(800);
//	scenario.setAlternativeRoutes(alternativeRoutes);
}

FceApplication::~FceApplication() {
	arrived = true;
	Simulator::Cancel(m_trafficInformationEvent);
}

void FceApplication::StartApplication(void) {

	NS_LOG_FUNCTION ("");

	// initialize vehicle
	string vehicleId = Names::FindName(GetNode());
	vehicle.initialize(vehicleId);
	vehicle.setScenario(scenario);
	tis.setScenario(scenario);

	// ns3
	// Connect the callback for the neighbor discovery service
	std::ostringstream oss;
	oss << "/NodeList/" << GetNode()->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::BeaconingAdhocWifiMac/NeighborLost";
	Config::Connect(oss.str(), MakeCallback(&FceApplication::NeighborLost, this));
	std::ostringstream oss2;
	oss2 << "/NodeList/" << GetNode()->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::BeaconingAdhocWifiMac/NewNeighbor";
	Config::Connect(oss2.str(), MakeCallback(&FceApplication::NewNeighborFound, this));

	mobilityModel = GetNode()->GetObject<ConstantVelocityMobilityModel>();

	// set socket
	Ptr<SocketFactory> socketFactory = GetNode()->GetObject<UdpSocketFactory> ();
	m_socket = socketFactory->CreateSocket();
	m_socket->Connect (InetSocketAddress (Ipv4Address::GetBroadcast(), m_port));
	m_socket->SetAllowBroadcast(true);
	m_socket->SetRecvCallback(MakeCallback(&FceApplication::ReceivePacket, this));
	m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));

	// start simualtion
	m_simulationEvent = Simulator::Schedule(Seconds(rando.GetValue(0, SIMULATION_STEP_INTERVAL)), &FceApplication::SimulationRun, this);
	m_trafficInformationEvent = Simulator::Schedule(Seconds(1+rando.GetValue(0, TRAFFIC_INFORMATION_SENDING_INTERVAL)), &FceApplication::SendTrafficInformation, this);
//	m_trafficInformationEvent = Simulator::Schedule(Seconds(1+rando.GetValue(0, TRAFFIC_INFORMATION_SENDING_INTERVAL)), &FceApplication::PseudoBeacon, this);

}

void FceApplication::StopApplication(void) {
	arrived = true;
//	vehicle.setArrivalTime(Simulator::Now().GetSeconds());
}

void FceApplication::DoDispose(void) {
	arrived = true;
	double now = Simulator::Now().GetSeconds();
	if (m_socket != NULL) {
		m_socket->Close();
	} else {
		NS_LOG_WARN("FceApplication found null socket to close in StopApplication");
	}
	OvnisApplication::DoDispose();
}

void FceApplication::SimulationRun(void) {
	try {
		if (arrived == false) {
			vehicle.requestCurrentEdge(Simulator::Now().GetSeconds());
			string currentEdge = vehicle.getItinerary().getCurrentEdge().getId();

			bool isDecisionPoint = find(vehicle.getScenario().getDecisionEdges().begin(), vehicle.getScenario().getDecisionEdges().end(), currentEdge) != vehicle.getScenario().getDecisionEdges().end();
			if (isDecisionPoint && !decisionTaken) {
				string routeChoice = tis.TakeDecision(knowledge, currentEdge, vehicle.getDestinationEdgeId());
				vehicle.reroute(routeChoice);
				decisionEdgeId = currentEdge;
				Log::getInstance().vehicleEnter(Simulator::Now().GetSeconds(), routeChoice);
				decisionTaken = true;
//				cout << Simulator::Now().GetSeconds() << " " << vehicle.getId() << " takes decision on " << currentEdge << " direction to " << vehicle.getDestinationEdgeId() << ": " << routeChoice << endl;
			}

			bool isReportingPoint = find(vehicle.getScenario().getNotificationEdges().begin(), vehicle.getScenario().getNotificationEdges().end(), currentEdge) != vehicle.getScenario().getNotificationEdges().end();
			if (isReportingPoint && !notificationSent) {
				string routeId = vehicle.getItinerary().getId();
				double travelTime = vehicle.getItinerary().computeTravelTime(decisionEdgeId, currentEdge);
				Log::getInstance().vehicleLeaveRoute(Simulator::Now().GetSeconds(), routeId, travelTime);
				tis.ReportRoute(routeId, decisionEdgeId, currentEdge, travelTime, Log::getInstance().getVehiclesOnRoute()[routeId]);
				notificationSent = true;
			}

			m_simulationEvent = Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &FceApplication::SimulationRun, this);
		}
	}
	catch (TraciException & ex) {
		arrived = true;
	}
}

void FceApplication::SendTrafficInformation(void) {
	if (arrived == false) {
		Vector position = mobilityModel->GetPosition();
		Data records[1];
		Data record;
		record.edgeId = "edgeId";
		record.travelTime = 1;
		record.numberOfVehicles = 2;
		record.date = Simulator::Now().GetSeconds();
		records[0] = record;
		Ptr<Packet> p = OvnisPacket::BuildTrafficInfoPacket(Simulator::Now().GetSeconds(), vehicle.getId(), position.x, position.y, TRAFFICINFO_PACKET_ID, Log::getInstance().getPacketId(), 1, records);
//		Ptr<Packet> p = OvnisPacket::BuildTrafficInfoPacket(Simulator::Now().GetSeconds(), vehicle.getId(), position.x, position.y, TRAFFICINFO_PACKET_ID, Log::getInstance().getPacketId(), 1);
		SendPacket(p);
		m_trafficInformationEvent = Simulator::Schedule(Seconds(TRAFFIC_INFORMATION_SENDING_INTERVAL), &FceApplication::SendTrafficInformation, this);
	}
//	if (arrived == false) {
//			Ptr<Packet> p = OvnisPacket::BuildPacket(1, vehicle.getId(), 1, 1, TRAVELTIME_EDGE_PACKET_ID , 1, 1, vehicle.getId(), vehicle.getItinerary().getCurrentEdge().getId(), 1);
//			SendPacket(p);
//			m_trafficInformationEvent = Simulator::Schedule(Seconds(TRAFFIC_INFORMATION_SENDING_INTERVAL), &FceApplication::SendTrafficInformation, this);
//		}
//	int size = vehicle.getRecords().size();
//	if (size > 0 && arrived == false) {
//		double now = Simulator::Now().GetSeconds();
//		Vector position = mobilityModel->GetPosition();
//		double travelTime = entry.getLatestValue();
//								double packetDate = entry.getLatestTime();
//					//			long packetId = entry.getLatestPacketId();
//								string senderId = entry.getLatestSenderId();
//					//			double travelTime = entry.getAverageValue();
//					//			double packetDate = entry.getAverageTime();
//					//			string senderId = vehicle.getId();
//								long packetId = Log::getInstance().getPacketId();
//								Log::getInstance().nextPacketId();
//								if (packetDate == 0 || (vehicle.getItinerary().containsEdge(it->first)==true && vehicle.getItinerary().getEdge(it->first).getLeftTime() > entry.getLatestTime())) {
//									travelTime = vehicle.getItinerary().getEdge(it->first).getTravelTime();
//									packetDate = vehicle.getItinerary().getEdge(it->first).getLeftTime();
//									senderId = vehicle.getId();
//					//				packetId = Log::getInstance().getPacketId();
//					//				Log::getInstance().nextPacketId();
//								}
//								if (packetDate != 0) {
//									if (now - packetDate < PACKET_TTL) {
//									}
//									}
//				}
//		Ptr<Packet> p = OvnisPacket::BuildPacket(now, vehicle.getId(), position.x, position.y, TRAVELTIME_EDGE_PACKET_ID , packetId, packetDate, senderId, it->first, travelTime);
//		SendPacket(p);
//		m_trafficInformationEvent = Simulator::Schedule(Seconds(TRAFFIC_INFORMATION_SENDING_INTERVAL), &FceApplication::TrafficInformationAction, this);
//	}
}

/**
 *	Beaconing current edge and speed, travel time so far (recorded), estimated travel time on the rest of the route.
 */
//void FceApplication::Beacon(void) {
//	int size = vehicle.getRecords().size();
//	if (size > 0 && arrived == false) {
//		double now = Simulator::Now().GetSeconds();
//		Vector position = mobilityModel->GetPosition();
//		string itineraryId = vehicle.getItinerary().getId();
//		string startEdge = vehicle.getItinerary().getDepartedEdge();
//		string lastEdge = vehicle.getItinerary().getArrivalEdge();
//		double travelTime = vehicle.getItinerary().getTravelTime();
//		string currentEdgeId = vehicle.getItinerary().getCurrentEdge().getId();
//		double currentSpeed = vehicle.getCurrentSpeed();
////		Log::getInstance().getStream("beaconing")
////		cout << vehicle.getId() << " beaconing about " <<  itineraryId << " (" << startEdge << ", " << lastEdge << ") " << " currentEdge: " << currentEdgeId << ", currentSpeed: " << currentSpeed << ", travelTime: " << travelTime << endl;
//		Ptr<Packet> p = OvnisPacket::BuildTravelTimePacket(now, vehicle.getId(), position.x, position.y, TRAVELTIME_ROUTE_PACKET_ID , Log::getInstance().getPacketId(), itineraryId, currentEdgeId, currentSpeed, travelTime, 0, 0);
//		SendPacket(p);
//		m_beaconEvent = Simulator::Schedule(Seconds(BEACONING_INTERVAL), &FceApplication::TrafficInformationAction, this);
//	}
//}


void FceApplication::SendPacket(Ptr<Packet> packet) {
	if (packet == NULL || arrived) {
		return;
	}
	try {
		if (m_socket->Send(packet) == -1) {
//		if (m_socket->SendTo(packet, 0, realTo) == -1) {
			cerr << "CrashTestApplication : error while sending packet : " << m_socket->GetErrno() << "" << Socket::ERROR_OPNOTSUPP << endl;
			exit(-1);
		}
		else {
//			Log::getInstance().packetSent();
		}
	}
	catch (exception & e) {
		cerr << e.what();
	}
}

void FceApplication::ReceivePacket(Ptr<Socket> socket) {

	Address neighborMacAddress;
    Ptr<Packet> packet = socket->RecvFrom(neighborMacAddress);
    //    cout << "receive something!" << endl;
    try {
    	OvnisPacket ovnisPacket(packet);

		if (ovnisPacket.getPacketType() == TRAVELTIME_PACKET_ID) {
			ReceiveTravelTimePacket(ovnisPacket);
		}
		if (ovnisPacket.getPacketType() == TRAVELTIME_EDGE_PACKET_ID) {
			ReceiveTravelTimeEdgePacket(ovnisPacket);
		}
		if (ovnisPacket.getPacketType() == TRAVELTIME_ROUTE_PACKET_ID) {
			ReceiveTravelTimeRoutePacket(ovnisPacket);
		}
    }
    catch (exception & e) {
		cerr << "receive packet not recognized by ovnis" << endl;
	}
}


void FceApplication::ReceiveTravelTimeRoutePacket(OvnisPacket ovnisPacket) {
	Vector position = mobilityModel->GetPosition();
	double distance = ovnisPacket.computeDistance(position.x, position.y);

//	string routeId, string currentEdgeId, double currentSpeed, double travelTime, double estimatedTravelTime, double estimationDate
	string routeId = ovnisPacket.readString();
	string currentEdgeId = ovnisPacket.readString();
	double currentSpeed = ovnisPacket.readDouble();
	double travelTime = ovnisPacket.readDouble();
	double estimatedTravelTime = ovnisPacket.readDouble();
	double estimationDate = ovnisPacket.readDouble();
	double packetAge = Simulator::Now().GetSeconds() - ovnisPacket.getSendingDate();
//	cout << vehicle.getId() << " heard from " << ovnisPacket.getSenderId() << " about: " << routeId << " currentEdge: " << currentEdgeId << " travelTime: " << travelTime << " (distance: " << distance << ", packetAge: " << packetAge << ")" << endl;

	Log::getInstance().packetReceived();
	Log::getInstance().addDistance(distance);
	double now =  Simulator::Now().GetSeconds();

}


void FceApplication::ReceiveTravelTimePacket(OvnisPacket ovnisPacket) {
	Vector position = mobilityModel->GetPosition();
	double distance = ovnisPacket.computeDistance(position.x, position.y);

	//double date, string vehicleId, string objectId, double objectValue
	double packetDate = ovnisPacket.readDouble();
	string vehicleId = ovnisPacket.readString();
	string objectId = ovnisPacket.readString();
	double travelTime = ovnisPacket.readDouble();

	knowledge.recordPacket(ovnisPacket.getPacketId());
	Log::getInstance().packetReceived();
	Log::getInstance().addDistance(distance);

	double now =  Simulator::Now().GetSeconds();

	// if heard for the first time
	if (knowledge.getPacketCount(ovnisPacket.getPacketId()) == 1 && vehicle.getId() != vehicleId) {
		if (!decisionTaken && (now - packetDate) < PACKET_TTL ) {
			Log::getInstance().getStream("hearing") << ovnisPacket.getPacketType() << ", hearing about " << objectId << " " << ovnisPacket.getSenderId() <<" from " << vehicleId << " " << packetDate << " " << travelTime << " " << now - packetDate << endl;
		}
		knowledge.recordDouble(objectId, ovnisPacket.getPacketId(), vehicleId, packetDate, travelTime);
		if (distance > BROADCASTING_DISTANCE_THRESHOLD) {
//			double waitingTime = ovnisPacket.computeWaitingTime(position.x, position.y);
//			double waitingTime = FceApplication::rando.GetValue(0, RESEND_INTERVAL);
//			Simulator::Schedule(Seconds(waitingTime), &FceApplication::TryRebroadcast, this, ovnisPacket, packetDate, vehicleId);
		}
	}
}

void FceApplication::ReceiveTravelTimeEdgePacket(OvnisPacket ovnisPacket) {

	double now =  Simulator::Now().GetSeconds();

	Vector position = mobilityModel->GetPosition();
	double distance = ovnisPacket.computeDistance(position.x, position.y);

	//double date, string vehicleId, string objectId, double objectValue
	double packetDate = ovnisPacket.readDouble();
	string vehicleId = ovnisPacket.readString();
	string objectId = ovnisPacket.readString();
	double travelTime = ovnisPacket.readDouble();

	knowledge.recordPacket(ovnisPacket.getPacketId());

	Log::getInstance().packetReceived();
	Log::getInstance().addDistance(distance);

	// if heard for the first time
	if (knowledge.getPacketCount(ovnisPacket.getPacketId()) == 1 && vehicle.getId() != vehicleId) {
		knowledge.recordEdge(objectId, ovnisPacket.getPacketId(), vehicleId, packetDate, travelTime);
		// if average:
//		vehicle.recordEdge(objectId, ovnisPacket.getPacketId(), vehicleId, ovnisPacket.getSendingDate(), travelTime);
//		if (vehicle.getId() == "0.1") {
//			cout << "[" << now << "] \t" << vehicle.getId() << " received from " << ovnisPacket.getSenderId() << " age: " << now - ovnisPacket.getSendingDate() <<" distance: " << distance << "\t vehicleId: " << vehicleId << ", edgeId: " << objectId << ", travelTime: " << travelTime << endl;
//			//<< ", packetDate: " << packetDate << ", packetAge: " << now - packetDate << endl;
//		}

	}
}

/**
 * Neighborhood discovery
 */
void FceApplication::NeighborLost(std::string context, Ptr<const Packet> packet, Mac48Address addr){
	// Find the nodeId that has been called
	size_t aux = context.find("/",10);
	std::string strIndex = context.substr(10,aux-10);
	// Convert the index to integer
	int index = atoi (strIndex.c_str());
	// Get the node (my node)
	Ptr<Node> node = ns3::NodeList::GetNode(index);
	//Get the application
	Ptr <Application> app = node->GetApplication(0);
	Ptr<FceApplication> myApp = DynamicCast<FceApplication>(app);

	MacAddrMapIterator i = myApp->m_neighborList.find (addr);
	if (i == myApp->m_neighborList.end ()){
		// update the beacon index
		NS_LOG_DEBUG("ERROR. Trying to delete an unexisting neighbor");
	}
	else {
		myApp->m_neighborList.erase(i);
	}

//	MacAddrMapIterator i = m_neighborList.find (addr);
//	if (i == m_neighborList.end ()){
//		// update the beacon index
//		NS_LOG_DEBUG("ERROR. Trying to delete an unexisting neighbor");
//	}
//	else {
//		m_neighborList.erase(i);
//	}

//	if (vehicle.getId() == "0.5") {
//		std::cout << vehicle.getId() << "," << " lost a neighbor: " << addr << ", power: " << " number of neighbors: " << m_neighborList.size() << ", context: " << context << std::endl;
//	}
}

/**
 * Neighborhood discovery
 */
void FceApplication::NewNeighborFound (std::string context, Ptr<const Packet> packet, Mac48Address addr, double rxPwDbm){
	// Find the nodeId that has been called
	size_t aux =context.find("/",10);
	std::string strIndex = context.substr(10,aux-10);
	// Convert the index to integer
	int index = atoi (strIndex.c_str());
	// Get the node
	Ptr<Node> node = ns3::NodeList::GetNode(index);
	//Get the application
	Ptr <Application> app = node->GetApplication(0);
	Ptr<FceApplication> myApp = DynamicCast<FceApplication>(app);

	MacAddrMapIterator i = myApp->m_neighborList.find (addr);
	if ( i== myApp->m_neighborList.end ()){
		// include the reception power
		myApp->m_neighborList[addr] = rxPwDbm;
	}
	else{
		i->second= rxPwDbm;
	}
//	MacAddrMapIterator i = m_neighborList.find (addr);
//	if (i== m_neighborList.end ()){
//		// include the reception power
//		m_neighborList[addr] = rxPwDbm;
//	}
//	else{
//		i->second= rxPwDbm;
//	}
//	if (vehicle.getId() == "0.5") {
//		std::cout << vehicle.getId() << " has a new neighbor: " << addr << ", power: " << rxPwDbm << " number of neighbors: " << m_neighborList.size() << ", context: " << context << std::endl;
//	}
}

}
