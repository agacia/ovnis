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
#include "vehicle.h"
#include "ovnisPacket.h"
#include "traci/structs.h"


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
	m_last_resend = 0;
	m_jam_state = false;
	arrived = false;
	flow = 1200;
	decisionTaken = false;
	notificationSent = false;
	trackedVehicleId = "flow1.59";
	tracked = false;

	// scenario
	scenario.setDecisionEdges(split("main_1b"));
	scenario.setNotificationEdges(split("main_2d bypass_2b"));
	map<string, Route> alternativeRoutes = map<string, Route>();
	alternativeRoutes["main"] = Route("main", "main_1 main_1b main_2 main_2a main_2b main_2c main_2d");
	alternativeRoutes["main"].setCapacity(1300);
	alternativeRoutes["bypass"] = Route("bypass", "main_1 main_1b bypass_1 bypass_2 bypass_2b");
	alternativeRoutes["bypass"].setCapacity(600);
	scenario.setAlternativeRoutes(alternativeRoutes);
}

const vector<string> FceApplication::split(string sentence) {
	vector<string> tokens;
	istringstream iss(sentence);
	copy(istream_iterator<string>(iss),
	         istream_iterator<string>(),
	         back_inserter<vector<string> >(tokens));
	return tokens;
}

FceApplication::~FceApplication() {
	arrived = true;
	Simulator::Cancel(m_travelTimeEvent);
}

void FceApplication::StartApplication(void) {
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
	m_socket->SetRecvCallback(MakeCallback(&FceApplication::ReceiveData, this));
	m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));

	Ipv4Address add = Ipv4Address::GetBroadcast();
	realTo = InetSocketAddress(add, FceApplication::m_port);

	m_simulationEvent = Simulator::Schedule(Seconds(rando.GetValue(0, SIMULATION_STEP_INTERVAL)), &FceApplication::GetCurrentSimulationState, this);
//	m_beaconEvent = Simulator::Schedule(Seconds(1+rando.GetValue(0, PROACTIVE_INTERVAL)), &FceApplication::Beacon, this);

}

void FceApplication::StopApplication(void) {
	arrived = true;
	vehicle.setArrivalTime(Simulator::Now().GetSeconds());
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

void FceApplication::Beacon(void) {
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
					Ptr<Packet> p = OvnisPacket::BuildPacket(now, vehicle.getId(), position.x, position.y, TRAVELTIME_PACKET_ID , entry.getLatestPacketId(), packetTime, entry.getLatestSenderId(), it->first, entry.getLatestValue());
					SendPacket(p);
					Log::getInstance().getStream("beaconing") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t" << vehicle.getItinerary().getId() << "\t" << vehicle.getItinerary().getTravelTime() << "\t" << vehicle.getItinerary().getLength() << endl;


				}
			}
		}
		m_beaconEvent = Simulator::Schedule(Seconds(PROACTIVE_INTERVAL), &FceApplication::Beacon, this);
	}
}

void FceApplication::GetCurrentSimulationState(void) {
	try {
//		cout << Simulator::Now().GetSeconds() << " " << vehicle.getId() << " " << arrived << endl;
		vehicle.requestCurrentVehicleState(Simulator::Now().GetSeconds());

		if (arrived == false) {
			AnalyseVehicleState();
			m_simulationEvent = Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &FceApplication::GetCurrentSimulationState, this);
		}
	}
	catch (TraciException & ex) {
		arrived = true;
	}
}

