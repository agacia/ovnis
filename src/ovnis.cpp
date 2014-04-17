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
#include <cstdlib>
#include <exception>
#include <signal.h>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>

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
#include "devices/wifi/ovnis-adhoc-wifi-mac.h"
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
//			AddAttribute("OutputFolder", "Output folder name", StringValue("outputFolder"), MakeStringAccessor(&Ovnis::outputFolder), MakeStringChecker()).
			AddAttribute("ScenarioFolder", "Scenario folder path", StringValue("scenarioFolder"), MakeStringAccessor(&Ovnis::scenarioFolder), MakeStringChecker()).
			AddAttribute( "StartTime", "Start time in the simulation scale (in seconds)", IntegerValue(0), MakeIntegerAccessor(&Ovnis::startTime), MakeIntegerChecker<int>(0)).
			AddAttribute("StopTime", "Stop time in the simulation scale (in seconds)", IntegerValue(0), MakeIntegerAccessor(&Ovnis::stopTime), MakeIntegerChecker<int>(0)).
			AddAttribute( "CommunicationRange", "Communication range used to subdivide the simulation space (in meters)", DoubleValue(MAX_COMMUNICATION_RANGE), MakeDoubleAccessor(&Ovnis::communicationRange), MakeDoubleChecker<double>(0.0)).
			AddAttribute("StartSumo", "Does OVNIS have to start SUMO or not?", BooleanValue(), MakeBooleanAccessor(&Ovnis::startSumo), MakeBooleanChecker()).
			AddAttribute("SumoPath", "The system path where the SUMO executable is located", StringValue(SUMO_PATH), MakeStringAccessor(&Ovnis::sumoPath), MakeStringChecker());

	return tid;
}

Ovnis::Ovnis() :
		runningVehicles(vector<string>()), departedVehicles(vector<string>()), arrivedVehicles(vector<string>()) {
}

Ovnis::~Ovnis() {
	cout << "Closing " << endl;
	try {
		traci->Close();
//		communitystream.close();
		pclose(fp_com);
	}
	catch (TraciException &e) {
		cerr << "Traci closing " << e.what();
	}
}

void Ovnis::SetOvnisParams(std::map <string,string> params) {
	_params = params;
}

void Ovnis::SetApplicationParams(std::map <string,string> params) {
	_applicationParams = params;
}

void Ovnis::DoDispose() {
	cout << " DoDispose " << endl;
	Object::DoDispose();
}

void Ovnis::DoInitialize(void) {
//	LogComponentEnable("YansWifiChannel", LOG_LEVEL_INFO);
//	LogComponentEnable("YansWifiPhy", LOG_LEVEL_INFO);
//	LogComponentEnable("OvnisWifiChannel", LOG_LEVEL_INFO);
//	LogComponentEnable("OvnisWifiPhy", LOG_LEVEL_INFO);

    is80211p = true;
    isOvnisChannel = false;
	currentTime = 0;
    Names::Add("Ovnis", this);

    Log::getInstance().setOutputFolder(scenarioFolder + "ovnisOutput");
    map<string, string>::iterator it = _params.find("outputFolder");
    if (it != _params.end()) {
    	cout << "Setting output folder " << it->second << endl;
    	Log::getInstance().setOutputFolder(it->second);
    }

   InitializeSumo();

   //InitializeCrowdz();

   //application
	m_application_factory.SetTypeId(m_ovnis_application);
	// Initialize ns-3 devices
	InitializeNetwork();
	UpdateInOutVehicles();
	UpdateVehiclesPositions();
	StartApplications();
	start =  time(0);
	Log::getInstance().getStream("") << "Starting simulation from " << startTime << " to " << stopTime << "..." << endl;
	Log::getInstance().getStream("simulation") << "start\t" << start << endl;
	Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &Ovnis::TrafficSimulationStep, this);
	Log::getInstance().getStream("simulation") << "time \t running \t connected \t departed \t arrived \t nodes \t sent \t received \t dropped Switching/TX/RX \t distance \n";

	Object::DoInitialize();
}

