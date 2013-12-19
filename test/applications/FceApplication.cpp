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
 * @author Agata Grzybek
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
#include "ovnis-constants.h"
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
//			AddAttribute("ApplicationParams", "Application parameters", PointerValue(), MakePointerAccessor(&FceApplication::applicationParams), MakePointerChecker<Object>());
	return tid;
}

FceApplication::FceApplication() {
	m_port = 2000;
	m_socket = 0;
	decisionTaken = false;
	notificationSent = false;
	isCheater = false;
	neededProbabilistic = false;
	startReroute = 0;
	isVanet = true;
	_neighborCount = 0;
	m_params["vanetKnowlegePenetrationRate"] = "1"; // re rest uses global ideal knowledge;
	m_params["vanetDisseminationPenetrationRate"] = "1"; // PENETRATION_RATE;
	m_params["cheatersRatio"] = "0"; // CHEATER_RATE; // always shortest
	m_params["accidentStartTime"] = "300"; // ACCIDENT_START_TIME;
	m_params["accidentStopTime"] = "1300"; // ACCIDENT_END_TIME;
	m_params["networkId"] = "Highway"; // "Kirchberg";
	m_params["routingStrategies"] = "noRouting,shortest,probabilistic,hybrid";
	m_params["routingStrategiesProbabilities"] = "0,1,0,0"; // no-routing - uninformed drivers,
	m_params["costFunctions"] = "travelTime,congestionLength,delayTime";
	m_params["costFunctionProbabilities"] = "1,0,0";
}

void FceApplication::InitializeParams() {
	for (map<string, string>::iterator i = _applicationParams.begin(); i != _applicationParams.end(); ++i) {
		m_params[i->first] = i->second;
	}
	Log::getInstance().getStream("scenarioSettings") << "--------------------------" << endl;
	for (map<string, string>::iterator i = m_params.begin(); i != m_params.end(); ++i) {
		Log::getInstance().getStream("scenarioSettings") << i->first << "\t" << i->second << endl;
	}

	// select routing strategy
	m_params["routingStrategy"] = "hybrid";
	vector<string> routingStrategies = CommonHelper::split(m_params["routingStrategies"], ',');
	vector<string> routingStrategiesProbabilities = CommonHelper::split(m_params["routingStrategiesProbabilities"], ',');
	int i = 0;
	map<string, double> probabilities;
	for (vector<string>::iterator it =  routingStrategies.begin(); it != routingStrategies.end(); ++it) {
		probabilities[*it] = atof(routingStrategiesProbabilities[i].c_str());
//		string routingStrategy = *it;
//		probabilities[routingStrategy] = 1.0;
		Log::getInstance().getStream("scenarioSettings") << " strategy: " << *it << ",probability: " << probabilities[*it] << "\t";
		++i;
	}
	m_params["routingStrategy"] = TIS::getInstance().getEvent(probabilities);
	Log::getInstance().getStream("scenarioSettings") << "\nSelected routing strategy " << m_params["routingStrategy"] << endl;
	Log::getInstance().getStream("selectedStrategies") << m_params["routingStrategy"] << endl;
	SetNetwork(m_params["networkId"]);
}

/**
 * TODO read from an external config file
 */
