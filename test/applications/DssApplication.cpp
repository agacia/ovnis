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
 * @file DssApplication.cpp
 * 
 * @author Yoann Pigné <yoann@pigne.org>
 * @author Agata Grzybek
 *
 */

#include "applications/DssApplication.h"
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
#include "ovnis-constants.h"
#include "ovnisPacket.h"
#include "traci/structs.h"
#include "vehicle.h"
#include "common/commonHelper.h";
#include <ctime>
#include <time.h>

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DssApplication");
NS_OBJECT_ENSURE_REGISTERED(DssApplication);

TypeId DssApplication::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::DssApplication").SetParent<OvnisApplication>().AddConstructor<DssApplication>();
	return tid;
}

DssApplication::DssApplication() {
	m_port = 2000;
	m_socket = 0;
	InitializeScenario();
}

void DssApplication::InitializeScenario() {
	// scenario settings
	isVanet = IS_VANET;
	penetrationRate = PENETRATION_RATE;
	cheatersRate = CHEATER_RATE;
	networkId = "Highway";
//	networkId = "Kirchberg";

	Log::getInstance().getStream("scenarioSettings") << "vehicleId\t" << vehicle.getId() << endl;
	Log::getInstance().getStream("scenarioSettings") << "isVanet\t" << isVanet << endl;
	Log::getInstance().getStream("scenarioSettings") << "penetrationRate\t" << penetrationRate << endl;
	Log::getInstance().getStream("scenarioSettings") << "cheatersRate\t" << cheatersRate << endl;;
	Log::getInstance().getStream("scenarioSettings") << "networkId\t" << networkId << endl;
}

DssApplication::~DssApplication() {
	running = false;
//	Simulator::Cancel(m_trafficInformationEvent);
}

void DssApplication::StartApplication(void) {
	NS_LOG_FUNCTION ("");
	// initialize vehicle
	string vehicleId = Names::FindName(GetNode());
	vehicle.initialize(vehicleId, Simulator::Now().GetSeconds());

	// ns3
	mobilityModel = GetNode()->GetObject<ConstantVelocityMobilityModel>();
	ToggleNeighborDiscovery(true);
	Ptr<SocketFactory> socketFactory = GetNode()->GetObject<UdpSocketFactory> ();
	m_socket = socketFactory->CreateSocket();
	m_socket->Connect(InetSocketAddress (Ipv4Address::GetBroadcast(), m_port));
	m_socket->SetAllowBroadcast(true);
	m_socket->SetRecvCallback(MakeCallback(&DssApplication::ReceivePacket, this));
	m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));

	// start simualtion
	running = true;
	double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX * SIMULATION_STEP_INTERVAL;
//	double r2 = (double)(rand()%RAND_MAX)/(double)RAND_MAX * TRAFFIC_INFORMATION_SENDING_INTERVAL;
	m_simulationEvent = Simulator::Schedule(Seconds(r), &DssApplication::SimulationRun, this);
//	m_trafficInformationEvent = Simulator::Schedule(Seconds(1+r2), &DssApplication::SendTrafficInformation, this);
}

void DssApplication::ToggleNeighborDiscovery(bool on) {
	if (on) {
		std::ostringstream oss;
		oss << "/NodeList/" << GetNode()->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::BeaconingAdhocWifiMac/NeighborLost";
		Config::Connect(oss.str(), MakeCallback(&DssApplication::NeighborLost, this));
		std::ostringstream oss2;
		oss2 << "/NodeList/" << GetNode()->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::BeaconingAdhocWifiMac/NewNeighbor";
		Config::Connect(oss2.str(), MakeCallback(&DssApplication::NewNeighborFound, this));
	}
}

void DssApplication::StopApplication(void) {
	running = false;
}

void DssApplication::DoDispose(void) {
	running = false;
	double now = Simulator::Now().GetSeconds();
	if (m_socket != NULL) {
		m_socket->Close();
	} else {
		NS_LOG_WARN("DssApplication found null socket to close in StopApplication");
	}
	OvnisApplication::DoDispose();
}