bool Ovnis::ReadCommunities(int step) {
	step /= 1000;
	string line;
	step -= 2; // adjust
	bool readSteps = false;
	bool waitForSteps = true;
	cout << "ReadCommunities reading communities for step " << step << ", posstream " << posstream << " communitystream.tellg() " << communitystream.tellg() << endl;
	if (step > 0) {
		while (!communitystream.is_open()) {
			communitystream.open((scenarioFolder +  "/communities.csv").c_str());
		}
		if (communitystream.is_open()) {
			if (posstream != -1) {
				communitystream.seekg(posstream);
			}
//			cout << "communitystream open  tellg " << communitystream.tellg() << ", posstream " << posstream << endl;
			while (getline (communitystream,line) || !readSteps) {
				vector<string> tokens = tokenizeString(line, "\t");
				if (tokens.size() == 17) {
					// correct line
					if (tokens[0] == "step") {
						// skip header
					}
					else {
						int linestep = atoi(tokens[0].c_str());

						if (linestep < step) {
							// skip earlier steps
							posstream = communitystream.tellg();
						}
						else if (linestep == step) {
							cout << "read line: " << line << " " << communitystream.tellg() << ", posstream " << posstream << endl;
							posstream = communitystream.tellg();
						}
						else {
							readSteps = true;
							break;
						}
					}
				}
				// end of file
//				cout << "end of file?  tellg " << communitystream.tellg() << ", posstream " << posstream << endl;
				if (communitystream.tellg() == -1 && readSteps == false) {
//					cout << "restart " << communitystream.tellg() << ", posstream " << posstream << endl;
					communitystream.close();
					communitystream.open((scenarioFolder +  "/communities.csv").c_str());
					if (posstream == -1) {
						communitystream.seekg(0);
					} else {
						communitystream.seekg(posstream);
					}

				}
			}
			cout << "stopped reading line " << communitystream.tellg() << ", posstream " << posstream << endl;
			communitystream.close();
		}
	}
	return readSteps;
}

void Ovnis::ReadCommunities() {
	string line;
	while (!communitystream.is_open()) {
		communitystream.open((scenarioFolder +  "/communities.csv").c_str());
	}
	if (communitystream.is_open()) {
		bool nextStep = true;
		cout << "ReadCommunities reading communities for step " << currentTime << ", posstream " << posstream << endl;
		while (true) {
			cout << "checking tellg " << communitystream.tellg() << " posstream " << posstream << endl;
			if (getline (communitystream,line) ) {
				vector<string> tokens = tokenizeString(line, "\t");

				if (tokens[0] == "step") {
					// skip read headers
				} else {
					int linestep = atoi(tokens[0].c_str());
					if (tokens.size() == 11) {
						cout << "good line line with step " << linestep << ": " << line << ", tellg " << communitystream.tellg() << endl;
						posstream = communitystream.tellg();
					}
				}
			}
			else {
				// return to the previous position
				communitystream.seekg(communitystream.beg);
			}
		}
	}
	else {
		cout << "Unable to open file";
	}
}


vector<string> Ovnis::tokenizeString(string s, string delim) {
	std::istringstream buf(s);
	std::istream_iterator<std::string> beg(buf), end;
	std::vector<std::string> tokens(beg, end); // done!
	return tokens;
}

void Ovnis::InitializeCrowdz() {
	cout << "InitializeCrowdz" << endl;
	posstream = -1;
	com_pid = fork();
	if (com_pid == -1) {
		std::cerr << "Uh-Oh! fork() failed.\n";
		exit(1);
	}
	else if (com_pid == 0) {
		char buff[512];
		ofstream out;
		out.open((scenarioFolder+"/crowdz_output.log").c_str());
		out << "Output log file from crowdz's execution.";
		string jar = "/Users/agatagrzybek/workspace/crowds/crowdz-trafficEQ.jar";
		string inputfile = "/Users/agatagrzybek/workspace/ovnis/scenarios/Kirchberg/vanet.dgs";

		string algorithm="MobileSandSharc";
		string congestion="CongestionMeasure";
		int congestion_threshold=5;
		int history_length=10;
		string speedType = "timemean";

		std::stringstream str;
		str << "java -Xmx4096m -jar " << jar << " --inputFile \"" << inputfile << "\" --outputDir " << scenarioFolder+"/" << " --startStep " << startTime <<
				" --endStep " << stopTime << " --algorithm " << algorithm << " --congestion " << congestion << " --congestionSpeedThreshold " << congestion_threshold << " --speedHistoryLength " << history_length <<
				" > " <<  scenarioFolder << "/crowds2log.txt";
		string call = str.str();
		cout << "call " << call << endl;
		system((call).c_str());
		exit(0);
	}
	else {
		// start reading community stream
		communitystream.seekg(communitystream.beg);
		posstream = communitystream.tellg();
		cout << "Initializing com stream , posstream " << posstream  << endl;
	}
}

