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
 * @file TestApplication.cpp
 * 
 * @author Yoann Pigné <yoann@pigne.org>
 *
 */

#include "applications/TestApplication.h"
#include <vector>
#include <math.h>

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

#include "my-constants.h"
#include "applications/ovnis-application.h"
#include "vehicle.h"
#include "traci/structs.h"

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TestApplication");
NS_OBJECT_ENSURE_REGISTERED(TestApplication);

TypeId TestApplication::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::TestApplication").SetParent<OvnisApplication>().AddConstructor<TestApplication>();
	return tid;
}

TestApplication::TestApplication() {
	m_port = 2000;
	m_socket = 0;
	m_last_resend = 0;
	m_jam_state = false;
}

TestApplication::~TestApplication() {
}

void TestApplication::StartApplication(void) {
	NS_LOG_FUNCTION ("");

	double now = Simulator::Now().GetSeconds();

	// get the mobility model (for speed requests)
	Ptr<Object> node = GetNode();
	mobilityModel = node->GetObject<ConstantVelocityMobilityModel>();

	vehicle.initialize(Names::FindName(node), now);
	vehicle.requestCurrentEdge(now);
//
	Ptr<SocketFactory> socketFactory = GetNode()->GetObject<UdpSocketFactory>();
	m_socket = socketFactory->CreateSocket();
	m_socket->SetAllowBroadcast(true);
	m_socket->SetRecvCallback(MakeCallback(&TestApplication::ReceiveData, this));
	m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));
	// store the own address
	//Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();

//	m_actionEvent = Simulator::Schedule(Seconds(rando.GetValue(0, PROACTIVE_INTERVAL)), &TestApplication::Action, this);
	m_actionEvent = Simulator::Schedule(Seconds(rando.GetValue(0, BEACONING_INTERVAL)), &TestApplication::Action, this);

}

void TestApplication::StopApplication(void) {
//	vehicle.setArrivalTime(Simulator::Now().GetSeconds());
	//ovnis::Log::getInstance().summariseVehicle(*vehicle);
	//ovnis::Log::getInstance().printRoute(vehicle->getRoutes());
	double now = Simulator::Now().GetSeconds();
	if (vehicle.getId() ==trackedVehicle) {
		cout << vehicle.getId() << " stopping application (" << now << ")" << endl;
	}
}

void TestApplication::DoDispose(void) {
	//NS_LOG_FUNCTION_NOARGS ();
	double now = Simulator::Now().GetSeconds();
	if (vehicle.getId() ==trackedVehicle) {
		cout << vehicle.getId() << " disposing application (" << now << ")" << endl;
	}
	if (m_socket != NULL) {
		m_socket->Close();
	} else {
		NS_LOG_WARN("TestApplication found null socket to close in StopApplication");
	}
	OvnisApplication::DoDispose();
}

void TestApplication::SendMyState() {
//	Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
//	Ipv4Address address = ipv4->GetAddress(1, 0).GetLocal();
//	double now = Simulator::Now().GetSeconds();
//	if (vehicle.getId() =="_h0_69") {
//		cout << vehicle.getId() << "(" << address << ") broadcasting (" << now << ")" << endl;
//	}

	NS_LOG_FUNCTION_NOARGS();
	Ipv4Address add = Ipv4Address::GetBroadcast();
	Address realTo = InetSocketAddress(add, TestApplication::m_port);
	Ptr<Packet> p = CreateStatePacket();
	if (m_socket->SendTo(p, 0, realTo) == -1) {
		cerr << "CrashTestApplication : error while sending packet : " << m_socket->GetErrno() << "" << Socket::ERROR_OPNOTSUPP << endl;
		exit(-1);
	}
	else {
		Log::getInstance().packetSent();
	}
}

Ptr<Packet> TestApplication::CreateStatePacket() {

//	if (vehicle.getId() =="_h0_38" || vehicle.getId() =="_h0_69") {
//	if (vehicle.getId() =="_h0_69") {
//		cout << "vehicle " << vehicle.getId() << " Sending Date: " << date << endl;
//	}
	double date = Simulator::Now().GetSeconds();

	vehicle.requestCurrentPosition();
	ovnis::Position2D pos = vehicle.getCurrentPosition();

	NS_LOG_FUNCTION_NOARGS();
	string id = vehicle.getId();
	int stringSize = id.size();
	int size = sizeof(double) + sizeof(int) + stringSize * sizeof(char) + sizeof(double) + sizeof(double);
	uint8_t * t = (uint8_t*) malloc(size);

	TagBuffer tg(t, t + size);
	tg.WriteDouble(date);
	//tg.WriteU64(date);
	tg.WriteU32(stringSize);
	tg.Write((uint8_t*) id.c_str(), stringSize);
	tg.WriteDouble(pos.x);
	tg.WriteDouble(pos.y);

	Ptr<Packet> p = Create<Packet>(t, size);
	free(t);
	return p;
}

