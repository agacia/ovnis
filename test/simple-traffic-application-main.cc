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

	string outputFolder = "scenarios/";
	string scenario = "Highway";
//	scenario = "Kirchberg";
//	scenario = "Luxembourg";
//	scenario = "Berkeley";
	string scenarioFolder = outputFolder + scenario;
	string sumoConfig = "scenario.sumocfg";
//	sumoConfig = "scenario_bypass_test.sumocfg";
//	sumoConfig = "scenario_main_test.sumocfg";
//	sumoConfig = "scenario_bypass_test_capacity.sumocfg";
//	string dir = "/Users/agatagrzybek/PhD/workshop/sumo-0.17.1";
	string sumoPath = "/opt/sumo/bin/sumo-gui";
	sumoPath="/opt/sumo/bin/sumo";
//	sumoPath="/home/users/agrzybek/src/sumo-0.16.0/bin"; // cluster
	string sumoHost = "localhost";
	int startTime = 0;
	int stopTime = 7000;
	stopTime = 1800;
//	stopTime = 600;
//	startTime = 21600; // 6h
//	stopTime = 25200; // 7h
	double communicationRange = MAX_COMMUNICATION_RANGE;
	bool startSumo = true;

	CommandLine cmd;
	cmd.AddValue("sumoConfig", "The SUMO xml config file", sumoConfig);
	cmd.AddValue("sumoHost", "Name of the machine hosting SUMO", sumoHost);
	cmd.AddValue(
			"startTime",
			"Date at which the network simulation starts. Before that, SUMO runs on its own. (Seconds)",
			startTime);
	cmd.AddValue("stopTime", "Date at which the simulation stops. (Seconds)",
			stopTime);
	cmd.AddValue("range",
			"The minimum distance limit for 2 devices to communicate. (Meters)",
			communicationRange);
	cmd.AddValue(
			"startSumo",
			"If true, ovnis will start SUMO by itself (on the same host only). If false, it is assumed that you start SUMO by yourself.",
			startSumo);
	cmd.AddValue(
			"sumoPath",
			"Path to binary file SUMO.",
			sumoPath);
	cmd.AddValue(
				"scenarioFolder",
				"Scenario folder path",
				scenarioFolder);
	cmd.Parse(argc, argv);

	// reset seed to generate different numbers every time
	srand(time(0));
	cout << time(0) << endl;

	Ptr<Ovnis> expe = CreateObjectWithAttributes<Ovnis>(
			"SumoConfig", StringValue(sumoConfig),
			"SumoPath", StringValue(sumoPath),
			"SumoHost", StringValue(sumoHost),
			"StartTime", IntegerValue(startTime),
			"ScenarioFolder", StringValue(scenarioFolder),
			"StopTime", IntegerValue(stopTime),
			"CommunicationRange", DoubleValue(communicationRange),
			"StartSumo", BooleanValue(startSumo),
			"OvnisApplication", StringValue("ns3::FceApplication"));
//			"OvnisApplication", StringValue("ns3::DssApplication"));
//			"OvnisApplication", StringValue("ns3::DsrApplication"));
//			StringValue("ns3::TestApplication"));

	Simulator::Schedule(Simulator::Now(), &Ovnis::Start, expe);
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