void FceApplication::SetNetwork(string networkId) {
	string decisionEdge = "";
	string notificationEdge = "";
	map<string, Route> alternativeRoutes = map<string, Route>();
	vector<string> route_ids = vector<string>();
	vector<double> route_capacities = vector<double>();
	vector<string> route_strategies = vector<string>();
	if (networkId == "Highway") {
		alternativeRoutes["bypass"] = Route("bypass", "");
		decisionEdge = "pre_2";
		notificationEdge = "main_6 bypass_3";
		route_ids.push_back("main");
		route_ids.push_back("bypass");
		route_capacities.push_back(900);
		route_capacities.push_back(1500);
		route_strategies.push_back("pre_1 pre_2 main_1 main_2a main_2b main_3a main_3b main_4a main_4b main_5a main_5b main_6");
		route_strategies.push_back("pre_1 pre_2 bypass_1 bypass_2 bypass_3");
	}
	else if (networkId == "Kirchberg") {
		decisionEdge = "56640729#5";
		notificationEdge = "53349130#1";
		route_ids.push_back("routedist#0"); // kennedy
		route_ids.push_back("routedist#1"); // asenauer
		route_ids.push_back("routedist#2"); // thuengen
		route_capacities.push_back(1000);
		route_capacities.push_back(600);
		route_capacities.push_back(800);
		route_strategies.push_back("56640729#0 56640729#1 56640729#2 56640729#3 56640729#4 56640729#5 56640728#0 56640728#1 56640728#2 56640728#3 56640728#4 56640728#5 56640728#6 56640728#7 56640728#8 55444662 23595095#0 23595095#1 53349130#0 53349130#1");
		route_strategies.push_back("56640729#0 56640729#1 56640729#2 56640729#3 56640729#4 56640729#5 56640724#0 56640724#1 56640724#2 56640724#3 56640724#4 48977754#0 48977754#1 48977754#2 48977754#3 48977754#4 48977754#5 95511865#0 95511865#1 126603964 -149693909#2 -149693909#1 -149693909#0 -149693907 49248917#0 49248917#1 149693908 126603969 53349130#0 53349130#1");
		route_strategies.push_back("56640729#0 56640729#1 56640729#2 56640729#3 56640729#4 56640729#5 95511899 95511885#0 95511885#1 95511885#2 95511885#3 95511885#4 95511885#5 -50649897 -37847306#1 56640728#8 55444662 23595095#0 23595095#1 53349130#0 53349130#1");
	}
	int i = 0;
	for (vector<string>::iterator it = route_ids.begin(); it < route_ids.end(); ++it, ++i) {
		alternativeRoutes[*it] = Route(*it, route_strategies[i]);
		alternativeRoutes[*it].setCapacity(route_capacities[i]);
	}
	network.setDecisionEdges(CommonHelper::split(decisionEdge, ' '));
	network.setNotificationEdges(CommonHelper::split(notificationEdge, ' '));
	network.setAlternativeRoutes(alternativeRoutes);
}

FceApplication::~FceApplication() {
	running = false;
	Simulator::Cancel(m_trafficInformationEvent);
	Simulator::Cancel(m_neighborInformationEvent);
}

void FceApplication::StartApplication(void) {
	NS_LOG_FUNCTION ("");

	InitializeParams();
	// initialize vehicle
	string vehicleId = Names::FindName(GetNode());
	vehicle.initialize(vehicleId, Simulator::Now().GetSeconds());
	Log::getInstance().getStream("scenarioSettings") << "vehicleId\t" << vehicle.getId() << endl;
	vehicle.setScenario(network);

	// add to the centralised Traffic Information System vehicle's routes (edges) of interest
	TIS::getInstance().initializeStaticTravelTimes(vehicle.getScenario().getAlternativeRoutes());

	// ns3
	mobilityModel = GetNode()->GetObject<ConstantVelocityMobilityModel>();
	ToggleNeighborDiscovery(true);
	Ptr<SocketFactory> socketFactory = GetNode()->GetObject<UdpSocketFactory> ();
	m_socket = socketFactory->CreateSocket();
	m_socket->Connect(InetSocketAddress (Ipv4Address::GetBroadcast(), m_port));
	m_socket->SetAllowBroadcast(true);
	m_socket->SetRecvCallback(MakeCallback(&FceApplication::ReceivePacket, this));
	m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port));

	// start simualtion
	running = true;
	double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX * SIMULATION_STEP_INTERVAL;
	double r2 = (double)(rand()%RAND_MAX)/(double)RAND_MAX * TRAFFIC_INFORMATION_SENDING_INTERVAL;
	cout << "r " << r << " r2 " << r2 << endl;
