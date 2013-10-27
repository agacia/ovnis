/**
 *
 *
 * Copyright (c) 2012-2013 University of Luxembourg
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
 * @file DsrApplication.cpp
 * 
 * @author Agata Grzybek <agatagrzybek@gmail.com>
 *
 */

#include "applications/DsrApplication.h"
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
#include "vehicle.h"
#include "ovnisPacket.h"
#include "traci/structs.h"


using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("DsrApplication");
NS_OBJECT_ENSURE_REGISTERED(DsrApplication);

TypeId DsrApplication::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::DsrApplication").SetParent<OvnisApplication>().AddConstructor<DsrApplication>();
	return tid;
}

DsrApplication::DsrApplication() {
	m_port = 2000;
	m_socket = 0;
	m_last_resend = 0;
	m_jam_state = false;
	arrived = false;
	flow = 1200;
	decisionTaken = false;
	notificationSent = false;
	trackedVehicleId = "flow1.59";
	tracked = false;

	// scenario
	scenario.setDecisionEdges(split("56640729#4 56640729#5"));
	scenario.setNotificationEdges(split("53349130#0"));
	map<string, Route> alternativeRoutes = map<string, Route>();
	alternativeRoutes["kennedy"] = Route("kennedy", "56640729#5 56640728#0 56640728#1 56640728#2 56640728#3 56640728#4 56640728#5 56640728#6 56640728#7 56640728#8 55444662 23595095#0 23595095#1 53349130#0");
	alternativeRoutes["kennedy"].setCapacity(1300);
	alternativeRoutes["adenauer"] = Route("adenauer", "56640729#5 56640724#0 56640724#1 56640724#2 56640724#3 56640724#4 48977754#0 48977754#1 48977754#2 48977754#3 48977754#4 48977754#5 95511865#0 95511865#1 126603964 -149693909#2 -149693909#1 -149693909#0 -149693907 49248917#0 49248917#1 149693908 126603969 53349130#0");
	alternativeRoutes["adenauer"].setCapacity(600);
	alternativeRoutes["thuengen"] = Route("thuengen", "56640729#5 95511899 95511885#0 95511885#1 95511885#2 95511885#3 95511885#4 95511885#5 -50649897 -37847306#1 56640728#8 55444662 23595095#0 23595095#1 53349130#0");
	alternativeRoutes["thuengen"].setCapacity(800);
	scenario.setAlternativeRoutes(alternativeRoutes);
}

const vector<string> DsrApplication::split(string sentence) {
	vector<string> tokens;
	istringstream iss(sentence);
	copy(istream_iterator<string>(iss),
	         istream_iterator<string>(),
	         back_inserter<vector<string> >(tokens));
	return tokens;
}

DsrApplication::~DsrApplication() {
	arrived = true;
	Simulator::Cancel(m_travelTimeEvent);
}

void DsrApplication::StartApplication(void) {
	NS_LOG_FUNCTION ("");

	double now = Simulator::Now().GetSeconds();

	string vehicleId = Names::FindName(GetNode());
	vehicle.initialize(vehicleId, now);
	vehicle.setScenario(scenario);

	vehicle.requestCurrentVehicleState(now);

	for (map<string,Route>::iterator it = vehicle.getAlternativeRoutes().begin(); it != vehicle.getAlternativeRoutes().end(); ++it) {
		Log::getInstance().vehicleOnRoadsInitialize(it->first);
	}

	mobilityModel = GetNode()->GetObject<ConstantVelocityMobilityModel>();

	Ptr<SocketFactory> socketFactory = GetNode()->GetObject<UdpSocketFactory>();
	m_socket = socketFactory->CreateSocket();
	m_socket->SetAllowBroadcast(true);
	m_socket->SetRecvCallback(MakeCallback(&DsrApplication::ReceiveData, this));
	m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));

	Ipv4Address add = Ipv4Address::GetBroadcast();
	realTo = InetSocketAddress(add, DsrApplication::m_port);

	m_simulationEvent = Simulator::Schedule(Seconds(rando.GetValue(0, SIMULATION_STEP_INTERVAL)), &DsrApplication::GetCurrentSimulationState, this);
//	m_beaconEvent = Simulator::Schedule(Seconds(1+rando.GetValue(0, PROACTIVE_INTERVAL)), &DsrApplication::Beacon, this);

}

void DsrApplication::StopApplication(void) {
	arrived = true;
	vehicle.setArrivalTime(Simulator::Now().GetSeconds());
}

