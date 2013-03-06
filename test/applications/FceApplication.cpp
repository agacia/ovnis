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
#include <ctime>
#include <time.h>

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
	isCheater = false;
	flow = 1250;

//	scenario.setDecisionEdges(CommonHelper::split("main_1b"));
//	scenario.setNotificationEdges(CommonHelper::split("main_2d bypass_2b"));
//	map<string, Route> alternativeRoutes = map<string, Route>();
//	alternativeRoutes["main"] = Route("main", "main_1 main_1b main_2 main_2a main_2b main_2c main_2d");
//	alternativeRoutes["main"].setCapacity(900);
//	alternativeRoutes["bypass"] = Route("bypass", "main_1 main_1b bypass_1 bypass_2 bypass_2b");
//	alternativeRoutes["bypass"].setCapacity(1500);
//	scenario.setAlternativeRoutes(alternativeRoutes);

	scenario.setDecisionEdges(CommonHelper::split("pre_2"));
	scenario.setNotificationEdges(CommonHelper::split("main_6 bypass_3"));
	map<string, Route> alternativeRoutes = map<string, Route>();
//	alternativeRoutes["main"] = Route("main", "pre_1 pre_2 main_1 main_2 main_3 main_4 main_5 main_6");
	alternativeRoutes["main"] = Route("main", "pre_1 pre_2 main_1 main_2a main_2b main_3a main_3b main_4a main_4b main_5a main_5b main_6");
	alternativeRoutes["main"].setCapacity(900);
	alternativeRoutes["bypass"] = Route("bypass", "pre_1 pre_2 bypass_1 bypass_2 bypass_3");
	alternativeRoutes["bypass"].setCapacity(1500);
	scenario.setAlternativeRoutes(alternativeRoutes);

//	scenario.setDecisionEdges(CommonHelper::split("56640729#5"));
//	scenario.setNotificationEdges(CommonHelper::split("53349130#1"));
//	map<string, Route> alternativeRoutes = map<string, Route>();
//	alternativeRoutes["kennedy"] = Route("kennedy", "56640729#0 56640729#1 56640729#2 56640729#3 56640729#4 56640729#5 56640728#0 56640728#1 56640728#2 56640728#3 56640728#4 56640728#5 56640728#6 56640728#7 56640728#8 55444662 23595095#0 23595095#1 53349130#0 53349130#1");
//	alternativeRoutes["kennedy"].setCapacity(1300);
//	alternativeRoutes["adenauer"] = Route("adenauer", "56640729#0 56640729#1 56640729#2 56640729#3 56640729#4 56640729#5 56640724#0 56640724#1 56640724#2 56640724#3 56640724#4 48977754#0 48977754#1 48977754#2 48977754#3 48977754#4 48977754#5 95511865#0 95511865#1 126603964 -149693909#2 -149693909#1 -149693909#0 -149693907 49248917#0 49248917#1 149693908 126603969 53349130#0 53349130#1");
//	alternativeRoutes["adenauer"].setCapacity(600);
//	alternativeRoutes["thuengen"] = Route("thuengen", "56640729#0 56640729#1 56640729#2 56640729#3 56640729#5 95511899 95511885#0 95511885#1 95511885#2 95511885#3 95511885#4 95511885#5 -50649897 -37847306#1 56640728#8 55444662 23595095#0 23595095#1 53349130#0 53349130#1");
//	alternativeRoutes["thuengen"].setCapacity(800);
//	scenario.setAlternativeRoutes(alternativeRoutes);
}

FceApplication::~FceApplication() {
	running = false;
	Simulator::Cancel(m_trafficInformationEvent);
}

void FceApplication::StartApplication(void) {

	NS_LOG_FUNCTION ("");

	// initialize vehicle
	string vehicleId = Names::FindName(GetNode());
	vehicle.initialize(vehicleId);
	vehicle.setScenario(scenario);
	TIS::getInstance().initializeStaticTravelTimes(scenario.getAlternativeRoutes());

	// ns3
	mobilityModel = GetNode()->GetObject<ConstantVelocityMobilityModel>();
	// Connect the callback for the neighbor discovery service
	std::ostringstream oss;
	oss << "/NodeList/" << GetNode()->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::BeaconingAdhocWifiMac/NeighborLost";
	Config::Connect(oss.str(), MakeCallback(&FceApplication::NeighborLost, this));
	std::ostringstream oss2;
	oss2 << "/NodeList/" << GetNode()->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::BeaconingAdhocWifiMac/NewNeighbor";
	Config::Connect(oss2.str(), MakeCallback(&FceApplication::NewNeighborFound, this));
	// set socket
	Ptr<SocketFactory> socketFactory = GetNode()->GetObject<UdpSocketFactory> ();
	m_socket = socketFactory->CreateSocket();
	m_socket->Connect (InetSocketAddress (Ipv4Address::GetBroadcast(), m_port));
	m_socket->SetAllowBroadcast(true);
	m_socket->SetRecvCallback(MakeCallback(&FceApplication::ReceivePacket, this));
	m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));

	// start simualtion
	running = true;

	double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX * SIMULATION_STEP_INTERVAL;