//	m_trafficInformationEvent = Simulator::Schedule(Seconds(0), &FceApplication::SendTrafficInformation, this);
	m_neighborInformationEvent = Simulator::Schedule(Seconds(r<r2?r:r2), &FceApplication::SendNeighborInformation, this);
	m_simulationEvent = Simulator::Schedule(Seconds(r<r2?r2:r), &FceApplication::SimulationRun, this);

}

void FceApplication::ToggleNeighborDiscovery(bool on) {
	if (on) {
		std::ostringstream oss;
		oss << "/NodeList/" << GetNode()->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::BeaconingAdhocWifiMac/NeighborLost";
		Config::Connect(oss.str(), MakeCallback(&FceApplication::NeighborLost, this));
		std::ostringstream oss2;
		oss2 << "/NodeList/" << GetNode()->GetId() << "/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::BeaconingAdhocWifiMac/NewNeighbor";
		Config::Connect(oss2.str(), MakeCallback(&FceApplication::NewNeighborFound, this));
	}
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

FceApplication::MacAddrMap FceApplication::getNeighborList() {
	if (vehicle.getId() == "1.7") {
		cout << Simulator::Now().GetSeconds() << " vehicle " << vehicle.getId() << " has " << m_neighborList.size() << " neighbors and received ovnis packets: " << _neighborCount << endl;
	}
	_neighborCount = 0;
//	for (MacAddrMapIterator i = m_neighborList.begin(); i != m_neighborList.end (); ++i) {
//		std::cout << i->first << "," << i->second << "\t";
//	}
//	cout << endl;
	return m_neighborList;
}

void FceApplication::SimulationRun(void) {
	try {
		if (running == true) {
			getNeighborList();
			double now = Simulator::Now().GetSeconds();
			bool edgeChanged = vehicle.requestCurrentEdge(now);
			string currentEdgeId = vehicle.getItinerary().getCurrentEdge().getId();

			if (edgeChanged) {
				OnEdgeChanged(now, currentEdgeId);
			}

			// if approaching an intersection take decision which route to take from current to destination edge
			bool isDecisionPoint = find(vehicle.getScenario().getDecisionEdges().begin(), vehicle.getScenario().getDecisionEdges().end(), currentEdgeId) != vehicle.getScenario().getDecisionEdges().end();
			if (isDecisionPoint && !decisionTaken) {
				isDense = false;
				isCongested = false;
				map<string, double> cost = EstimateTravelCostBasedOnCentralised(now, currentEdgeId);
				// vanets
				double vanetKnowledgeRatio = atof(m_params["vanetKnowlegePenetrationRate"].c_str());
				double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX;
				isVanet =  r < vanetKnowledgeRatio;
				if (isVanet) {
					cost = EstimateTravelCostBasedOnVanets(now, currentEdgeId, m_params["costFunction"]);
				}
				double cheatersRatio = atof(m_params["cheatersRatio"].c_str());
				routeChoice = ChooseRoute(now, currentEdgeId, cost, m_params["routingStrategy"], cheatersRatio);
				vehicle.reroute(routeChoice);
			}

			// if approaching the point that we want to evaluate
			bool isReportingPoint = find(vehicle.getScenario().getNotificationEdges().begin(), vehicle.getScenario().getNotificationEdges().end(), currentEdgeId) != vehicle.getScenario().getNotificationEdges().end();
			if (isReportingPoint && !notificationSent) {
				OnReporting(now, currentEdgeId);
			}

			m_simulationEvent = Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &FceApplication::SimulationRun, this);
		}
	}
	catch (TraciException & ex) {
		running = false;
	}
}

/**
 * The vehicle is entering a new edge it has already traced the time on its itinerary.
 * Broadcast information about it and about travel time on the last edge
 */