string FceApplication::GetEvent(vector<pair<string, double> > probabilities) {
    double r = FceApplication::rando.GetValue(0, 1);

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

string FceApplication::ChooseFlowAwareRoute(double flow, map<string,double> costs) {
	string chosenRouteId = ChooseMinTravelTimeRoute(costs);
	double capacity = vehicle.getAlternativeRoutes()[chosenRouteId].getCapacity();
	double flowRatioNeededToUseOtherAlternatives = (flow - capacity) / flow;
	if (flowRatioNeededToUseOtherAlternatives <= 0) {
		flowRatioNeededToUseOtherAlternatives = 0;
	}
	else if (flowRatioNeededToUseOtherAlternatives >= 1) {
		flowRatioNeededToUseOtherAlternatives = 1;
	}
	double random = FceApplication::rando.GetValue(0, 1);

	if (random < flowRatioNeededToUseOtherAlternatives) {
		map<string, double>::iterator it = costs.find(chosenRouteId);
		if (it != costs.end()) {
			costs.erase(it);
		}
		chosenRouteId = ChooseProbTravelTimeRoute(costs);
	}
	return chosenRouteId;
}

string FceApplication::ChooseProbTravelTimeRoute(map<string,double> costs) {
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

string FceApplication::ChooseMinTravelTimeRoute(map<string,double> costs) {
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

string FceApplication::ChooseRandomRoute() {
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

void FceApplication::PrintCosts(string fileName, map<string, double> costs) {
	Log::getInstance().getStream(fileName) << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t";
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		Log::getInstance().getStream(fileName) << it->first << "," << it->second << "\t" ;
	}
	Log::getInstance().getStream(fileName) << endl;
}

void FceApplication::DetectJam(string currentEdge) {

	double currentSpeed = vehicle.getCurrentSpeed();
	double maxLaneSpeed = vehicle.getItinerary().getEdgeMaxSpeed(currentEdge);
	//decide if JAM or not according to a threshold
	if (currentSpeed < (SPEED_THRESHOLD * maxLaneSpeed)) {
		if (m_time_jammed == 0) {
			m_time_jammed = Simulator::Now().GetSeconds();
		}
		if (Simulator::Now().GetSeconds() - m_time_jammed > JAMMED_TIME_THRESHOLD) {
			// I am in a jam for more than JAMMED_TIME_THRESHOLD send only ofr the first time
			if (!m_jam_state) {
				Ptr<Packet> packet = CreateWarningPacket();
				SendPacket(packet);
	//			m_sendEvent = Simulator::ScheduleNow(&TestApplication::SendMyState,this, Simulator::Now().GetSeconds(), vehicle.getCurrentEdge());
			}
			m_jam_state = true;
			}
	}
	else {
		// if ever I was in JAM state, then I am not anymore
		if (m_jam_state) {
			Ptr<Packet> packet = CreateWarningPacket();
			SendPacket(packet);
			m_jam_state = false;
			m_time_jammed = 0;
		}
	}
}

void FceApplication::TakeDecision(string currentEdge) {
	if (std::find(vehicle.getScenario().getDecisionEdges().begin(), vehicle.getScenario().getDecisionEdges().end(), currentEdge)!=vehicle.getScenario().getDecisionEdges().end() && !decisionTaken) {
		// costs global
		map<string, double> globalCosts = vehicle.getGlobalCosts();
		string global_minTravelTimeChoice = ChooseMinTravelTimeRoute(globalCosts);
		string global_proportionalProbabilisticChoice = ChooseProbTravelTimeRoute(globalCosts);
		string global_routeChoice = ChooseFlowAwareRoute(flow, globalCosts);
		// costs vanets
		map<string, double> vanetCosts = vehicle.getVanetCosts();
		string vanet_minTravelTimeChoice = ChooseMinTravelTimeRoute(vanetCosts);
		string vanet_proportionalProbabilisticChoice = ChooseProbTravelTimeRoute(vanetCosts);
		string vanet_routeChoice = ChooseFlowAwareRoute(flow, vanetCosts);
		// select strategy
		string routeChoice = global_minTravelTimeChoice;

		PrintCosts("global_costs", globalCosts);
		PrintCosts("vanet_costs", vanetCosts);
		Log::getInstance().getStream("global_routing_strategies") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t" << global_minTravelTimeChoice << "\t" << global_proportionalProbabilisticChoice << "\t" << global_routeChoice << endl;
		Log::getInstance().getStream("vanet_routing_strategies") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t" << vanet_minTravelTimeChoice << "\t" << vanet_proportionalProbabilisticChoice << "\t" << vanet_routeChoice << endl;
		Log::getInstance().getStream("routing") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t" << routeChoice << endl;

		Log::getInstance().vehicleEnter(Simulator::Now().GetSeconds(), routeChoice);

		vehicle.reroute(routeChoice);

		decisionTaken = true;
	}
}

/**
 * Local estimation of traffic condisions
 * Checks if the speed of vehicle differs from the expected
 * Specifies if traffic condisions are oversaturated/jammed or undersaturated/freeflow
 */
void FceApplication::EstimateTrafficConditionChanges(string currentEdge) {

	double currentSpeed = vehicle.getCurrentSpeed();
	double maxLaneSpeed = vehicle.getItinerary().getEdgeMaxSpeed(currentEdge);

	// decide if currentSpeed has changed significantly from the last speed
	if (abs(currentSpeed - vehicle.getLastSpeed()) > SPEED_SENSIVITY) {

	}
}

/**
 * One-hop broadcasting about all all known travel times on every edge of its own route
 * Knowledge aggregated from what was listened to from the neighbors
 * Send periodically every simulation step
 */
void FceApplication::SendTrafficConditions(string currentEdge) {

	// send all known travel times on every edge of its own route
//
//	Ptr<Packet> packet = CreateTravelTimePacket();
//	SendPacket(packet);

}

void FceApplication::SendRouteTravelTime(string currentEdge) {
	if (std::find(vehicle.getScenario().getNotificationEdges().begin(), vehicle.getScenario().getNotificationEdges().end(), currentEdge)!=vehicle.getScenario().getNotificationEdges().end() && !notificationSent) {

		Ptr<Packet> packet = CreateTravelTimePacket();
		Log::getInstance().vehicleLeave(Simulator::Now().GetSeconds(), vehicle.getItinerary().getId(), vehicle.getItinerary().getTravelTime());

		Log::getInstance().getStream("sending") << vehicle.getId() << "\t" << Simulator::Now().GetSeconds() << "\t" << vehicle.getItinerary().getId() << "\t" << vehicle.getItinerary().getTravelTime() << "\t" << vehicle.getItinerary().getLength();
		Log::getInstance().getStream("sending") << "\t" <<  Log::getInstance().getVehiclesOnRoute()[vehicle.getItinerary().getId()];
		Log::getInstance().getStream("sending") << endl;

		SendPacket(packet);
		notificationSent = true;
	}
}

/**
 * All parameters from simulation were already requested
 */
void FceApplication::AnalyseVehicleState() {

	string currentEdge = vehicle.getItinerary().getCurrentEdge().getId();

	SendTrafficConditions(currentEdge);

	TakeDecision(currentEdge);

	SendRouteTravelTime(currentEdge);
}

Ptr<Packet> FceApplication::CreateTravelTimePacket() {
	double date = Simulator::Now().GetSeconds();
	double routeTravelTime = vehicle.getItinerary().computeTravelTime();
	Vector position = mobilityModel->GetPosition();
	Log::getInstance().nextPacketId();
	Ptr<Packet> p = OvnisPacket::BuildPacket(date, vehicle.getId(), position.x, position.y, TRAVELTIME_PACKET_ID, Log::getInstance().getPacketId(), date, vehicle.getId(), vehicle.getItinerary().getId(), routeTravelTime);
	OvnisPacket packet = OvnisPacket(p);
	string routeId = packet.readString();
	double travelTime = packet.readDouble();
//	Simulator::Schedule(Seconds(RESEND_INTERVAL), &FceApplication::CheckRebroadcast, this, packet);
	return p;
}

Ptr<Packet> FceApplication::CreateWarningPacket() {
	double date = Simulator::Now().GetSeconds();
	string warningEdge = vehicle.getItinerary().getCurrentEdge().getId();
	double warningSpeed = vehicle.getCurrentSpeed();
	Vector position = mobilityModel->GetPosition();
	Log::getInstance().nextPacketId();
	Ptr<Packet> p = OvnisPacket::BuildPacket(date, vehicle.getId(), position.x, position.y, WARNING_PACKET_ID, Log::getInstance().getPacketId(), date, vehicle.getId(), warningEdge, warningSpeed);
	OvnisPacket packet = OvnisPacket(p);
	string edgeId = packet.readString();
	double speed = packet.readDouble();
	//	Simulator::Schedule(Seconds(RESEND_INTERVAL), &FceApplication::CheckRebroadcast, this, packet);
	return p;
}

void FceApplication::SendPacket(Ptr<Packet> packet) {
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

void FceApplication::ReceiveData(Ptr<Socket> socket) {

	Address neighborMacAddress;
    Ptr<Packet> packet = socket->RecvFrom(neighborMacAddress);
    OvnisPacket ovnisPacket(packet);

	if (ovnisPacket.getPacketType() == TRAVELTIME_PACKET_ID) {
		ReceiveTravelTimePacket(ovnisPacket);
	}
	if (ovnisPacket.getPacketType() == WARNING_PACKET_ID) {
		ReceiveWarningPacket(ovnisPacket);
	}
}


void FceApplication::ReceiveWarningPacket(OvnisPacket ovnisPacket) {
	Vector position = mobilityModel->GetPosition();
	double distance = ovnisPacket.computeDistance(position.x, position.y);
	string edgeId = ovnisPacket.readString();
	double speed = ovnisPacket.readDouble();

	vehicle.recordPacket(ovnisPacket.getPacketId());
	Log::getInstance().packetReceived();
	Log::getInstance().addDistance(distance);
	double now =  Simulator::Now().GetSeconds();
//	// if heard for the first time
	if (vehicle.getPacketCount(ovnisPacket.getPacketId()) == 1 && vehicle.getId() != ovnisPacket.getVehicleId()) {
		if (!decisionTaken && (now - ovnisPacket.getDate()) < PACKET_TTL ) {
			Log::getInstance().getStream("hearing") << ovnisPacket.getPacketType() << vehicle.getId() << " hearing from " << ovnisPacket.getSenderId() <<" about " << edgeId << "," << speed << "\t" << ovnisPacket.getVehicleId()<< " " << ovnisPacket.getDate() << "," << now - ovnisPacket.getDate() << endl;
		}
//		vehicle.recordDouble(routeId, ovnisPacket.getPacketId(), ovnisPacket.getVehicleId(), ovnisPacket.getDate(), travelTime);
		if (distance > BROADCASTING_DISTANCE_THRESHOLD) {
//			double waitingTime = ovnisPacket.computeWaitingTime(position.x, position.y);
			double waitingTime = FceApplication::rando.GetValue(0, RESEND_INTERVAL);
			Simulator::Schedule(Seconds(waitingTime), &FceApplication::TryRebroadcast, this, ovnisPacket);
		}
	}
}


void FceApplication::ReceiveTravelTimePacket(OvnisPacket ovnisPacket) {
	Vector position = mobilityModel->GetPosition();
	double distance = ovnisPacket.computeDistance(position.x, position.y);
	string routeId = ovnisPacket.readString();
	double travelTime = ovnisPacket.readDouble();
	vehicle.recordPacket(ovnisPacket.getPacketId());
	Log::getInstance().packetReceived();
	Log::getInstance().addDistance(distance);
	double now =  Simulator::Now().GetSeconds();
	// if heard for the first time
	if (vehicle.getPacketCount(ovnisPacket.getPacketId()) == 1 && vehicle.getId() != ovnisPacket.getVehicleId()) {
		if (!decisionTaken && (now - ovnisPacket.getDate()) < PACKET_TTL ) {
			Log::getInstance().getStream("hearing") << ovnisPacket.getPacketType() << ", hearing from " << routeId << " " << ovnisPacket.getSenderId() <<" about " << ovnisPacket.getVehicleId()<< " " << ovnisPacket.getDate() << " " << travelTime << " " << now - ovnisPacket.getDate() << endl;
		}
		vehicle.recordDouble(routeId, ovnisPacket.getPacketId(), ovnisPacket.getVehicleId(), ovnisPacket.getDate(), travelTime);
		if (distance > BROADCASTING_DISTANCE_THRESHOLD) {
//			double waitingTime = ovnisPacket.computeWaitingTime(position.x, position.y);
			double waitingTime = FceApplication::rando.GetValue(0, RESEND_INTERVAL);
			Simulator::Schedule(Seconds(waitingTime), &FceApplication::TryRebroadcast, this, ovnisPacket);
		}
	}
}

void FceApplication::TryRebroadcast(OvnisPacket packet) {
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
	Ptr<Packet> p = OvnisPacket::BuildPacket(now, vehicle.getId(), position.x, position.y, packet.getPacketType() , packet.getPacketId(), packetTime, packet.getVehicleId(), packet.getStringValue(), packet.getDoubleValue());
	SendPacket(p);
}

}