void DssApplication::SimulationRun(void) {
	try {
		if (running == true) {
//			bool edgeChanged = vehicle.requestCurrentEdge(Simulator::Now().GetSeconds());
//			string currentEdge = vehicle.getItinerary().getCurrentEdge().getId();
//			double now = Simulator::Now().GetSeconds();
//			// if a vehicle is entering a new edge it has already traced the time on its itinerary
//			// broadcast information about it and about travel time on the last edge
//			if (edgeChanged) {
////				Edge lastEdge = vehicle.getItinerary().getLastEdge();
////				double travelTimeOnLastEdge = lastEdge.getTravelTime();
////				string lastEdgeId = lastEdge.getId();
////				Vector position = mobilityModel->GetPosition();
////				Log::getInstance().getStream(lastEdgeId) << "vehicle: " << vehicle.getId() << "\t now:" << now << "\t travelTimeOnLastEdge: " << travelTimeOnLastEdge << "\t vehs on route: " <<  TIS::getInstance().getVehiclesOnRoute("main") << endl;
//			}

			// if approaching an intersection
			// take decision which route to take from current to destination edge based on the collected knowledge about travel times on edges
			// reroute and mark that rerouted

			// if approaching the point that we want to evaluate
			// report to TIS the total travel time on the route between the decision edge and the current edge

			m_simulationEvent = Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &DssApplication::SimulationRun, this);
		}
	}
	catch (TraciException & ex) {
		running = false;
	}
}

void DssApplication::SendPacket(Ptr<Packet> packet) {
	if (packet == NULL || !running) {
		return;
	}
	try {
		if (m_socket->Send(packet) == -1) {
			cerr << "CrashTestApplication : error while sending packet : " << m_socket->GetErrno() << "" << Socket::ERROR_OPNOTSUPP << endl;
			exit(-1);
		}
		else {
			Log::getInstance().nextPacketId();
		}
	}
	catch (exception & e) {
		cerr << e.what();
	}
}

void DssApplication::ReceivePacket(Ptr<Socket> socket) {

	Address neighborMacAddress;
    Ptr<Packet> packet = socket->RecvFrom(neighborMacAddress);
    try {
    	// packet processing
    }
    catch (exception & e) {
		cerr << "receive packet not recognized by the application" << endl;
	}
}

/**
 * Neighborhood discovery
 */
void DssApplication::NeighborLost(std::string context, Ptr<const Packet> packet, Mac48Address addr){
	// Find the nodeId that has been called
	size_t aux = context.find("/",10);
	std::string strIndex = context.substr(10,aux-10);
	// Convert the index to integer
	int index = atoi (strIndex.c_str());
	// Get the node (my node)
	Ptr<Node> node = ns3::NodeList::GetNode(index);
	//Get the application
	Ptr <Application> app = node->GetApplication(0);
	Ptr<DssApplication> myApp = DynamicCast<DssApplication>(app);

	MacAddrMapIterator i = myApp->m_neighborList.find (addr);
	if (i == myApp->m_neighborList.end ()){
		// update the beacon index
		NS_LOG_DEBUG("ERROR. Trying to delete an unexisting neighbor");
	}
	else {
		myApp->m_neighborList.erase(i);
	}

	if (vehicle.getId() == "0.5") {
		std::cout << vehicle.getId() << "," << " lost a neighbor: " << addr << ", power: " << " number of neighbors: " << m_neighborList.size() << ", context: " << context << std::endl;
	}
}

/**
 * Neighborhood discovery
 */
void DssApplication::NewNeighborFound(std::string context, Ptr<const Packet> packet, Mac48Address addr, double rxPwDbm) {
	// Find the nodeId that has been called
	size_t aux =context.find("/",10);
	std::string strIndex = context.substr(10,aux-10);
	// Convert the index to integer
	int index = atoi (strIndex.c_str());
	// Get the node
	Ptr<Node> node = ns3::NodeList::GetNode(index);
	//Get the application
	Ptr <Application> app = node->GetApplication(0);
	Ptr<DssApplication> myApp = DynamicCast<DssApplication>(app);
	MacAddrMapIterator i = myApp->m_neighborList.find (addr);
	if (i== myApp->m_neighborList.end ()) {
		// include the reception power
		myApp->m_neighborList[addr] = rxPwDbm;
	}
	else {
		i->second= rxPwDbm;
	}
	if (vehicle.getId() == "0.5") {
//		packet->Print(cout) ;
		std::cout << vehicle.getId() << "," <<  " discovered a neighbor: " << addr << ", power: " << " number of neighbors: " << m_neighborList.size() << ", packet.size: " << packet->GetSize() << ", packet.serializedSize: " << packet->GetSerializedSize()<< std::endl;
	}
}

Vehicle* DssApplication::getData() {
	return new Vehicle();
}
}