//	double r = rando.GetValue(0, SIMULATION_STEP_INTERVAL);
	double r2 = (double)(rand()%RAND_MAX)/(double)RAND_MAX * TRAFFIC_INFORMATION_SENDING_INTERVAL;
//	double r2 = rando.GetValue(0, TRAFFIC_INFORMATION_SENDING_INTERVAL);

	m_simulationEvent = Simulator::Schedule(Seconds(r), &FceApplication::SimulationRun, this);
	m_trafficInformationEvent = Simulator::Schedule(Seconds(1+r2), &FceApplication::SendTrafficInformation, this);

}

void FceApplication::StopApplication(void) {
	running = false;
}

void FceApplication::DoDispose(void) {
	running = false;
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
		if (running == true) {
			bool edgeChanged = vehicle.requestCurrentEdge(Simulator::Now().GetSeconds());
			string currentEdge = vehicle.getItinerary().getCurrentEdge().getId();
			double now = Simulator::Now().GetSeconds();

			// if a vehicle is entering a new edge it has already traced the time on its itinerary
			// broadcast information about it and about travel time on the last edge
			if (edgeChanged) {
				Edge lastEdge = vehicle.getItinerary().getLastEdge();
				double travelTimeOnLastEdge = lastEdge.getTravelTime();
				string lastEdgeId = lastEdge.getId();
				Vector position = mobilityModel->GetPosition();
				TIS::getInstance().reportEdgePosition(lastEdge.getId(), position.x, position.y);
//				Log::getInstance().getStream("") << now << "\t" <<lastEdgeId << "\t" << position.x << "," << position.y << "\n" ;
				Ptr<Packet> p = OvnisPacket::BuildChangedEdgePacket(now, vehicle.getId(), position.x, position.y, CHANGED_EDGE_PACKET_ID, lastEdgeId, travelTimeOnLastEdge, currentEdge);
				SendPacket(p);
			}

			// if approaching an intersection
			// take decision which route to takefrom current to destination edge based on the collected knowledge about travel times on edges
			bool isDecisionPoint = find(vehicle.getScenario().getDecisionEdges().begin(), vehicle.getScenario().getDecisionEdges().end(), currentEdge) != vehicle.getScenario().getDecisionEdges().end();
			if (isDecisionPoint && !decisionTaken) {

				map<string, double> globalCosts = TIS::getInstance().getCosts(vehicle.getScenario().getAlternativeRoutes(), currentEdge, vehicle.getDestinationEdgeId());
				string global_minTravelTimeChoice = TIS::getInstance().chooseMinTravelTimeRoute(globalCosts);
				string global_proportionalProbabilisticChoice = TIS::getInstance().chooseProbTravelTimeRoute(globalCosts);
				string global_flowAwareChoice =  TIS::getInstance().chooseFlowAwareRoute(flow, globalCosts);
				//
				Vector position = mobilityModel->GetPosition();
				Log::getInstance().getStream("vanets_knowledge") << now << "\t" << position.x << "\t" << position.y << "\t" << currentEdge << "\t" << vehicle.getDestinationEdgeId() << "\t";
				map<string, double> vanetCosts =  vanetsKnowledge.getCosts(vehicle.getScenario().getAlternativeRoutes(), currentEdge, vehicle.getDestinationEdgeId());
				string vanet_minTravelTimeChoice =  TIS::getInstance().chooseMinTravelTimeRoute(vanetCosts);
				string vanet_proportionalProbabilisticChoice =  TIS::getInstance().chooseProbTravelTimeRoute(vanetCosts);
				string vanet_flowAwareChoice =  TIS::getInstance().chooseFlowAwareRoute(flow, vanetCosts);

				// select strategy

				// centralized
//				string selfishRouteChoice = global_minTravelTimeChoice;
//				string systemRouteChoice = global_proportionalProbabilisticChoice;
//				double selfishTravelTime = globalCosts[selfishRouteChoice];
//				double systemTravelTIme = globalCosts[systemRouteChoice];

				// vanets
				string selfishRouteChoice = vanet_minTravelTimeChoice;
				string systemRouteChoice = vanet_proportionalProbabilisticChoice;
				double selfishTravelTime = vanetCosts[selfishRouteChoice];
				double systemTravelTIme = vanetCosts[systemRouteChoice];

				string routeChoice = selfishRouteChoice;
				selfishExpectedTravelTime = selfishTravelTime;
				expectedTravelTime = selfishTravelTime;

				// hybrid
				double capacityDrop = vanetsKnowledge.isCapacityDrop(vehicle.getScenario().getAlternativeRoutes(), currentEdge, vehicle.getDestinationEdgeId());
				TIS::getInstance().setCongestion(capacityDrop);
				if (capacityDrop) {
					routeChoice = systemRouteChoice;
					expectedTravelTime = systemTravelTIme;
				}

//				double now = Simulator::Now().GetSeconds();
//				Log::getInstance().getStream("global_routing_strategies") << now << "\t" << global_minTravelTimeChoice << "\t" << global_proportionalProbabilisticChoice << "\t" << global_flowAwareChoice << endl;
//				Log::getInstance().getStream("vanet_routing_strategies") << now << "\t" << vanet_minTravelTimeChoice << "\t" << vanet_proportionalProbabilisticChoice << "\t" << vanet_flowAwareChoice << endl;

				double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX;

//				string routeChoice = selfishRouteChoice;
				if (r < CHEATER_RATIO) {
					if (routeChoice != selfishRouteChoice) {
						routeChoice = selfishRouteChoice;
						isCheater = true;
					}
				}

				vehicle.reroute(routeChoice);
				decisionEdgeId = currentEdge;
				TIS::getInstance().reportStartingRoute(routeChoice, currentEdge, vehicle.getDestinationEdgeId());
				decisionTaken = true;
			}

			// if approaching the point that we want to evaluate
			// report to TIS the total travel time on the route between the decision edge and the current edge
			bool isReportingPoint = find(vehicle.getScenario().getNotificationEdges().begin(), vehicle.getScenario().getNotificationEdges().end(), currentEdge) != vehicle.getScenario().getNotificationEdges().end();
			if (isReportingPoint && !notificationSent) {
				string routeId = vehicle.getItinerary().getId();
				double travelTime = vehicle.getItinerary().computeTravelTime(decisionEdgeId, currentEdge);
				//cout << Simulator::Now().GetSeconds() << " " ;
				TIS::getInstance().reportEndingRoute(routeId, decisionEdgeId, currentEdge, travelTime, isCheater, selfishExpectedTravelTime, expectedTravelTime);
				notificationSent = true;
			}

			m_simulationEvent = Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &FceApplication::SimulationRun, this);
		}
	}
	catch (TraciException & ex) {
		running = false;
	}
}

