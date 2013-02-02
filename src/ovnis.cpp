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
 * @file ovnis.cpp
 * @date Apr 21, 2010
 *
 * @author Yoann Pigné
 * @author Agata Grzybek
 */

#include <iostream>
#include <cmath>
#include <math.h>
#include <time.h>
#include <stdlib.h>

// ----- NS-3 related includes
#include "ns3/core-module.h"
#include "ns3/wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-net-device.h"
#include "ns3/mobility-helper.h"
#include "ns3/names.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/simulator.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/random-variable.h"

// ----- SUMO related includes
#include <traci-server/TraCIConstants.h>
#include "xml-sumo-conf-parser.h"

// ----- Ovnis application includes
#include "applications/ovnis-application.h"
#include "helper/ovnis-wifi-helper.h"
#include "devices/wifi/beaconing-adhoc-wifi-mac.h"
#include "ovnis.h"
#include "ovnis-constants.h"
#include "log.h"

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Ovnis");
NS_OBJECT_ENSURE_REGISTERED(Ovnis);

ns3::TypeId Ovnis::GetTypeId(void) {
	static TypeId tid = TypeId("ns3::Ovnis").SetParent<Object>().AddConstructor<Ovnis>().
			AddAttribute("OvnisApplication", "Application used in devices", StringValue("ns3::OvnisApplication"), MakeStringAccessor(&Ovnis::m_ovnis_application), MakeStringChecker()).
			AddAttribute("SumoConfig","The configuration file for running SUMO", StringValue("/Users/pigne/Documents/Projects/TrafficSimulation/SimpleTrafficApplication/kirchberg.sumo.cfg"),MakeStringAccessor(&Ovnis::sumoConfig), MakeStringChecker()).
			AddAttribute("SumoHost", "The host machine on which SUMO will run",StringValue(SUMO_HOST),MakeStringAccessor(&Ovnis::sumoHost), MakeStringChecker()).
			AddAttribute("OutputFolder", "Output folder name", StringValue("outputFolder"), MakeStringAccessor(&Ovnis::outputFolder), MakeStringChecker()).
			AddAttribute( "StartTime", "Start time in the simulation scale (in seconds)", IntegerValue(0), MakeIntegerAccessor(&Ovnis::startTime), MakeIntegerChecker<int>(0)).
			AddAttribute("StopTime", "Stop time in the simulation scale (in seconds)", IntegerValue(0), MakeIntegerAccessor(&Ovnis::stopTime), MakeIntegerChecker<int>(0)).
			AddAttribute( "CommunicationRange", "Communication range used to subdivide the simulation space (in meters)", DoubleValue(500.0), MakeDoubleAccessor(&Ovnis::communicationRange), MakeDoubleChecker<double>(0.0)).
			AddAttribute("StartSumo", "Does OVNIS have to start SUMO or not?", BooleanValue(), MakeBooleanAccessor(&Ovnis::startSumo), MakeBooleanChecker()).
			AddAttribute("SumoPath", "The system path where the SUMO executable is located", StringValue(SUMO_PATH), MakeStringAccessor(&Ovnis::sumoPath), MakeStringChecker());
	return tid;
}

Ovnis::Ovnis() :
		runningVehicles(vector<string>()), departedVehicles(vector<string>()), arrivedVehicles(vector<string>()) {
}

Ovnis::~Ovnis() {
	try {
		traci->Close();
	}
	catch (TraciException &e) {
		cerr << "Traci closing " << e.what();
	}
}

void Ovnis::DoDispose() {
	Log::getInstance().summariseSimulation("simulation");
	Object::DoDispose();
}