void DsrApplication::DoDispose(void) {
	arrived = true;
	double now = Simulator::Now().GetSeconds();
	if (m_socket != NULL) {
		m_socket->Close();
	} else {
		NS_LOG_WARN("DsrApplication found null socket to close in StopApplication");
	}
	OvnisApplication::DoDispose();
}

void DsrApplication::Beacon(void) {
	int size = vehicle.getRecords().size();
	if (size > 0 && arrived == false) {
		double now = Simulator::Now().GetSeconds();
		map<string,RecordEntry> records = vehicle.getRecords();
		for (map<string, RecordEntry>::iterator it = records.begin(); it != records.end(); ++it) {
			RecordEntry entry = vehicle.getRecords()[it->first];
			if (entry.getLatestValue() != 0) {
				double packetTime = entry.getLatestTime();
				if (now - packetTime < PACKET_TTL) {
					Vector position = mobilityModel->GetPosition();
					Ptr<Packet> p = OvnisPacket::BuildTravelTimePacket(now, vehicle.getId(), position.x, position.y, TRAVELTIME_PACKET_ID , entry.getLatestPacketId(), packetTime, entry.getLatestSenderId(), it->first, entry.getLatestValue());
					SendPacket(p);
				}
			}
		}
		m_beaconEvent = Simulator::Schedule(Seconds(PROACTIVE_INTERVAL), &DsrApplication::Beacon, this);
	}
}

void DsrApplication::GetCurrentSimulationState(void) {
	try {
		vehicle.requestCurrentVehicleState(Simulator::Now().GetSeconds());

		if (arrived == false) {
			AnalyseVehicleState();
			m_simulationEvent = Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &DsrApplication::GetCurrentSimulationState, this);
		}
	}
	catch (TraciException & ex) {
		arrived = true;
	}
}

string DsrApplication::GetEvent(vector<pair<string, double> > probabilities) {
    double r = DsrApplication::rando.GetValue(0, 1);

//    Log::getInstance().getStream("prob") << r << "\t" << endl;
	for (vector<pair<string, double> >::iterator it = probabilities.begin(); it != probabilities.end(); ++it)
	{
		Log::getInstance().getStream("prob") << it->first << "," << it->second << " ";
	}
	Log::getInstance().getStream("prob") << endl;

    vector<pair<string, double> >::iterator it;
    //cout << r << endl;
    for (it = probabilities.begin(); it != probabilities.end(); ++it) {
    	r -= it->second;
//    	cout << "r: " << r << " " << it->first << "," << it->second << endl;
    	if (r < eps) {
			return it->first;
		}
    }
    return "";
}

bool comp_prob(const pair<string,double>& v1, const pair<string,double>& v2)
{
	return v1.second < v2.second;
}

string DsrApplication::ChooseFlowAwareRoute(double flow, map<string,double> costs) {
	string chosenRouteId = ChooseMinTravelTimeRoute(costs);
	double capacity = vehicle.getAlternativeRoutes()[chosenRouteId].getCapacity();
	double flowRatioNeededToUseOtherAlternatives = (flow - capacity) / flow;
	if (flowRatioNeededToUseOtherAlternatives <= 0) {
		flowRatioNeededToUseOtherAlternatives = 0;
	}
	else if (flowRatioNeededToUseOtherAlternatives >= 1) {
		flowRatioNeededToUseOtherAlternatives = 1;
	}
	double random = DsrApplication::rando.GetValue(0, 1);

	if (random < flowRatioNeededToUseOtherAlternatives) {
		map<string, double>::iterator it = costs.find(chosenRouteId);
		if (it != costs.end()) {
			costs.erase(it);
		}
		chosenRouteId = ChooseProbTravelTimeRoute(costs);
	}
	return chosenRouteId;
}

string DsrApplication::ChooseProbTravelTimeRoute(map<string,double> costs) {
	double minCost = numeric_limits<double>::max();
	double sumCost = 0;

	string chosenRouteId = "";

	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		sumCost += it->second;
	}
	int costsSize = costs.size();
	if (sumCost == 0 || costsSize == 0) {
		return "";
	}
	if (costsSize == 1) {
		return costs.begin()->first;
	}
	vector<pair<string, double> > sortedProbabilities = vector<pair<string, double> >();
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		double probability = (sumCost-it->second)/((costsSize-1)*sumCost);
		sortedProbabilities.push_back(pair<string,double>(it->first, probability));
	}
	sort(sortedProbabilities.begin(), sortedProbabilities.end(), comp_prob);

	chosenRouteId = GetEvent(sortedProbabilities);
	return chosenRouteId;
}