void FceApplication::SendTrafficInformation(void) {
	if (running == true) {
		Vector position = mobilityModel->GetPosition();
		vector<Data> records = dissemination.getTrafficInformationToSend(vanetsKnowledge, vehicle.getEdgesAhead());
		if (records.size() > 0) {
			Ptr<Packet> p = OvnisPacket::BuildTrafficInfoPacket(Simulator::Now().GetSeconds(), vehicle.getId(), position.x, position.y, TRAFFICINFO_PACKET_ID, records.size(), records);
			SendPacket(p);
		}
		m_trafficInformationEvent = Simulator::Schedule(Seconds(TRAFFIC_INFORMATION_SENDING_INTERVAL), &FceApplication::SendTrafficInformation, this);
	}
}

void FceApplication::SendPacket(Ptr<Packet> packet) {
	if (packet == NULL || !running) {
		return;
	}
	try {
		if (m_socket->Send(packet) == -1) {
//		if (m_socket->SendTo(packet, 0, realTo) == -1) {
			cerr << "CrashTestApplication : error while sending packet : " << m_socket->GetErrno() << "" << Socket::ERROR_OPNOTSUPP << endl;
			exit(-1);
		}
		else {
			Log::getInstance().nextPacketId();
//			Log::getInstance().packetSent(); // moved to beaconing-adhoc-wifi-mac.cpp
		}
	}
	catch (exception & e) {
		cerr << e.what();
	}
}

void FceApplication::ReceivePacket(Ptr<Socket> socket) {

	Address neighborMacAddress;
    Ptr<Packet> packet = socket->RecvFrom(neighborMacAddress);

    try {
    	OvnisPacket ovnisPacket(packet);
    	Vector position = mobilityModel->GetPosition();

		double distance = ovnisPacket.computeDistance(position.x, position.y);

		double packetDate = ovnisPacket.getTimeStamp();
		double packetAge = Simulator::Now().GetSeconds() - packetDate;
		string senderId = ovnisPacket.getSenderId();
//		long packetId = ovnisPacket.getPacketId();

		if (ovnisPacket.getPacketType() == CHANGED_EDGE_PACKET_ID) {
			string lastEdge = ovnisPacket.readString();
			double travelTime = ovnisPacket.readDouble();
			string currentEdge = ovnisPacket.readString();
			double numberOfVehiclesOnTheLastEdge = vanetsKnowledge.substractNumberOfVehicles(lastEdge);
			double numberOfVehiclesOnTheCurrentEdge = vanetsKnowledge.addNumberOfVehicles(currentEdge);
			Data data;
			data.date = packetDate;
			data.edgeId = lastEdge;
			data.travelTime = travelTime;
			vanetsKnowledge.record(data);
		}
		if (ovnisPacket.getPacketType() == TRAFFICINFO_PACKET_ID) {
			vector<Data> data = ovnisPacket.ReadTrafficInfoPacket();
			vanetsKnowledge.record(data);
		}

		Log::getInstance().packetReceived();
		Log::getInstance().addDistance(distance);
    }
    catch (exception & e) {
		cerr << "receive packet not recognized by ovnis" << endl;
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