void FceApplication::OnEdgeChanged(double now, string currentEdgeId) {
	Edge lastEdge = vehicle.getItinerary().getLastEdge();
	double travelTimeOnLastEdge = lastEdge.getTravelTime();
	string lastEdgeId = lastEdge.getId();
	Vector position = mobilityModel->GetPosition();
	Log::getInstance().reportEdgePosition(lastEdge.getId(), position.x, position.y);
	Ptr<Packet> p = OvnisPacket::BuildChangedEdgePacket(now, vehicle.getId(), position.x, position.y, CHANGED_EDGE_PACKET_ID, lastEdgeId, travelTimeOnLastEdge, currentEdgeId);
//	SendPacket(p);
	TIS::getInstance().reportEndingEdge(vehicle.getId(), lastEdgeId,  travelTimeOnLastEdge);
	//	Log::getInstance().getStream(lastEdgeId) << "vehicle: " << vehicle.getId() << "\t now:" << now << "\t travelTimeOnLastEdge: " << travelTimeOnLastEdge << "\t vehs on route: " <<  TIS::getInstance().getVehiclesOnRoute("main") << endl;
}


map<string, double> FceApplication::EstimateTravelCostBasedOnCentralised(double now, string currentEdgeId) {
	map<string, double> globalCosts = TIS::getInstance().getCosts(vehicle.getScenario().getAlternativeRoutes(), currentEdgeId, vehicle.getDestinationEdgeId());
	return globalCosts;
}

map<string, double> FceApplication::EstimateTravelCostBasedOnVanets(double now, string currentEdgeId, string costFunction) {
	Vector position = mobilityModel->GetPosition();
	Log::getInstance().getStream("vanets_knowledge") << now << "\t" << position.x << "\t" << position.y << "\t" << currentEdgeId << "\t" << vehicle.getDestinationEdgeId() << "\t";
	string endEdgeId = vehicle.getDestinationEdgeId();
	// for shortest we need to reset (do dynamic value)
	map<string,double> routeTTL;
	for (map<string,Route>::iterator it = vehicle.getScenario().getAlternativeRoutes().begin(); it != vehicle.getScenario().getAlternativeRoutes().end(); ++it) {
		routeTTL[it->first] = PACKET_TTL;
	}
	if (m_params["routingStrategy"] == "shortest") {
		for (map<string,Route>::iterator it = vehicle.getScenario().getAlternativeRoutes().begin(); it != vehicle.getScenario().getAlternativeRoutes().end(); ++it) {
			routeTTL[it->first] = TIS::getInstance().computeStaticCostExcludingMargins(it->first, currentEdgeId, endEdgeId) * (1+ CONGESTION_THRESHOLD);
//			routeTTL[it->first] = TIS::getInstance().computeStaticCostExcludingMargins(it->first, currentEdge, endEdgeId);
//			routeTTL[it->first] = 120;
		}
	}
	vanetsKnowledge.analyseLocalDatabase(vehicle.getScenario().getAlternativeRoutes(), currentEdgeId, endEdgeId, routeTTL, true);
	map<string, double> travelTimeCost = vanetsKnowledge.getTravelTimesOnRoutes();
//	for (map<string, double>::iterator it = travelTimeCost.begin(); it != travelTimeCost.end(); ++it) {
//		cout << "in fce read knowledge: route " << it->first << ", travel time " << it->second << endl;
//	}
	map<string, double> delayTimeCost = vanetsKnowledge.getDelayOnRoutes();
	map<string, double> congestionLengthCost = vanetsKnowledge.getCongestedLengthOnRoutes();
	Log::getInstance().logMap("delay_scale", now, delayTimeCost, 0);
	Log::getInstance().logMap("congestion_scale", now, congestionLengthCost, 0);
	Log::getInstance().logMap("travelTime_scale", now, travelTimeCost, 0);
	map<string, double> cost = travelTimeCost;
	if (costFunction == "congestionLength") {
		cost = congestionLengthCost;
	}
	if (costFunction == "delayTime") {
		cost = delayTimeCost;
	}

	isDense = vanetsKnowledge.isDenseFlow();
	isCongested = vanetsKnowledge.isCongestedFlow();

	CalculateError(currentEdgeId);

	return cost;
}