void TestApplication::Action() {
	NS_LOG_FUNCTION(now);

	if (m_stopTime != TimeStep(0) && Simulator::Now() >= m_stopTime) {
		return;
	}

//	string currentEdge = vehicle.getCurrentEdge();
//		if (currentEdge.empty()) {
//			currentEdge = vehicle.requestCurrentEdge();
//		}
//		else {
//			// ask for my current edge
//			string newEdge = vehicle.requestCurrentEdge();
//			// if I changed edge then I ask for new speed limits
//			if (newEdge.empty()) {
//				// if new_edge is empty, then there is a problem. The vehicle has been removed from
//				cerr
//						<< vehicle.getId()
//						<< " can't access it cunning edge. It has probably been removed in SUMO. Let stop it."
//						<< endl;
//				m_stopTime == Simulator::Now();
//				return;
//			}
//		}
//		m_actionEvent = Simulator::Schedule(Seconds(PROACTIVE_INTERVAL), &TestApplication::Action, this);


	// ask for my speed
		//	double speed = vehicle.requestCurrentSpeed();

	//decide if JAM or not according to a threshold
//	if (speed < (SPEED_THRESHOLD * vehicle.getMaxLaneSpeed())) {
//		NS_LOG_DEBUG(vehicle.getId() << " JAM?");
//		if (m_jam_state) {
//			// I am in a jam for more than PROACTIVE_INTERVAL
//			NS_LOG_DEBUG(vehicle.getId()<<" JAM.");
//			m_last_resend = Simulator::Now().GetSeconds();
//			m_sendEvent = Simulator::ScheduleNow(&TestApplication::SendMyState,this, Simulator::Now().GetSeconds(), vehicle.getCurrentEdge());
//		}
//		m_jam_state = true;
//	}
//	else {
//		// if ever I was in JAM state, then I am not anymore
//		if (m_jam_state) {
//			NS_LOG_DEBUG(vehicle.getId()<<" NO MORE JAM.");
//			m_jam_state = false;
//		}
//	}

//	m_sendEvent = Simulator::ScheduleNow(&TestApplication::SendMyState, this);
	//SendMyState(now, vehicle.getId());
	//cout << now << endl;
	m_actionEvent = Simulator::Schedule(Seconds(BEACONING_INTERVAL), &TestApplication::Action, this);

}

void TestApplication::ReceiveData(Ptr<Socket> socket) {

	Address neighborMacAddress;
	Ptr<Packet> packet = socket->RecvFrom(neighborMacAddress);
	InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom(neighborMacAddress);
	Ipv4Address neighborIPv4Addr = inetSourceAddr.GetIpv4();

//	vehicle.requestCurrentPosition();
//	ovnis::Position2D pos = vehicle.getCurrentPosition();
	Ptr<NetDevice> d = GetNode()->GetDevice(0);
	Ptr<WifiNetDevice> wd = DynamicCast<WifiNetDevice>(d);
	Ptr<WifiPhy> wp = wd->GetPhy();
	Ptr<OvnisWifiPhy> ywp = DynamicCast<OvnisWifiPhy>(wp);
	Ptr<MobilityModel> mobility = ywp->GetMobility()->GetObject<MobilityModel> ();
	Vector position = mobility->GetPosition();
	if (position.x > 0 && position.y > 0) {
		// retrieves from packet
		double receivedDate;
		string receivedId;
		ovnis::Position2D receivedPos;
		RetrieveStatePacket(packet, receivedDate, receivedId, receivedPos);
		Log::getInstance().packetReceived();
		double dx   = receivedPos.x - position.x;         //horizontal difference
		double dy   = receivedPos.y - position.y;         //vertical difference
		double distance = sqrt(dx*dx + dy*dy);
		Log::getInstance().addDistance(distance);
		if (distance > 250) {
			cout << receivedDate << " vehicle " << vehicle.getId() << " received packet from " << receivedId << " from " << distance << "m " << endl;
		}

		if (vehicle.getId().compare("flow1.94") == 0 && receivedId.compare("flow1.96") == 0) {
			//cout << receivedDate << " vehicle " << vehicle.getId() << " received packet from " << receivedId << " from " << distance << "m " << endl;
			//cout << "sumo pos: " << pos.x << "," << pos.y << " ns3 pos: " << position.x << ", " << position.y << endl;
		}

	}

	// set the JAM flag if it's my edge however my speed is not 0
//	if (vehicle.getCurrentEdge() == received_edge) {
//		m_jam_state = true;
//	}
//
//	vehicle.tryReroute(received_edge);

	// do I re-send
//	double now = Simulator::Now().GetSeconds();
//	if (now - m_last_resend > RESEND_INTERVAL && now -received_date < PACKET_TTL) {
//		m_sendEvent = Simulator::Schedule(Seconds(TestApplication::rando.GetValue(0, RESEND_INTERVAL)), &TestApplication::SendMyState, this, received_date, received_edge);
//		m_last_resend = now;
//	}



	Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
	Ipv4Address address = ipv4->GetAddress(1, 0).GetLocal();
	double now = Simulator::Now().GetSeconds();
//	if (vehicle.getId() ==trackedVehicle) {
//		cout << vehicle.getId() << "(" << address << ") received [" << received_date << ", " << received_edge << "] from " << neighborIPv4Addr << std::endl;
//	}
	//Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
//		for (uint32_t j = 0; j < GetNode()->GetNDevices(); ++j) {
//			int32_t ifIndex = ipv4->GetInterfaceForDevice(GetNode()->GetDevice(j));
//			Ipv4InterfaceAddress address = ipv4->GetAddress(ifIndex, 0);
//			if (vehicle.getId() == trackedVehicle) {
//				cout << "- ipv4 (" << address << ") " << vehicle.getId() << endl;
//			}
//		}
}

void TestApplication::RetrieveStatePacket(Ptr<Packet> & packet, double & date, string & id, ovnis::Position2D & pos) {
	uint8_t * t;
	t = (uint8_t*) malloc(200 * sizeof(uint8_t));
	int size = packet->CopyData(t, 200);
	TagBuffer tg(t, t + size);

	date = tg.ReadDouble();
	uint32_t s_size = tg.ReadU32();
	char *s = (char*) malloc(s_size * sizeof(char));
	tg.Read((uint8_t*) s, s_size);
	id.assign(s, s_size);

	pos.x = tg.ReadDouble();
	pos.y = tg.ReadDouble();

	free(s);
	free(t);
}


}