void Ovnis::DoStart(void) {
//	LogComponentEnable("YansWifiChannel", LOG_LEVEL_INFO);
//	LogComponentEnable("YansWifiPhy", LOG_LEVEL_INFO);
//	LogComponentEnable("OvnisWifiChannel", LOG_LEVEL_INFO);
//	LogComponentEnable("OvnisWifiPhy", LOG_LEVEL_INFO);

    is80211p = true;
    isLTE = false;
    isOvnisChannel = false;

    Log::getInstance().setOutputFolder(outputFolder);

	currentTime = 0;
    Names::Add("Ovnis", this);

	try {
		traci = CreateObject<SumoTraciConnection> ();
		traci->RunServer(sumoConfig, sumoHost, sumoPath, sumoPort, outputFolder);
		cout << "stoptime " << stopTime << endl;
		traci->SubscribeSimulation(startTime*1000, stopTime*1000);
		traci->NextSimStep(departedVehicles, arrivedVehicles);
		vector<double> bounds = traci->GetSimulationBoundaries();
		if (bounds.size() > 3) {
			boundaries[0] = bounds[2];
			boundaries[1] = bounds[3];
		}

		Names::Add("SumoTraci", traci);

		//application
		m_application_factory.SetTypeId(m_ovnis_application);

		// Initialize ns-3 devices
		InitializeNetwork();

		UpdateInOutVehicles();
		UpdateVehiclesPositions();
		StartApplications();
		start =  time(0);
		Log::getInstance().getStream("") << "starting simulation from " << startTime << " to " << stopTime << "..." << endl;
		Log::getInstance().getStream("simulation") << "start\t" << start << endl;
		Simulator::Schedule(Simulator::Now(), &Ovnis::TrafficSimulationStep, this);

		//string edgeId = "56640728#1";
		//EventId m_readEvent = Simulator::Schedule(Simulator::Now(), &Ovnis::ReadTravelTime, this, edgeId);
//		EventId m_closeEvent = Simulator::Schedule(Seconds(100c), &Ovnis::CloseRoad, this, edgeId);

		Log::getInstance().getStream("simulation") << "time \t running \t departed \t arrived \t nodes \t sent \t received \t dropped Switching/TX/RX \t distance \n";

		Object::DoStart();
	}
	catch (TraciException &e) {
		cerr << "TrafficSimulationStep " << e.what();
	}
	catch(exception & e) {
		cerr << "#Error while connecting: " << e.what();
	}
}

void Ovnis::CreateNetworkDevices(ns3::NodeContainer & node_container) {
	NetDeviceContainer devices;
	if (isOvnisChannel) {
		devices = wifi.Install(ovnisPhyHelper, mac, node_container);
	}
	else {
		devices = wifi.Install(phyHelper, mac, node_container);
	}
	InternetStackHelper stack;
	stack.Install(node_container);
	Ipv4InterfaceContainer wifiInterfaces;
	wifiInterfaces = address.Assign(devices);
}

// transmission power: 40 mW = 16.0206 dBm
double txPowerStart = 21.0206;
double txPowerEnd = 21.0206;
int txPowerLevels = 1;
double txGain = 0;
double rxGain = 0;
double energyDetectionThreshold = -96.0;
double ccaMode1Threshold = -99;

//string propagationLossModel = "ns3::LogDistancePropagationLossModel";
string propagationLossModel = "ns3::NakagamiPropagationLossModel";
string propagationDelayModel = "ns3::ConstantSpeedPropagationDelayModel";
WifiPhyStandard wifiPhyStandard = WIFI_PHY_STANDARD_80211_10MHZ;
string phyMode ("OfdmRate6MbpsBW10MHz");
string remoteStationManager = "ns3::ConstantRateWifiManager";
string macType = "ns3::BeaconingAdhocWifiMac";
//macType = "BeaconingAdhocWifiMacOriginal";
//string macType = "ns3::AdhocWifiMac";


void Ovnis::InitializeNetwork() {
	isOvnisChannel = true;
	is80211p = true;

	if (isOvnisChannel) {
		InitializeOvnisNetwork();
	}
	else {
		InitializeDefaultNetwork();
	}
}

void Ovnis::InitializeDefaultNetwork() {

	// phy layer
	phyHelper =  YansWifiPhyHelper::Default ();
	phyHelper.Set("TxPowerStart",DoubleValue(txPowerStart));	// 250-300 meter transmission range
	phyHelper.Set("TxPowerEnd",DoubleValue(txPowerEnd));      // 250-300 meter transmission range
	phyHelper.Set("TxPowerLevels",UintegerValue(txPowerLevels));
	phyHelper.Set("TxGain",DoubleValue(txGain));
	phyHelper.Set("RxGain",DoubleValue(rxGain));
	phyHelper.Set("EnergyDetectionThreshold", DoubleValue(energyDetectionThreshold));
	phyHelper.Set("CcaMode1Threshold", DoubleValue(ccaMode1Threshold));

	// channel
	channelHelper = YansWifiChannelHelper::Default ();
	channelHelper.AddPropagationLoss(propagationLossModel);
	channelHelper.SetPropagationDelay(propagationDelayModel);
	phyHelper.SetChannel(channelHelper.Create ());

	wifi = WifiHelper::Default();
	wifi.SetStandard(wifiPhyStandard);
	wifi.SetRemoteStationManager (remoteStationManager, "DataMode", StringValue(phyMode), "ControlMode",StringValue(phyMode));
	address.SetBase("10.0.0.0", "255.0.0.0");
	mac = NqosWifiMacHelper::Default();
	mac.SetType(macType);
	BeaconingAdhocWifiMac * mm = new BeaconingAdhocWifiMac();
//	BeaconingAdhocWifiMacOriginal * mm = new BeaconingAdhocWifiMacOriginal();
//	mac = NqosWifiMacHelper::Default();
//	mac.SetType("ns3::BeaconingAdhocWifiMac");

}