string FceApplication::ChooseRoute(double now, string currentEdgeId, map<string, double> routeCost, string routingStrategy, double cheatersRatio) {
	string shortest_choice = TIS::getInstance().chooseMinCostRoute(routeCost);
	map<string, double> correlatedValues = AnalyseRouteCorrelation();
	string probabilistic_choice =  TIS::getInstance().chooseProbTravelTimeRoute(routeCost, correlatedValues);
	bool needProbabilistic = isCongested;
	//	needProbabilistic = isCongested || isDense;
	//	needProbabilistic = vanetsKnowledge.getSumDelay() > 0;
	//	bool isAccident = now > m_params["accidentStartTime"] && now < m_params["accidentStopTime"];
	//	needProbabilistic = isAccident;
	TIS::getInstance().setCongestion(needProbabilistic, isDense, isCongested);

//	cout << "Routing. Need probabilistic (iscCngested) " << needProbabilistic << " " <<  isCongested << endl;
//	cout << "Routing. " << routingStrategy << " " << endl;

	routeChoice = shortest_choice;
	if (shortest_choice != "") {
		routeChoice = shortest_choice;
	}
	if (routingStrategy == "probabilistic" && probabilistic_choice != "") {
		  routeChoice = probabilistic_choice;
	}
	// hybrid - probabilistic only if 'needed'
	if (routingStrategy == "hybrid" && needProbabilistic && probabilistic_choice != "") {
		routeChoice = probabilistic_choice;
		Log::getInstance().needProbabilistic ++;
	}
	// no routing
	if (routingStrategy == "noRouting") {
		routeChoice = vehicle.getItinerary().getId();
	}
	// cheaters
	if (routeChoice != shortest_choice) {
		isCheater = false;
		Log::getInstance().couldCheat ++;
		double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX * 100;
//		cout << "cheatersRatio " << cheatersRatio << ", r " << r << ", routeChoice: " << routeChoice << " shortest_choice: " << shortest_choice << endl;
		if (cheatersRatio > 0 && r < cheatersRatio) {
			routeChoice = shortest_choice;
			isCheater = true;
		}
		if (cheatersRatio < 0 && -r > cheatersRatio) {
			routeChoice = probabilistic_choice;
			isCheater = true;
		}
	}

	if (isCheater) {
		Log::getInstance().cheaters ++;
	}

	selfishExpectedTravelTime = routeCost[shortest_choice];
	expectedTravelTime = routeCost[routeChoice];

	startReroute = now;
	decisionEdgeId = currentEdgeId;
	decisionTaken = true;

	Log::getInstance().getStream("scenarioSettings") << "chosen routingStrategy\t" << routingStrategy << endl;
	Log::getInstance().getStream("scenarioSettings") << "needProbabilistic\t" << needProbabilistic << endl;
	Log::getInstance().getStream("scenarioSettings") << "routeChoice\t" << routeChoice << endl;
	// todo get newEdgeId (pointer to the next edge)
	TIS::getInstance().reportStartingRoute(vehicle.getId(), currentEdgeId, vehicle.getItinerary().getId(), currentEdgeId,
			routeChoice, vehicle.getOriginEdgeId(), vehicle.getDestinationEdgeId(), isCheater,
			needProbabilistic, expectedTravelTime, selfishExpectedTravelTime);

	return routeChoice;
}

/**
 * Report to TIS the total travel time on the route between the decision edge and the current edge
 */