void Ovnis::InitializeSumo() {
	 try {
		traci = CreateObject<SumoTraciConnection> ();
		traci->RunServer(sumoConfig, sumoHost, sumoPath, sumoPort, scenarioFolder);
		traci->SubscribeSimulation(startTime*SIMULATION_TIME_UNIT, stopTime*SIMULATION_TIME_UNIT);
		traci->NextSimStep(departedVehicles, arrivedVehicles);
		vector<double> bounds = traci->GetSimulationBoundaries();
		if (bounds.size() > 3) {
			boundaries[0] = bounds[2];
			boundaries[1] = bounds[3];
		}
		Names::Add("SumoTraci", traci);
		sumo_pid = traci->pid;
	}
	catch (TraciException &e) {
		cerr << "TrafficSimulationStep " << e.what();
	}
	catch(exception & e) {
		cerr << "#Error while connecting: " << e.what();
	}
	cout << "after InitializeSumo" << endl;
}

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
	phyHelper.Set("TxPowerStart",DoubleValue(TX_POWER_START));
	phyHelper.Set("TxPowerEnd",DoubleValue(TX_POWER_END));
	phyHelper.Set("TxPowerLevels",UintegerValue(TX_POWER_LEVELS));
	phyHelper.Set("TxGain",DoubleValue(TX_GAIN));
	phyHelper.Set("RxGain",DoubleValue(RX_GAIN));
	phyHelper.Set("EnergyDetectionThreshold", DoubleValue((double)ENERGY_DETECTION_THRESHOLD));
	phyHelper.Set("CcaMode1Threshold", DoubleValue(CCA_MODEL_THRESHOLD));
	// channel
	channelHelper = YansWifiChannelHelper::Default ();
	channelHelper.AddPropagationLoss(PROPAGATION_LOSS_MODEL);
	channelHelper.SetPropagationDelay(PROPAGATION_DELAY_MODEL);
	phyHelper.SetChannel(channelHelper.Create ());
	wifi = WifiHelper::Default();
	wifi.SetStandard(WIFI_PHY_STANDARD);
	wifi.SetRemoteStationManager (REMOTE_STATION_MANAGER, "DataMode", StringValue(PHY_MODE), "ControlMode", StringValue(PHY_MODE));
	address.SetBase(BASE_NETWORK_ADDRESS, NETWORK_MASK); // initial address it defaults to "0.0.0.1"
	mac = NqosWifiMacHelper::Default();
	mac.SetType(MAC_TYPE);
	BeaconingAdhocWifiMac * bm = new BeaconingAdhocWifiMac();
}