//	string propagationLossModel = "ns3::NakagamiPropagationLossModel";
//	string propagationDelayModel = "ns3::ConstantSpeedPropagationDelayModel";
//	WifiPhyStandard wifiPhyStandard = WIFI_PHY_STANDARD_80211_10MHZ;
//	string phyMode ("OfdmRate6MbpsBW10MHz");
//	string remoteStationManager = "ns3::ConstantRateWifiManager";
//	string macType = "ns3::AdhocWifiMac";

void Ovnis::InitializeOvnisNetwork() {

	if (is80211p) {
		ovnisPhyHelper = OvnisWifiPhyHelper::Default();
		ovnisPhyHelper.Set("TxPowerStart",DoubleValue(txPowerStart));	// 250-300 meter transmission range
		ovnisPhyHelper.Set("TxPowerEnd",DoubleValue(txPowerEnd));      // 250-300 meter transmission range
		ovnisPhyHelper.Set("TxPowerLevels",UintegerValue(txPowerLevels));
		ovnisPhyHelper.Set("TxGain",DoubleValue(txGain));
		ovnisPhyHelper.Set("RxGain",DoubleValue(rxGain));
		ovnisPhyHelper.Set("EnergyDetectionThreshold", DoubleValue(energyDetectionThreshold));
		ovnisPhyHelper.Set("CcaMode1Threshold", DoubleValue(ccaMode1Threshold));

//		ovnisChannel = CreateObject<OvnisWifiChannel>();
//		ObjectFactory factory1;
//		factory1.SetTypeId(propagationLossModel);
//		ovnisChannel->SetPropagationLossModel(factory1.Create<PropagationLossModel>());
//		ObjectFactory factory2;
//		factory2.SetTypeId(propagationDelayModel);
//		ovnisChannel->SetPropagationDelayModel(factory2.Create<PropagationDelayModel>());
//		ovnisChannel->updateArea(boundaries[0], boundaries[1], communicationRange);
//		ovnisPhyHelper.SetChannel(ovnisChannel);

		OvnisWifiChannelHelper ovnisChannelHelper = OvnisWifiChannelHelper::Default ();
		ovnisChannelHelper.AddPropagationLoss(propagationLossModel);
		ovnisChannelHelper.SetPropagationDelay(propagationDelayModel);
		ovnisChannel = ovnisChannelHelper.Create();
		ovnisChannel->updateArea(boundaries[0], boundaries[1], communicationRange);
		ovnisPhyHelper.SetChannel(ovnisChannel);

		wifi = WifiHelper::Default();
		wifi.SetStandard(wifiPhyStandard);
		wifi.SetRemoteStationManager (remoteStationManager, "DataMode", StringValue(phyMode), "ControlMode",StringValue(phyMode));
		address.SetBase("10.0.0.0", "255.0.0.0");
		mac = NqosWifiMacHelper::Default();
		mac.SetType(macType);
	}
	else if (isLTE) {
//		phy = OvnisWifiPhyHelper::Default();
//		ObjectFactory factory1;
//		channel = CreateObject<OvnisWifiChannel>();
////		factory1.SetTypeId("ns3::NakagamiPropagationLossModel");
////		channel->SetPropagationLossModel(factory1.Create<PropagationLossModel>());
////		ObjectFactory factory2;
////		factory2.SetTypeId("ns3::ConstantSpeedPropagationDelayModel");
////		channel->SetPropagationDelayModel(factory2.Create<PropagationDelayModel>());
//		channel->updateArea(boundaries[0], boundaries[1], communicationRange);
//		phy.SetChannel(channel);
//		phy.Set("TxPowerStart",DoubleValue(26.5));	// 250-300 meter transmission range
//		phy.Set("TxPowerEnd",DoubleValue(26.5));      // 250-300 meter transmission range
//		phy.Set("TxPowerLevels",UintegerValue(1));
//		phy.Set("TxGain",DoubleValue(0));
//		phy.Set("RxGain",DoubleValue(0));
//		phy.Set("EnergyDetectionThreshold", DoubleValue(-101.0));
//		wifi = WifiHelper::Default();
//		wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
//		wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"));
//		address.SetBase("10.0.0.0", "255.0.0.0");
//		mac = NqosWifiMacHelper::Default();
//		mac.SetType("ns3::AdhocWifiMac");
	}
}