void FceApplication::OnReporting(double now, string currentEdgeId) {
	// to fix a bug if a vehicle didn't visited the decision edge because of teleportation
	if (startReroute > vehicle.getItinerary().getStartTime()) {
		string routeId = vehicle.getItinerary().getId();
		double travelTime = vehicle.getItinerary().computeTravelTime(decisionEdgeId, currentEdgeId);
		double travelTime2 = vehicle.getItinerary().computeTravelTime(vehicle.getItinerary().getEdgeIds()[0], currentEdgeId);
		cout << Simulator::Now().GetSeconds() << "\t" << routeId << "\t"
				<< decisionEdgeId << "-" << currentEdgeId << "\t" << travelTime << "\t"
				<< vehicle.getItinerary().computeStaticCostExcludingMargins(decisionEdgeId, currentEdgeId) << "\t" << vehicle.getItinerary().computeLength(decisionEdgeId, currentEdgeId) << "[m]\t"
				<< vehicle.getItinerary().getEdgeIds()[0] << "-" << vehicle.getItinerary().getEdgeIds()[vehicle.getItinerary().getEdgeIds().size()-1] << "\t"
				<< travelTime2 << "\t"
				<< vehicle.getItinerary().computeStaticCostExcludingMargins(vehicle.getItinerary().getEdgeIds()[0], currentEdgeId) << "\t" << vehicle.getItinerary().computeLength() << "[m]" << endl;
		TIS::getInstance().reportEndingRoute(vehicle.getId(), routeId, decisionEdgeId, currentEdgeId,
				startReroute, travelTime, isCheater, selfishExpectedTravelTime, expectedTravelTime, neededProbabilistic,
				m_params["routingStrategy"], vehicle.getStart());
		notificationSent = true;
	}
}

map<string, double> FceApplication::AnalyseRouteCorrelation() {
	string currentEdgeId = vehicle.getItinerary().getCurrentEdge().getId();
	string endEdgeId = vehicle.getDestinationEdgeId();
	map<string,map<string,vector<string> > > correlated = vanetsKnowledge.analyseCorrelation(vehicle.getScenario().getAlternativeRoutes(), currentEdgeId, endEdgeId);
	//map<string,map<string,vector<string> > > correlated;
	map<string, double> correlatedValues;
	string defaultRoute = vehicle.getItinerary().getId();
	correlatedValues[defaultRoute] = 0;
	Log::getInstance().getStream("scenarioSettings") << "correlation of default route " << defaultRoute << "\t";
	for (map<string,vector<string> >::iterator itCorrelatedTo = correlated[defaultRoute].begin(); itCorrelatedTo != correlated[defaultRoute].end(); ++itCorrelatedTo) {
		correlatedValues[itCorrelatedTo->first] = itCorrelatedTo->second.size()*2;
		Log::getInstance().getStream("scenarioSettings") << itCorrelatedTo->first << "," << correlatedValues[itCorrelatedTo->first] << "\t";
	}
	Log::getInstance().getStream("scenarioSettings") << endl;
	return correlatedValues;
}

void FceApplication::CalculateError(string currentEdge) {

//	string currentEdgeId = vehicle.getItinerary().getCurrentEdge().getId();
	map<string, double> sumoEdgesCosts = vehicle.getSumoCosts(currentEdge);
	map<string, double> vanetEdgesCosts = vanetsKnowledge.getEdgesCosts(vehicle.getScenario().getAlternativeRoutes(), currentEdge, vehicle.getDestinationEdgeId()); // refer to the last call of vanetsKnowledge.getCosts !

	double now = Simulator::Now().GetSeconds();

	Log::getInstance().getStream("edges_sumo") << now << "\t";
	for (map<string, double>::iterator it=sumoEdgesCosts.begin(); it!=sumoEdgesCosts.end(); ++it) {
		Log::getInstance().getStream("edges_sumo") << "\t"  << it->first << "," << it->second << "\t";
	}
	Log::getInstance().getStream("edges_sumo") << endl;
	Log::getInstance().getStream("edges_vanet") << now << "\t";
	for (map<string, double>::iterator it=vanetEdgesCosts.begin(); it!=vanetEdgesCosts.end(); ++it) {
		Log::getInstance().getStream("edges_vanet") << it->first << "," << it->second << "\t";
	}
	Log::getInstance().getStream("edges_vanet") << endl;


	bool usePerfect = true;
	Log::getInstance().getStream("scenarioSettings") << "usePerfect" << "," << usePerfect << "\n";
	map<string, double> perfectEdgesCosts = vanetsKnowledge.getEdgesCosts(vehicle.getScenario().getAlternativeRoutes(), currentEdge, vehicle.getDestinationEdgeId(), usePerfect); // refer to the last call of vanetsKnowledge.getCosts !
	Log::getInstance().getStream("edges_perfect") << now << "\t";
	for (map<string, double>::iterator it=vanetEdgesCosts.begin(); it!=vanetEdgesCosts.end(); ++it) {
		Log::getInstance().getStream("edges_perfect") << it->first << "," << it->second << "\t";
	}
	Log::getInstance().getStream("edges_perfect") << endl;


	// error
	double error = 0;
	double sumSumo = 0;
	double sumVanet = 0;
	for (map<string, double>::iterator it=sumoEdgesCosts.begin(); it!=sumoEdgesCosts.end(); ++it) {
//		if (it->second < 1000) {
			error += abs(it->second - vanetEdgesCosts[it->first]);
			sumSumo += it->second;
			sumVanet += vanetEdgesCosts[it->first];
//		}
	}
	double percent = (sumSumo + sumVanet) == 0 ? 0 : error / (sumSumo + sumVanet);
	Log::getInstance().getStream("edges_error") << now << "\t" << error << "\t" << percent << "\t" << sumVanet << "\t" << sumSumo << "\t" << sumVanet-sumSumo << endl;

}

