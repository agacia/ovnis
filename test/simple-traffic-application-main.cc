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
 *
 * @file simple-traffic-application-main.cc
 * @date Mar 31, 2010
 *
 * @author Yoann Pigné
 * @author Agata Grzybek
 *
 */

//
// ----- Global includes
#include <iostream>

//
// ----- NS-3 related includes
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/command-line.h"
//
// ----- application related includes
#include "ovnis.h"

using namespace ns3;
using namespace ovnis;
using namespace std;

NS_LOG_COMPONENT_DEFINE("MainTA");

int main(int argc, char ** argv) {

//  LogComponentEnableAll(LOG_LEVEL_ALL);
//  LogComponentEnable("OvnisApplication", LOG_LEVEL_ALL);
//  LogComponentEnable("MyApplication", LOG_LEVEL_ALL);
//  LogComponentEnable("Main", LOG_LEVEL_ALL);
//  LogComponentEnable("SubdividedWifiChannel", LOG_LEVEL_ALL);
//  LogComponentEnable("WifiChannel", LOG_LEVEL_ALL);
//  LogComponentEnable("OvnisWifiChannel", LOG_LEVEL_ALL);
//  LogComponentEnable("OvnisErrorRateModel", LOG_LEVEL_ALL);
//  LogComponentEnable("OvnisWifiPhy", LOG_LEVEL_ALL);
//  LogComponentEnable("TraciClient", LOG_LEVEL_ALL);
//  LogComponentEnable("Ovnis", LOG_LEVEL_ALL);

	// ovnis params
	bool startSumo = true;
	string sumoHost = "localhost";
	string sumoPath = "/opt/sumo/bin/sumo-gui";
	sumoPath="/opt/sumo/bin/sumo";
	sumoPath = "/home/agata/Documents/workshop/sumo-0.18.0/bin/sumo";

	double communicationRange = MAX_COMMUNICATION_RANGE;
//	string scenarioFolder = "scenarios/Highway/";
//	string outputFolder = "/Users/agatagrzybek/workspace/ovnis/scenarios/Highway/";
	string scenarioFolder = "scenarios/Kirchberg/";
	string outputFolder = "/Users/agatagrzybek/workspace/ovnis/scenarios/Kirchberg/";
	string sumoConfig = "scenario_eclipse.sumocfg"; // "scenario_bypass_test.sumocfg"  "scenario_main_test.sumocfg" "scenario_bypass_test_capacity.sumocfg";
	int startTime = 0; // 21600; // 6h
	int stopTime = 400; // 25200; // 7h
    string penetrationRate = "1";

	// TrafficEQ (FceApplication) params
    string networkId = "Kirchberg"; // "Highway", "Kirchberg, Luxembourg, Berkeley"
    string routingStrategies = "noRouting,shortest,probabilistic,hybrid";
    string routingStrategiesProbabilities = "0,0,1,0"; // no-routing - uninformed drivers,
    string costFunctions = "travelTime,congestionLength,delayTime";
    string costFunctionProbabilities = "1,0,0";
    string vanetKnowlegePenetrationRate = "1"; // re rest uses global ideal knowledge;
    string vanetDisseminationPenetrationRate = "1"; // PENETRATION_RATE;
    string cheatersRatio = "0";
    string accidentStartTime = "0";
    string accidentStopTime = "1300";

	CommandLine cmd;
	// ovnis
	cmd.AddValue("sumoConfig", "The SUMO xml config file", sumoConfig);
	cmd.AddValue("sumoHost", "Name of the machine hosting SUMO", sumoHost);
	cmd.AddValue("startTime","Date at which the network simulation starts. Before that, SUMO runs on its own. (Seconds)",startTime);
	cmd.AddValue("stopTime", "Date at which the simulation stops. (Seconds)",stopTime);
	cmd.AddValue("communicationRange", "The minimum distance limit for 2 devices to communicate. (Meters)", communicationRange);
	cmd.AddValue("startSumo","If true, ovnis will start SUMO by itself (on the same host only). If false, it is assumed that you start SUMO by yourself.",startSumo);
	cmd.AddValue("sumoPath","Path to binary file SUMO.",sumoPath);
	cmd.AddValue("scenarioFolder","Scenario folder path",scenarioFolder);
	cmd.AddValue("outputFolder","Output folder path",outputFolder);
	cmd.AddValue("penetrationRate","penetrationRate",penetrationRate);
	// fce
	cmd.AddValue("networkId", "Network name", networkId);
	cmd.AddValue("routingStrategies","Names of routing strategies",routingStrategies);
	cmd.AddValue("routingStrategiesProbabilities","Probabilities of routing strategies",routingStrategiesProbabilities);
	cmd.AddValue("cheatersRatio","cheatersRatio", cheatersRatio);
	cmd.AddValue("vanetKnowlegePenetrationRate","vanetKnowlegePenetrationRate", vanetKnowlegePenetrationRate);
	cmd.AddValue("vanetDisseminationPenetrationRate","vanetDisseminationPenetrationRate", vanetDisseminationPenetrationRate);
	cmd.AddValue("accidentStartTime","accidentStartTime", accidentStartTime);
	cmd.AddValue("accidentStopTime","accidentStopTime", accidentStopTime);
	cmd.AddValue("costFunctions","Names of routing costFunctions",costFunctions);
	cmd.AddValue("costFunctionProbabilities","Probabilities of routing costFunctions",costFunctionProbabilities);

	cmd.Parse(argc, argv);

	// reset seed to generate different numbers every time
	srand(time(0));
	cout << "Start time\t" << time(0) << endl;

	Ptr<Ovnis> expe = CreateObjectWithAttributes<Ovnis>(
			"SumoConfig", StringValue(sumoConfig),
			"SumoPath", StringValue(sumoPath),
			"SumoHost", StringValue(sumoHost),
			"StartTime", IntegerValue(startTime),
			"StopTime", IntegerValue(stopTime),
			"CommunicationRange", DoubleValue(communicationRange),
			"StartSumo", BooleanValue(startSumo),
			"ScenarioFolder", StringValue(scenarioFolder),
			"OvnisApplication", StringValue("ns3::FceApplication"));

	std::map <string,string> ovnisParams;
	ovnisParams["penetrationRate"] = penetrationRate;
	ovnisParams["outputFolder"] = outputFolder;
	expe->SetOvnisParams(ovnisParams);

	std::map <string,string> fceParams;
	fceParams["routingStrategies"] = routingStrategies;
	fceParams["cheatersRatio"] = cheatersRatio;
	fceParams["networkId"] = networkId;
	fceParams["routingStrategiesProbabilities"] = routingStrategiesProbabilities;
	fceParams["vanetKnowlegePenetrationRate"] = vanetKnowlegePenetrationRate;
	fceParams["vanetDisseminationPenetrationRate"] = vanetDisseminationPenetrationRate;
	fceParams["accidentStartTime"] = accidentStartTime;
	fceParams["accidentStopTime"] = accidentStopTime;
	fceParams["costFunctions"] = costFunctions;
	fceParams["costFunctionProbabilities"] = costFunctionProbabilities;

	expe->SetApplicationParams(fceParams);

	Simulator::Schedule(Simulator::Now(), &Ovnis::Initialize, expe);
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