void Ovnis::DestroyNetworkDevices(vector<string> to_destroy) {

	for (vector<string>::iterator i = to_destroy.begin(); i != to_destroy.end(); ++i) {
		Ptr<Node> n = Names::Find<Node>((*i));
		if (n != 0) {
			double now = Simulator::Now().GetSeconds();
			Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
			for (uint32_t j = 0; j < n->GetNDevices(); ++j) {
				int32_t ifIndex = ipv4->GetInterfaceForDevice(n->GetDevice(j));
				Ipv4InterfaceAddress address = ipv4->GetAddress(ifIndex, 0);
				ipv4->RemoveAddress(ifIndex, 0);
				ipv4->SetDown(ifIndex);
			}
			for (uint32_t j = 0; j < n->GetNApplications(); ++j) {
				ns3::Time stopTime = Simulator::Now();
				if (isOvnisChannel) {
					Ptr<OvnisApplication> app = DynamicCast<OvnisApplication>(n->GetApplication(j));
					app->SetStopTime(stopTime);
				}
				else {
					n->GetApplication(j)->SetStopTime(stopTime);
				}
			}
			if (isOvnisChannel) {
				Ptr<NetDevice> d = n->GetDevice(0);
				Ptr<WifiNetDevice> wd = DynamicCast<WifiNetDevice>(d);
				Ptr<WifiPhy> wp = wd->GetPhy();
				Ptr<OvnisWifiPhy> ywp = DynamicCast<OvnisWifiPhy>(wp);
				Ptr<WifiMac> wm = wd->GetMac();
				Ptr<BeaconingAdhocWifiMac> bwm = DynamicCast<BeaconingAdhocWifiMac>(wm);
				bwm->StopBeaconing();
				ovnisChannel->Remove(ywp);
			}
		}
	}
}

void Ovnis::UpdateInOutVehicles() {
	NS_LOG_FUNCTION_NOARGS();
	// remove the eventually removed vehicles while added in the inserted list (especially a the beginning)
	vector<string>::iterator i = arrivedVehicles.begin();
	while (i != arrivedVehicles.end()) {
		vector<string>::iterator it;
		it = std::find(departedVehicles.begin(), departedVehicles.end(), (*i));
		if (it != departedVehicles.end()) {
			departedVehicles.erase(it);
			i = arrivedVehicles.erase(i);
		}
		else {
			++i;
		}
	}

	// create new nodes and start applications.
	NodeContainer node_container;
	node_container.Create(departedVehicles.size());
	int j = 0;
	for (vector<string>::iterator i = departedVehicles.begin(); i != departedVehicles.end(); ++i) {
		Names::Add("Nodes", (*i), node_container.Get(j));
		++j;
	}
	MobilityHelper mobility;
	mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
	mobility.Install(node_container);

	//--  Network
	CreateNetworkDevices(node_container);

	// -------- set 'down' interfaces of terminated vehicles and remove nodes from the channel.
	DestroyNetworkDevices(arrivedVehicles);

	// -------- update the set of running vehicles
	for (std::vector<std::string>::iterator i = arrivedVehicles.begin(); i != arrivedVehicles.end(); ++i) {
		vector<string>::iterator it;
		it = std::find(runningVehicles.begin(), runningVehicles.end(), (*i));
		if (it != runningVehicles.end()) {
			runningVehicles.erase(it);
		}
	}

	runningVehicles.insert(runningVehicles.end(), departedVehicles.begin(), departedVehicles.end());
	arrivedVehicles.clear();
}

void Ovnis::StartApplications() {
	int j = 0;
	for (vector<string>::iterator i = departedVehicles.begin(); i != departedVehicles.end(); ++i) {
		Ptr<Node> node = Names::Find<Node>((*i));
		if (node!=0) {
			Ptr<Application> app = m_application_factory.Create<Application>();
			node->AddApplication(app);
			app->SetStartTime(Seconds(0));
		}
	}
	departedVehicles.clear();
}