string DsrApplication::ChooseMinTravelTimeRoute(map<string,double> costs) {
	double minCost = numeric_limits<double>::max();
	string chosenRouteId = "";
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		double value = it->second;
		if (value > 0 && value < minCost) {
			minCost = value;
			chosenRouteId = it->first;
		}
	}
	return chosenRouteId;
}

string DsrApplication::ChooseRandomRoute() {
	string chosenRouteId = "";
	int chosenIndex = rand() % vehicle.getAlternativeRoutes().size();
	int i = 0;
	for (map<string,Route>::iterator it = vehicle.getAlternativeRoutes().begin(); it != vehicle.getAlternativeRoutes().end(); ++it) {
		if (i == chosenIndex) {
			chosenRouteId = it->second.getId();
		}
		++i;
	}
	return chosenRouteId;
}

void DsrApplication::PrintRouteGlobalRouteStats() {
	double now = Simulator::Now().GetSeconds();
	Log::getInstance().getStream("global") <<  vehicle.getId() << "\t" << now  << "\t";
	map<string,int> vehicles = Log::getInstance().getVehiclesOnRoute();
	map<string,double> travelTimes = Log::getInstance().getTravelTimesOnRoute();
	map<string,double> travelTimeDate = Log::getInstance().getTravelTimeDateOnRoute();
	for (map<string,int>::iterator it = vehicles.begin(); it != vehicles.end(); ++it) {
		double packetAge = travelTimeDate[it->first] == 0 ? 0 : now-travelTimeDate[it->first];
		Log::getInstance().getStream("global") << it->first << "," << it->second << "," << travelTimes[it->first] << "," << travelTimeDate[it->first] << "," << packetAge << "\t";
	}
	Log::getInstance().getStream("global") << endl;
}

void DsrApplication::PrintRouteLocalRouteStats() {
	double now = Simulator::Now().GetSeconds();
	Log::getInstance().getStream("local") <<  vehicle.getId() << "\t" << now  << "\t";
	map<string,RecordEntry> localrecords = vehicle.getRecords();
	for (map<string, RecordEntry>::iterator it = localrecords.begin(); it != localrecords.end(); ++it) {
		// todo
		int vehsOnRoute = 0;
		double packetAge = it->second.getLatestTime() == 0 ? 0 : now - it->second.getLatestTime();
		Log::getInstance().getStream("local") <<  it->first << "," << vehsOnRoute << "," <<  it->second.getLatestValue() << ","  << it->second.getLatestTime() << "," << packetAge << "\t";
	}
	Log::getInstance().getStream("local") << endl;
}

void DsrApplication::AnalyseVehicleState() {
	string currentEdge = vehicle.getCurrentEdgeId();

	if (std::find(scenario.getDecisionEdges().begin(), scenario.getDecisionEdges().end(), currentEdge)!=scenario.getDecisionEdges().end() && !decisionTaken) {

		PrintRouteGlobalRouteStats();
		PrintRouteLocalRouteStats();

		// vanets

		map<string, double> vanetCosts = vehicle.getVanetCosts(true);
		string vanet_minTravelTimeChoice = ChooseMinTravelTimeRoute(vanetCosts);
		string vanet_proportionalProbabilisticChoice = ChooseProbTravelTimeRoute(vanetCosts);
		string vanet_routeChoice = ChooseFlowAwareRoute(flow, vanetCosts);

		Log::getInstance().getStream("vanet_costs") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t";
		for (map<string, double>::iterator it = vanetCosts.begin(); it != vanetCosts.end(); ++it) {
			Log::getInstance().getStream("vanet_costs") << it->first << "," << it->second << "\t" ;
		}
		Log::getInstance().getStream("vanet_costs") << endl;
		Log::getInstance().getStream("vanet_routing_strategies") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t" << vanet_minTravelTimeChoice << "\t" << vanet_proportionalProbabilisticChoice << "\t" << vanet_routeChoice << endl;


		// global

		map<string, double> globalCosts = vehicle.getGlobalCosts();
		string global_minTravelTimeChoice = ChooseMinTravelTimeRoute(globalCosts);
		string global_proportionalProbabilisticChoice = ChooseProbTravelTimeRoute(globalCosts);
		string global_routeChoice = ChooseFlowAwareRoute(flow, globalCosts);

		Log::getInstance().getStream("global_costs") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t";
		for (map<string, double>::iterator it = globalCosts.begin(); it != globalCosts.end(); ++it) {
			Log::getInstance().getStream("global_costs") << it->first << "," << it->second << "\t" ;
		}
		Log::getInstance().getStream("global_costs") << endl;
		Log::getInstance().getStream("global_routing_strategies") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t" << global_minTravelTimeChoice << "\t" << global_proportionalProbabilisticChoice << "\t" << global_routeChoice << endl;


		// select strategy
		string routeChoice = vanet_routeChoice;
		Log::getInstance().getStream("routing") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t" << routeChoice << endl;
		Log::getInstance().vehicleEnter(Simulator::Now().GetSeconds(), routeChoice);
		vehicle.reroute(routeChoice);
		decisionTaken = true;


	}
	if (std::find(scenario.getNotificationEdges().begin(), scenario.getNotificationEdges().end(), currentEdge)!=scenario.getNotificationEdges().end() && !notificationSent) {
		Ptr<Packet> packet = CreateTravelTimePacket();
		SendPacket(packet);
		notificationSent = true;

		Log::getInstance().vehicleLeave(Simulator::Now().GetSeconds(), vehicle.getRoute().getId(), vehicle.getRoute().getTravelTime());
	}
}

