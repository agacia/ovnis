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

	// ovnis params
	bool startSumo = true;
	string sumoHost = "localhost";
	double communicationRange = MAX_COMMUNICATION_RANGE;
	int sumoPort = 1234;

	string sumoPath = "/opt/sumo/bin/sumo-gui";
	sumoPath="/opt/sumo/bin/sumo";
	sumoPath = "/"
			"home/agata/Documents/workshop/sumo-0.18.0/bin/sumo";
	string scenarioFolder = "/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/";
	string outputFolder = "/home/agata/Documents/workshop/ovnis/scenarios/Kirchberg/test";
//	string scenarioFolder = "/home/agata/Documents/workshop/ovnis/scenarios/Highway/";
//	string outputFolder = "/home/agata/Documents/workshop/ovnis/scenarios/Highway/test";
	string sumoConfig = "scenario_accident_const.sumocfg"; // "scenario_bypass_test.sumocfg"  "scenario_main_test.sumocfg" "scenario_bypass_test_capacity.sumocfg";
	int startTime = 0; // 21600; // 6h
	int stopTime = 3600; // 25200; // 7h
    string penetrationRate = "1";

	// TrafficEQ (FceApplication) params
    string networkId = "Kirchberg"; // "Highway", "Kirchberg, Luxembourg, Berkeley"
//    string networkId = "Highway";
    string knowledgeType = "vanet";
//    knowledgeType = "perfect";
    string routingStrategies = "noRouting,shortest,probabilistic-simple,probabilistic,hybrid,mnl,clogit";
    string routingStrategiesProbabilities = "0,0,0,0,0,0,1"; // no-routing - uninformed drivers,
    string costFunctions = "travelTime,congestionLength,delayTime";
    string costFunctionProbabilities = "1,0,0";
    string vanetKnowlegePenetrationRate = "1"; // re rest uses global ideal knowledge;
    string vanetDisseminationPenetrationRate = "1"; // PENETRATION_RATE;
    string cheatersRatio = "0";
    string accidentStartTime = "300";
    string accidentStopTime = "3600";
    string ttl = "120";
    string timeEstimationMethod = "last";
    timeEstimationMethod = "average";
    string decayFactor = "0.5";

	CommandLine cmd;
	// ovnis
	cmd.AddValue("sumoConfig", "The SUMO xml config file", sumoConfig);
	cmd.AddValue("sumoPort", "Name of the port on machine hosting SUMO", sumoPort);
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
	cmd.AddValue("knowledgeType", "Use perfect real-time information (recorded by a vehicle) or vanet disseminated", knowledgeType);
	cmd.AddValue("cheatersRatio","cheatersRatio", cheatersRatio);
	cmd.AddValue("ttl","ttl", ttl);
	cmd.AddValue("timeEstimationMethod","timeEstimationMethod", timeEstimationMethod);
	cmd.AddValue("decayFactor","decayFactor", decayFactor);
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
	cout << "config\t" << sumoConfig << endl;
	cout << "networkId\t" << networkId << endl;
	cout << "scenarioFolder\t" << scenarioFolder << endl;
	cout << "outputFolder\t" << outputFolder << endl;
	cout << "startTime\t" << startTime << endl;
	cout << "stopTime\t" << stopTime << endl;
	cout << "routingStrategiesProbabilities\t" << routingStrategiesProbabilities << endl;
	cout << "ttl\t" << ttl << endl;
	cout << "timeEstimationMethod\t" << timeEstimationMethod << endl;
	cout << "decayFactor\t" << decayFactor << endl;
	cout << "knowledge type\t" << knowledgeType << endl;

	Ptr<Ovnis> expe = CreateObjectWithAttributes<Ovnis>(
			"SumoConfig", StringValue(sumoConfig),
			"SumoPath", StringValue(sumoPath),
//			"SumoPort", IntegerValue(sumoPort),
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
	ovnisParams["timeEstimationMethod"] = timeEstimationMethod;
	ovnisParams["decayFactor"] = decayFactor;
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
	fceParams["knowledgeType"] = knowledgeType;
	fceParams["ttl"] = ttl;
	fceParams["costFunctions"] = costFunctions;
	fceParams["costFunctionProbabilities"] = costFunctionProbabilities;

	expe->SetApplicationParams(fceParams);

	Simulator::Schedule(Simulator::Now(), &Ovnis::Initialize, expe);
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
