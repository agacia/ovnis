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
 * @file ovnis.h
 * @date Apr 21, 2010
 *
 * @author Yoann Pigné
 * @author Agata Grzybek
 */

#ifndef OVNIS_H_
#define OVNIS_H_
//
// ----- NS-3 related includes
#include <ns3/object.h>
#include "ns3/node-container.h"
#include "ns3/node-list.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/ipv4-address-helper.h"

#include "ns3/wifi-helper.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-helper.h"
#include "ns3/wifi-channel.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mac48-address.h"

// ----- application related includes
#include "helper/ovnis-wifi-helper.h"
#include "applications/trafficInformationSystem.h"
#include "devices/wifi/ovnis-wifi-channel.h"
#include "traci/sumoTraciConnection.h"
#include "vehicle.h"
#include "traci/structs.h"

namespace ns3
{

  class Ovnis : public Object
  {

  public:
	static TypeId GetTypeId(void);
	Ovnis();
	virtual ~Ovnis();
	void SetApplicationParams(std::map <string,string> params);
	void SetOvnisParams(std::map <string,string> params);
	typedef std::map <Mac48Address,string> MacVehIdMap;
	typedef std::map<Mac48Address, string>::iterator MacVehIdMapIterator;
	typedef	std::map <Mac48Address,uint16_t> MacAddrMap;
	typedef std::map<Mac48Address, uint16_t>::iterator MacAddrMapIterator;
	void runSumo();

  protected:
    virtual void DoDispose(void);
    virtual void DoInitialize	(void);
    void InitializeNetwork();
    void InitializeOvnisNetwork();
    void InitializeDefaultNetwork();
    void CreateNetworkDevices(NodeContainer & node_container);
    void DestroyNetworkDevices(std::vector<std::string> to_destroy);
    void DestroyNetworkDevices(NodeContainer node_container);
    void TrafficSimulationStep();
    void UpdateInOutVehicles();
    void UpdateVehiclesPositions();
    void StartApplications();
    void CloseRoad(std::string edgeId);
    void ReadTravelTime(std::string edgeId);
    void CloseLane(std::string laneId);
    void summariseRoutes(std::vector<std::string> routes);
    void logRoutes(std::vector<std::string> routes);
    void resetRouteStats(std::vector<std::string> routes);

    void DetectCommunities();
    void writeStep(std::ostream& out);
    void writeNewNode(std::ostream& out, std::string nodeId);
    void writeChangeNode(std::ostream& out, std::string nodeId);
    void writeDeleteNode(std::ostream& out, std::string nodeId);
    void updateNodePosition(string nodeId);
    void writeEdges(ostream& out);
    void cleanTemporaryArrays();
    void MapMacVehId();
    void writeEdge(string cmd, string n1, string n2, map<string, string>& addedEdges, ostream& out);
    void InitializeSumo();
    void InitializeCrowdz();
    bool ReadCommunities(int step);
    void ReadCommunities();
    vector<string> tokenizeString(string s, string delim);

    FILE * fp_com;
    pid_t com_pid;
    pid_t sumo_pid;

	map<string, string> addedEdges;
	map<string, string> removedEdges;
	ifstream communitystream;
	int posstream;
    MacVehIdMap macList;
    UniformVariable  rando;

    // network
    NqosWifiMacHelper mac;
    ns3::WifiHelper wifi;
    Ipv4AddressHelper address;
    ObjectFactory m_application_factory;
    std::string m_ovnis_application;

    // ovnis
    OvnisWifiPhyHelper ovnisPhyHelper;
	Ptr<OvnisWifiChannel> ovnisChannel;
//	OvnisWifiChannelHelper ovnisChannelHelper;

	//default
    YansWifiPhyHelper phyHelper;
    YansWifiChannelHelper channelHelper;

    // SUMO mandatory parameters
    /**
     * The configuration file for running SUMO
     */
    std::string sumoConfig;
    /**
     * The host machine on which SUMO will run
     */
    std::string sumoHost;
    /**
     * The system path where the SUMO executable is located
     */
    std::string sumoPath;

    /**
     * The path to the scenario folder
     */
    std::string scenarioFolder;
    std::string outputFolder;

    /**
     * The port number (network) on the host machine SUMO will run on
     */
    int sumoPort;

    /**
	 * Do we start SUMO?
	 */
	bool startSumo;

    /**
     * Start time in the simulation scale (in seconds)
     */
	int startTime;

	/**
     * Stop time in the simulation scale (in seconds)
     */
	int stopTime;

	/**
     * Communication range used to subdivide the simulation space (in meters)
     */
    double communicationRange;

    /**
     * Optional ovnis params
     */
    std::map <string,string> _params;

    /**
     * Application parameters
     */
    std::map <string,string> _applicationParams;

    Ptr<ovnis::SumoTraciConnection> traci;
    std::vector<std::string> runningVehicles;
    std::vector<std::string> connectedVehicles;
    std::vector<std::string> departedVehicles;
    std::vector<std::string> arrivedVehicles;
    std::vector<std::string> lastdepartedVehicles;
    std::vector<std::string> lastarrivedVehicles;
    std::vector<std::string> lastrunningVehicles;
    int newConnectedVehiclesCount;
    double boundaries[2];
    bool is80211p;
    bool isOvnisChannel;
    int currentTime;
    time_t start;
    std::map<std::string,double> edgeTravelTime;
    std::map<std::string,long> edgeLogCount;

  };
}

#endif /* OVNIS_H_ */