Ptr<Packet> DsrApplication::CreateTravelTimePacket() {
	double date = Simulator::Now().GetSeconds();
	double routeTravelTime = vehicle.getRoute().computeTravelTime();

	Vector position = mobilityModel->GetPosition();
	Log::getInstance().nextPacketId();
	Ptr<Packet> p = OvnisPacket::BuildTravelTimePacket(date, vehicle.getId(), position.x, position.y, TRAVELTIME_PACKET_ID, Log::getInstance().getPacketId(), date, vehicle.getId(), vehicle.getRoute().getId(), routeTravelTime);
	OvnisPacket packet = OvnisPacket(p);
	string routeId = packet.readString();
	double travelTime = packet.readDouble();
	// startDate=date-travelTime
//	cout << "creating travel time packet : " << vehicle.getId() << "\t" << date << "\t" << routeId << "\t" << travelTime << "\t" << vehicle.getRoute().getLength() << endl;
	Log::getInstance().getStream("sending") << vehicle.getId() << "\t" << date << "\t" << routeId << "\t" << travelTime << "\t" << vehicle.getRoute().getLength() << endl;
//	Simulator::Schedule(Seconds(RESEND_INTERVAL), &DsrApplication::CheckRebroadcast, this, packet);
	return p;
}

void DsrApplication::SendPacket(Ptr<Packet> packet) {
	if (packet == NULL || arrived) {
		return;
	}
	try {
		if (m_socket->SendTo(packet, 0, realTo) == -1) {
			cerr << "CrashTestApplication : error while sending packet : " << m_socket->GetErrno() << "" << Socket::ERROR_OPNOTSUPP << endl;
			exit(-1);
		}
		else {
			Log::getInstance().packetSent();
		}
	}
	catch (exception & e) {
		cerr << e.what();
	}
}

void DsrApplication::ReceiveData(Ptr<Socket> socket) {

	Address neighborMacAddress;
    Ptr<Packet> packet = socket->RecvFrom(neighborMacAddress);
    OvnisPacket ovnisPacket(packet);

	if (ovnisPacket.getPacketType() == TRAVELTIME_PACKET_ID) {

		Vector position = mobilityModel->GetPosition();
		double distance = ovnisPacket.computeDistance(position.x, position.y);
//	    Ptr<Node> sender = Names::Find<Node>(ovnisPacket.getSenderId());
//		Ptr<ConstantVelocityMobilityModel> senderMobilityModel = sender->GetObject<ConstantVelocityMobilityModel>();
//		double distanceNow = mobilityModel->GetDistanceFrom(senderMobilityModel);
		string routeId = ovnisPacket.readString();
		double travelTime = ovnisPacket.readDouble();
		vehicle.recordPacket(ovnisPacket.getPacketId());
		Log::getInstance().packetReceived();
		Log::getInstance().addDistance(distance);
		double now =  Simulator::Now().GetSeconds();
		// if heard for the first time
		if (vehicle.getPacketCount(ovnisPacket.getPacketId()) == 1 && vehicle.getId() != ovnisPacket.getVehicleId()) {
			if (!decisionTaken && (now - ovnisPacket.getDate()) > PACKET_TTL ) {
				Log::getInstance().getStream("hearing") << "hearing from " << routeId << " " << ovnisPacket.getSenderId() <<" about " << ovnisPacket.getVehicleId()<< " " << ovnisPacket.getDate() << " " << travelTime << " " << now - ovnisPacket.getDate() << endl;
			}
			vehicle.recordDouble(routeId, ovnisPacket.getPacketId(), ovnisPacket.getVehicleId(), ovnisPacket.getDate(), travelTime);
			if (distance > BROADCASTING_DISTANCE_THRESHOLD) {
	//			double waitingTime = ovnisPacket.computeWaitingTime(position.x, position.y);
				double waitingTime = DsrApplication::rando.GetValue(0, RESEND_INTERVAL);
				Simulator::Schedule(Seconds(waitingTime), &DsrApplication::TryRebroadcast, this, ovnisPacket);
			}
		}
	}
}