void FceApplication::SendNeighborInformation(void) {
	if (running == true) {
		Vector position = mobilityModel->GetPosition();
		if (vehicle.getId() == "1.7") {
			cout << Simulator::Now().GetSeconds() << " vehicle " << vehicle.getId() << " sending neighbor packet" << endl;
		}
		Ptr<Packet> p = OvnisPacket::BuildPacket(Simulator::Now().GetSeconds(), vehicle.getId(), position.x, position.y, STATE_PACKET_ID, vehicle.getItinerary().getCurrentEdge().getId(), vehicle.getCurrentSpeed());
		SendPacket(p);
		m_neighborInformationEvent = Simulator::Schedule(Seconds(NEIGHBOR_INFORMATION_SENDING_INTERVAL), &FceApplication::SendNeighborInformation, this);
	}
}

void FceApplication::SendTrafficInformation(void) {
	if (running == true) {
		Vector position = mobilityModel->GetPosition();
		vector<Data> records = dissemination.getTrafficInformationToSend(vanetsKnowledge, vehicle.getEdgesAhead());
		if (records.size() > 0) {
			Ptr<Packet> p = OvnisPacket::BuildTrafficInfoPacket(Simulator::Now().GetSeconds(), vehicle.getId(), position.x, position.y, TRAFFICINFO_PACKET_ID, records.size(), records);
			if (vehicle.getId() == "1.7") {
				cout << Simulator::Now().GetSeconds() << " vehicle " << vehicle.getId() << " sending traffic packet" << endl;
			}
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
			Log::getInstance().packetSent(); // moved to beaconing-adhoc-wifi-mac.cpp
			Log::getInstance().nextPacketId();
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

		_neighborCount ++;
//		_neighbors[senderId] = "1";
		Log::getInstance().packetReceived2();
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

//	if (vehicle.getId() == "0.5") {
//		std::cout << vehicle.getId() << "," << " lost a neighbor: " << addr << ", power: " << " number of neighbors: " << m_neighborList.size() << ", context: " << context << std::endl;
//	}
}

/**
 * Neighborhood discovery
 */
void FceApplication::NewNeighborFound(std::string context, Ptr<const Packet> packet, Mac48Address addr, double rxPwDbm) {
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
	if (i== myApp->m_neighborList.end ()) {
		// include the reception power
		myApp->m_neighborList[addr] = rxPwDbm;
	}
	else {
		i->second= rxPwDbm;
	}
//	if (vehicle.getId() == "0.5") {
//	packet->Print(cout) ;
//	std::cout << vehicle.getId() << "," <<  " discovered a neighbor: " << addr << ", power: " << " number of neighbors: " << m_neighborList.size() << ", packet.size: " << packet->GetSize() << ", packet.serializedSize: " << packet->GetSerializedSize()<< std::endl;
//	}
}

}