void Ovnis::InitializeOvnisNetwork() {
	if (is80211p) {
		ovnisPhyHelper = OvnisWifiPhyHelper::Default();
		ovnisPhyHelper.Set("TxPowerStart",DoubleValue(TX_POWER_START));
		ovnisPhyHelper.Set("TxPowerEnd",DoubleValue(TX_POWER_END));
		ovnisPhyHelper.Set("TxPowerLevels",UintegerValue(TX_POWER_LEVELS));
		ovnisPhyHelper.Set("TxGain",DoubleValue(TX_GAIN));
		ovnisPhyHelper.Set("RxGain",DoubleValue(RX_GAIN));
		ovnisPhyHelper.Set("EnergyDetectionThreshold", DoubleValue(ENERGY_DETECTION_THRESHOLD));
		ovnisPhyHelper.Set("CcaMode1Threshold", DoubleValue(CCA_MODEL_THRESHOLD));

		OvnisWifiChannelHelper ovnisChannelHelper = OvnisWifiChannelHelper::Default ();
		ovnisChannelHelper.AddPropagationLoss(PROPAGATION_LOSS_MODEL);
		ovnisChannelHelper.SetPropagationDelay(PROPAGATION_DELAY_MODEL);
		ovnisChannel = ovnisChannelHelper.Create();
		ovnisChannel->updateArea(boundaries[0], boundaries[1], communicationRange);
		ovnisPhyHelper.SetChannel(ovnisChannel);

		wifi = WifiHelper::Default();
		wifi.SetStandard(WIFI_PHY_STANDARD);
		wifi.SetRemoteStationManager (REMOTE_STATION_MANAGER, "DataMode", StringValue(PHY_MODE), "ControlMode", StringValue(PHY_MODE));
		address.SetBase(BASE_NETWORK_ADDRESS, NETWORK_MASK); // initial address it defaults to "0.0.0.1"
		mac = NqosWifiMacHelper::Default();
		mac.SetType(MAC_TYPE);
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
					Ptr<Application> app = n->GetApplication(j);
					try {
						Ptr<OvnisApplication> ovnisApp = DynamicCast<OvnisApplication>(app);
						for (vector<string>::iterator k = connectedVehicles.begin(); k < connectedVehicles.end(); ++k) {
							if (*i == *k) {
								ovnisApp->SetStopTime(stopTime);
								break;
							}
						}
					}
					catch (exception & e) {
					}
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

void Ovnis::writeStep(ostream& out) {
	int step = traci->GetCurrentTime();
	if (step == 1000) {
		out << "DGS004" << endl;
		out << "vanet 0 0" << endl;
	}
//	cout << "st " << ((step)/1000) << endl;
	out << "st " << ((step)/1000) << endl;
}

void Ovnis::writeNewNode(ostream& out, string nodeId) {
	Ptr<Node> n = Names::Find<Node>(nodeId);
	if (n!=0 && isOvnisChannel) {
		for (uint32_t j = 0; j < n->GetNApplications(); ++j) {
			ns3::Time curtime = Simulator::Now();
			Ptr<Application> app = n->GetApplication(j);
			try {
				Ptr<OvnisApplication> ovnisApp = DynamicCast<OvnisApplication>(app);
				Vehicle* vehicle = ovnisApp->getData();
				vehicle->requestCurrentSpeed();
				vehicle->requestCurrentPosition();
				vehicle->requestCurrentAngle();
				out << "an " << vehicle->getId() << " x=" << vehicle->getCurrentPosition().x << " y=" << vehicle->getCurrentPosition().y << " vehicleSlope=" << 0 << " vehicleAngle=" << vehicle->getCurrentAngle() << " vehiclePos=" << 0 << " vehicleSpeed=" << vehicle->getCurrentSpeed() << " vehicleLane=" << (vehicle->getItinerary()).getCurrentEdge().getId() << endl; // an 0.0 x=16248.69 y=8138.77 vehicleSlope=0.0 vehicleAngle=-47.1 vehiclePos=7.6 vehicleSpeed=19.44 vehicleLane="56640729#0_0"
			}
			catch (exception & e) {
			}
		}
	}
}

void Ovnis::writeChangeNode(ostream& out, string nodeId) {
	Ptr<Node> n = Names::Find<Node>(nodeId);
	if (n!=0 && isOvnisChannel) {
		for (uint32_t j = 0; j < n->GetNApplications(); ++j) {
			ns3::Time curtime = Simulator::Now();
			Ptr<Application> app = n->GetApplication(j);
			try {
				Ptr<OvnisApplication> ovnisApp = DynamicCast<OvnisApplication>(app);
				Vehicle* vehicle = ovnisApp->getData();
				vehicle->requestCurrentAngle();
				vehicle->requestCurrentSpeed();
				vehicle->requestCurrentPosition();
				out << "cn " << vehicle->getId() << " x=" << vehicle->getCurrentPosition().x << " y=" << vehicle->getCurrentPosition().y << " vehicleSlope=" << 0 << " vehicleAngle=" << vehicle->getCurrentAngle() << " vehiclePos=" << 0 << " vehicleSpeed=" << vehicle->getCurrentSpeed() << " vehicleLane=" << (vehicle->getItinerary()).getCurrentEdge().getId() << endl; // an 0.0 x=16248.69 y=8138.77 vehicleSlope=0.0 vehicleAngle=-47.1 vehiclePos=7.6 vehicleSpeed=19.44 vehicleLane="56640729#0_0"
			}
			catch (exception & e) {
			}
		}
	}
}

void Ovnis::writeDeleteNode(ostream& out, string nodeId) {
	Ptr<Node> n = Names::Find<Node>(nodeId);
	if (n!=0 && isOvnisChannel) {
		for (uint32_t j = 0; j < n->GetNApplications(); ++j) {
			ns3::Time curtime = Simulator::Now();
			Ptr<Application> app = n->GetApplication(j);
			try {
				Ptr<OvnisApplication> ovnisApp = DynamicCast<OvnisApplication>(app);
				Vehicle* vehicle = ovnisApp->getData();
				out << "dn " << vehicle->getId() << endl;
			}
			catch (exception & e) {
			}
		}
	}
}

void Ovnis::writeEdges(ostream& out) {
	runningVehicles.insert(runningVehicles.end(), departedVehicles.begin(), departedVehicles.end());

	for (vector<string>::iterator i = runningVehicles.begin(); i != runningVehicles.end(); ++i) {
		Ptr<Node> n = Names::Find<Node>(*i);
		if (n!=0 && isOvnisChannel) {
			for (uint32_t j = 0; j < n->GetNApplications(); ++j) {
				ns3::Time curtime = Simulator::Now();
				Ptr<Application> app = n->GetApplication(j);
				try {
					Ptr<OvnisApplication> ovnisApp = DynamicCast<OvnisApplication>(app);
					Vehicle* vehicle = ovnisApp->getData();
					string n1 = vehicle->getId();
					for (MacAddrMapIterator i =vehicle->m_neighborListDiscovered.begin(); i != vehicle->m_neighborListDiscovered.end (); ++i) {
						string n2 = macList[i->first];
//						cout << "ae ? " << n1 << " " << n2 << " addedEdges " << addedEdges.size() << endl;
						writeEdge("ae", n1, n2, addedEdges, out);
					}
					for (MacAddrMapIterator i =vehicle->m_neighborListDiscovered.begin(); i != vehicle->m_neighborListDiscovered.end (); ++i) {
						string n2 = macList[i->first];
//						writeEdge("de", n1, n2, removedEdges, out);
					}
					vehicle->m_neighborListDiscovered = MacAddrMap();
					vehicle->m_neighborListLost = MacAddrMap();
				}
				catch (exception & e) {
				}
			}
		}
	}
}

void Ovnis::writeEdge(string cmd, string n1, string n2, map<string, string>& addedEdges, ostream& out) {
	string edge1 = n1+"-"+n2;
	string edge2 = n2+"-"+n1;
//	cout << "Checkig edge " << edge1 << " and " << edge2 << " in added edges " << addedEdges.size() << endl;
	map<string,string>::iterator it1 = addedEdges.find(edge1);
	// an edge has not yet been added, add both edges
	if (it1 == addedEdges.end()) {
		addedEdges[edge1] = edge1;
		addedEdges[edge2] = edge2;
		out << "ae \"" << edge1 << "\" " << n1 << " " << n2 << endl; //ae "0.1-1.0" 0.1 1.0 weight=139.307967109
//		cout << "Adding do added edges" << edge1 << " and " << edge2 <<  " new added edges " << addedEdges.size() << endl;
	}
}

//void Ovnis::writeDeleteEdges(ostream& out, string nodeId) {
//	Ptr<Node> n = Names::Find<Node>(nodeId);
//	if (n!=0 && isOvnisChannel) {
//		for (uint32_t j = 0; j < n->GetNApplications(); ++j) {
//			ns3::Time curtime = Simulator::Now();
//			Ptr<Application> app = n->GetApplication(j);
//			try {
//				Ptr<OvnisApplication> ovnisApp = DynamicCast<OvnisApplication>(app);
////				Vehicle vehicle = ovnisApp->getData();
////				out << "dn " << vehicle.getId() << " " << nodeId << endl;
//			}
//			catch (exception & e) {
//			}
//		}
//	}
//}

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

//	runningVehicles.insert(runningVehicles.end(), departedVehicles.begin(), departedVehicles.end());
//	arrivedVehicles.clear();
}

void Ovnis::StartApplications() {
	newConnectedVehiclesCount = 0;
	for (vector<string>::iterator i = departedVehicles.begin(); i != departedVehicles.end(); ++i) {
		Ptr<Node> node = Names::Find<Node>((*i));
		bool isVANET = true;
		map<string,string>::iterator it = _params.find("penedtrationRate");
		if (it != _params.end()) {
			double r = (double)(rand()%RAND_MAX)/(double)RAND_MAX;
			double penetrationRate = atof((it->second).c_str());
			cout << "Penetration rate is set: " << penetrationRate << endl;
			isVANET = penetrationRate > 0 && r <= penetrationRate;
		}
		if (node!=0 && isOvnisChannel && isVANET) {
			connectedVehicles.insert(connectedVehicles.end(), i, i+1);
			Ptr<Application> app = m_application_factory.Create<Application>();
			node->AddApplication(app);
			app->SetStartTime(Seconds(0));
			Ptr<OvnisApplication> ovnisApp = DynamicCast<OvnisApplication>(app);
			if (ovnisApp==0) {
				cout << "Null pointer on fceApplication" << endl;
			}
			else {
				ovnisApp->SetParams(_applicationParams);
			}
			++newConnectedVehiclesCount;
		}
	}
//	departedVehicles.clear();
}

void Ovnis::updateNodePosition(string nodeId) {
	Ptr<Node> node = Names::Find<Node>(nodeId);
	if (node!=0) {
		Ptr<Object> object = node;
		Ptr<ConstantVelocityMobilityModel> model = object->GetObject<ConstantVelocityMobilityModel>();
		ovnis::Position2D newPos = traci->GetVehiclePosition(nodeId);
		if (newPos.x > 0 && newPos.y > 0)
		{
			double newSpeed = traci->GetVehicleSpeed(nodeId);
			double newAngle = traci->GetVehicleAngle(nodeId);
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

void Ovnis::UpdateVehiclesPositions() {
	NS_LOG_FUNCTION_NOARGS();
	for (vector<string>::iterator i = runningVehicles.begin(); i != runningVehicles.end(); ++i) {
		updateNodePosition(*i);
	}
	for (vector<string>::iterator i = departedVehicles.begin(); i != departedVehicles.end(); ++i) {
		updateNodePosition(*i);
	}
}

void Ovnis::MapMacVehId() {
	for ( std::vector< Ptr<Node> >::const_iterator it = ns3::NodeList::Begin(); it != ns3::NodeList::End(); it ++) {
		Ptr<WifiNetDevice> dev = DynamicCast<WifiNetDevice> ((*it)->GetDevice(0));
		Mac48Address addr = dev->GetMac()->GetAddress();
		string vehId = Names::FindName((*it));
//		cout << "node " << (*it)->GetId() << " addr: " << addr << " vehId " << vehId << endl;
		macList[addr] = vehId;
	}
}

void Ovnis::TrafficSimulationStep() {

	try {
		currentTime = traci->GetCurrentTime();
		ovnis::Log::getInstance().logIn(VEHICLES_DEPARTURED, departedVehicles.size(), currentTime);
		ovnis::Log::getInstance().logIn(VEHICLES_CONNECTED, newConnectedVehiclesCount, currentTime);
		ovnis::Log::getInstance().logIn(VEHICLES_ARRIVED, arrivedVehicles.size(), currentTime);
		Log::getInstance().getStream("simulation") << currentTime/1000 << " \t " << runningVehicles.size() << " \t " << connectedVehicles.size() << " \t " << departedVehicles.size() << " \t " << arrivedVehicles.size() << " \t " << ns3::NodeList::GetNNodes() << " \t "
				<< Log::getInstance().getSentPackets() << " \t " << Log::getInstance().getReceivedPackets() << " \t"
				<< Log::getInstance().getDroppedPackets(ns3::WifiPhy::SWITCHING) << ", "
				<< Log::getInstance().getDroppedPackets(ns3::WifiPhy::TX) << ", "
				<< Log::getInstance().getDroppedPackets(ns3::WifiPhy::RX) << " \t "
				<< Log::getInstance().getAvgDistance() << endl;

		MapMacVehId();

//		DetectCommunities();
//		ReadCommunities(currentTime);
//		TIS::getInstance().communities = currentTime;
//		cout << "step\t" << currentTime/1000 << "\trunningVehicles " << runningVehicles.size() << " \tdepartedVehicles " << departedVehicles.size() << " \tarrivedVehicles " << arrivedVehicles.size() << endl;

		runningVehicles.insert(runningVehicles.end(), departedVehicles.begin(), departedVehicles.end());
		cleanTemporaryArrays();

		traci->NextSimStep(departedVehicles, arrivedVehicles);

//		MapMacVehId();
//		DetectCommunities();
		// update running vehicles
		UpdateInOutVehicles();
		UpdateVehiclesPositions();
		StartApplications();

		// real loop until stop time
		// this is the second step (first is immediately called after the subscription
		// in the first step, departed and arrived vehicles are aggregated from the beginning of running

		if (currentTime < stopTime*SIMULATION_TIME_UNIT) {
			Simulator::Schedule(Seconds(SIMULATION_STEP_INTERVAL), &Ovnis::TrafficSimulationStep, this);
		}
		else {
			pclose(fp_com);
			cout << "kill java " << kill(com_pid, SIGTERM) << endl;
			cout << " kill sumo " << kill(sumo_pid, SIGKILL) << endl;
			DestroyNetworkDevices(runningVehicles);
			Log::getInstance().summariseSimulation("simulation");
			time_t stop = time(0);
			cout << "Finished! Steps: " << currentTime/1000 << ", Simulation time" << (double)(time(0) - start) << " s. " << endl;
			Log::getInstance().getStream("simulation") << "stop\t" << stop << endl;
			Log::getInstance().getStream("simulation") << "duration\t" << (stop-start) << endl;
			Log::getInstance().getStream("simulation") << "needed probabilistic (congestion detected)\t" << Log::getInstance().needProbabilistic << endl;
			Log::getInstance().getStream("simulation") << "could cheat (the sugegsted trip != the shortest)\t" << Log::getInstance().cheaters << endl;
			Log::getInstance().getStream("simulation") << "cheaters (actually cheatet)\t" << Log::getInstance().cheaters << endl;
		}
	}
	catch (TraciException &e) {
//		cerr << "TrafficSimulationStep " << e.what();
	}
}

void Ovnis::DetectCommunities() {
	ostream & communityStream = Log::getInstance().getStream("vanet.dgs");
	communityStream.setf( std::ios::fixed, std:: ios::floatfield );
	communityStream.precision(4);
	writeStep(communityStream);
	for (vector<string>::iterator i = departedVehicles.begin(); i != departedVehicles.end(); ++i) {
		writeNewNode(communityStream, *i);
	}
	for (vector<string>::iterator i = runningVehicles.begin(); i != runningVehicles.end(); ++i) {
		writeChangeNode(communityStream, *i);
	}
	for (vector<string>::iterator i = arrivedVehicles.begin(); i != arrivedVehicles.end(); ++i) {
		writeDeleteNode(communityStream, *i);
	}
	writeEdges(communityStream);
	communityStream << endl;
//	communityStream.flush();
	cout << "Wrote dgs at step " << traci->GetCurrentTime() << endl;
}

void Ovnis::cleanTemporaryArrays() {
	arrivedVehicles.clear();
	departedVehicles.clear();
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

//void Ovnis::CreateStaticNodes () {
//	NS_LOG_FUNCTION (this);
//	nodes.Create (staticNodes);
//	// Create two sta'c nodes, with fixed coordinates.
//	MobilityHelper mobility;
//	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
//	positionAlloc-­>Add (Vector (2010.0, 1500.0, 0.0)); // Static Node 1 (x:2010, y:1500)
//	positionAlloc-­>Add (Vector (10.0, 500.0, 0.0)); // Static Node 2 (x:10, y:500)
//	mobility.SetPositionAllocator (positionAlloc);
//	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//	// Name static nodes
//	for (uint32_t i = size; i < size + staticNodes; ++i) {
//		std::ostringstream os;
//		os << "node-­‐" << i;
//		Names::Add (os.str (), nodes.Get (i));
//		mobility.Install (nodes.Get (i));
//	}
//}

}