void DsrApplication::TryRebroadcast(OvnisPacket packet) {
	double now = Simulator::Now().GetSeconds();
	double packetTime = packet.getDate();
	// drop if was already forwarded or packet is old
	if (vehicle.getPacketCount(packet.getPacketId()) > 1 || (now - packetTime) > PACKET_TTL) {
		return;
	}
	Log::getInstance().packetForwarded();
	Vector position = mobilityModel->GetPosition();
	if (Log::getInstance().forwardedPackets.count(packet.getPacketId())== 0) {
		Log::getInstance().forwardedPackets[packet.getPacketId()] = 0;
	}
//	Log::getInstance().getStream("broadcasting") << Simulator::Now().GetSeconds() << "\t" << packet.getPacketId() << "\t" << vehicle.getId() << "\t forwarding from \t" << packet.getSenderId() << "\t vehicleId: " << packet.getVehicleId() << "\t decisionTaken: " << decisionTaken << "\t" << packet.getStringValue() <<","<< packet.getDoubleValue() << endl;
	++Log::getInstance().forwardedPackets[packet.getPacketId()];
	Ptr<Packet> p = OvnisPacket::BuildTravelTimePacket(now, vehicle.getId(), position.x, position.y, packet.getPacketType() , packet.getPacketId(), packetTime, packet.getVehicleId(), packet.getStringValue(), packet.getDoubleValue());
	SendPacket(p);
}

}

//void DsrApplication::CheckRebroadcast(OvnisPacket ovnisPacket) {
//
//	Log::getInstance().getStream("checking") << Simulator::Now().GetSeconds() << "\t" << ovnisPacket.getPacketId() << "\t" << vehicle.getId() << "\t heared his own message  " << ovnisPacket.getVehicleId() << " " << vehicle.getPacketCount(ovnisPacket.getPacketId()) << " times " << endl;
//	// rebroadcast
//
//}

//	Address neighborMacAddress;
//	InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom(neighborMacAddress);
//	Ipv4Address neighborIPv4Addr = inetSourceAddr.GetIpv4();
//
//

		// do I re-send
	//	double now = Simulator::Now().GetSeconds();
	//	if (now - m_last_resend > RESEND_INTERVAL && now -received_date < PACKET_TTL) {
	//		m_sendEvent = Simulator::Schedule(Seconds(DsrApplication::rando.GetValue(0, RESEND_INTERVAL)), &DsrApplication::SendMyState, this, received_date, received_edge);
	//		m_last_resend = now;
	//	}

	//	Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
	//	Ipv4Address address = ipv4->GetAddress(1, 0).GetLocal();
	//	double now = Simulator::Now().GetSeconds();
	//	if (vehicle.getId() ==trackedVehicle) {
	//		 << vehicle.getId() << "(" << address << ") received [" << received_date << ", " << received_edge << "] from " << neighborIPv4Addr << std::endl;
	//	}
		//Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
	//		for (uint32_t j = 0; j < GetNode()->GetNDevices(); ++j) {
	//			int32_t ifIndex = ipv4->GetInterfaceForDevice(GetNode()->GetDevice(j));
	//			Ipv4InterfaceAddress address = ipv4->GetAddress(ifIndex, 0);
	//			if (vehicle.getId() == trackedVehicle) {
	//				cout << "- ipv4 (" << address << ") " << vehicle.getId() << endl;
	//			}
	//		}