void Ovnis::UpdateVehiclesPositions() {
	NS_LOG_FUNCTION_NOARGS();
	for (vector<string>::iterator i = runningVehicles.begin(); i != runningVehicles.end(); ++i) {
		Ptr<Node> node = Names::Find<Node>((*i));
		if (node!=0) {
			Ptr<Object> object = node;
			Ptr<ConstantVelocityMobilityModel> model = object->GetObject<ConstantVelocityMobilityModel>();
			ovnis::Position2D newPos = traci->GetVehiclePosition(*i);
			if (newPos.x > 0 && newPos.y > 0)
			{
				double newSpeed = traci->GetVehicleSpeed((*i));
				double newAngle = traci->GetVehicleAngle(*i);
				double PI = 3.14159265;
				Vector velocity(newSpeed * cos((newAngle + 90) * PI / 180.0), newSpeed * sin((newAngle - 90) * PI / 180.0), 0.0);
				Vector position(newPos.x, newPos.y, 0.0);
				model->SetPosition(position);
				model->SetVelocity(velocity); // Note : set the velocity AFTER the position . Else velocity is reset
				Ptr<NetDevice> d = node->GetDevice(0);
				Ptr<WifiNetDevice> wd = DynamicCast<WifiNetDevice>(d);
				Ptr<WifiPhy> wp = wd->GetPhy();
				if (isOvnisChannel) {
					Ptr<OvnisWifiPhy> ywp = DynamicCast<OvnisWifiPhy>(wp);
					ovnisChannel->updatePhy(ywp);
				}
			}
		}
	}
}

void Ovnis::TrafficSimulationStep() {

	try {
		currentTime = traci->GetCurrentTime();
		ovnis::Log::getInstance().logInTime(currentTime);
		ovnis::Log::getInstance().logIn(VEHICLES_DEPARTURED, departedVehicles.size(), currentTime);
		ovnis::Log::getInstance().logIn(VEHICLES_ARRIVED, arrivedVehicles.size(), currentTime);

		Log::getInstance().getStream("simulation") << currentTime/1000 << " \t " << runningVehicles.size() << " \t " << departedVehicles.size() << " \t " << arrivedVehicles.size() << " \t " << ns3::NodeList::GetNNodes() << " \t "
				<< Log::getInstance().getSentPackets() << " \t " << Log::getInstance().getReceivedPackets() << " \t"
				<< Log::getInstance().getDroppedPackets(ns3::WifiPhy::SWITCHING) << ", "
				<< Log::getInstance().getDroppedPackets(ns3::WifiPhy::TX) << ", "
				<< Log::getInstance().getDroppedPackets(ns3::WifiPhy::RX) << " \t "
				<< Log::getInstance().getAvgDistance() << endl;

		// update running vehicles
		UpdateInOutVehicles();
		UpdateVehiclesPositions();
		StartApplications();

		// 56640728#2
//		double travelTime = traci->GetEdgeTravelTime("56640728#2");
//		cout << travelTime << endl;

		// real loop until stop time
		// this is the second step (first is immediately called after the subscription
		// in the first step, departed and arrived vehicles are aggregated from the beginning of running
		traci->NextSimStep(departedVehicles, arrivedVehicles);

		if (currentTime < stopTime*1000) {
			Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &Ovnis::TrafficSimulationStep, this);
		}
		else {
			cout << "stopping simulation at currentTime " << currentTime << " (stopTime: " << stopTime << ")" << endl;
			DestroyNetworkDevices(runningVehicles);
			time_t stop = time(0);
			Log::getInstance().getStream("simulation") << "stop\t" << stop << endl;
			Log::getInstance().getStream("simulation") << "duration\t" << (stop-start) << endl;
		}
	}
	catch (TraciException &e) {
		cerr << "TrafficSimulationStep " << e.what();
	}
}

void Ovnis::CloseRoad(string edgeId) {
	// set max speed on each lane -> 0
	vector<string> lanes;
	lanes.push_back(edgeId+"_0");
	lanes.push_back(edgeId+"_1");
	for (vector<string>::iterator it = lanes.begin(); it != lanes.end(); ++it) {
		cout << "try closing lane " << *it << "..." << endl;
		traci->CloseLane(*it);
	}
	EventId m_readEvent = Simulator::Schedule(Simulator::Now()+Seconds(2), &Ovnis::ReadTravelTime, this, edgeId);
}

void Ovnis::ReadTravelTime(string edgeId) {
	double travelTime = traci->GetEdgeTravelTime(edgeId);
	cout << "edge " << edgeId << ", currentTravelTime = " << travelTime << endl;

//	double travelTime2 = traci->GetEdgeGlobalTravelTime(edgeId);
//	cout << "edge " << edgeId << ", globalTravelTime= " << travelTime2 << endl;

}
}
