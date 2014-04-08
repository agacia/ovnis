/*
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
 * FceApplication.h
 *
 *  Created on: Mar 26, 2010
 *      Author: Yoann Pigné
 */

#ifndef FCEAPPLICATION_H_
#define FCEAPPLICATION_H_

#include <vector>

#include "applications/ovnis-application.h"
#include "vehicle.h"
#include "scenario.h"
#include "knowledge.h"
#include "route.h"
#include "ovnisPacket.h"
#include "applications/trafficInformationSystem.h"
#include "applications/dissemination/dissemination.h"

using namespace std;

namespace ns3
{

  class FceApplication : public OvnisApplication
  {
  public:

    static TypeId GetTypeId(void);

    FceApplication();
    virtual ~FceApplication();

    // neighborhood discovery
    void NewNeighborFound(std::string context, Ptr<const Packet> packet, Mac48Address addr, double rxPwDbm);
    void NeighborLost(std::string context, Ptr<const Packet> packet, Mac48Address addr);
    typedef	std::map <Mac48Address,uint16_t> MacAddrMap;
    typedef std::map<Mac48Address, uint16_t>::iterator MacAddrMapIterator;
    MacAddrMap getNeighborList();

    typedef struct n {
      string id;
      double speed;
    } neighbor;

    virtual Vehicle* getData();

  protected:

    virtual void DoDispose(void);

  private:
    int _neighborCount;
    std::map<std::string, neighbor> _neighbors;

    /**
     * Inherited from Application base class.
     */
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    virtual void ReceivePacket(Ptr<Socket> );
    void SendPacket(Ptr<Packet> packet);

    void SimulationRun();

    /**
	 * Periodic sending of traffic information
	 */
    void SendTrafficInformation();
    EventId m_trafficInformationEvent;
    EventId m_neighborInformationEvent;
    bool m_verbose;

    void SendNeighborInformation(void);

    /**
     * Specifies an algorithm for data dissemination.
     * Criteria for:
     * - which data to send (e.g. only information about edges ahead, or the one that has changed or is different from expected).
     * - if/when to forward.
     */
    Dissemination dissemination;

    /**
     * Network settings
     */
    uint16_t m_port; //Port used to communicate.
    Ipv4Address m_addr; // IP Address used to communicate.
    Address realTo;
    Ptr<Socket> m_socket; //Socket used to communicate.

    /**
	 * Neighbour discovery
	 */
	void ToggleNeighborDiscovery(bool on); // Connects the callback for the neighbor discovery service
    MacAddrMap m_neighborList;
    MacAddrMap m_neighborListDiscovered;
    MacAddrMap m_neighborListLost;

    /**
     * Physical information about a vehicle, such as position, current speed. It's connected to SUMO with TraCi in both way (reading and writing).
     * Contains also the current selected route (following edges from SUMO).
     * Also keeps a track on an itinerary: remembers all traveled edges, times, etc.
     * Contains it's own scenario (in case vehicles have different scenarios)
     */
    Vehicle vehicle;

	/**
	 * Scenario settings
	 */
	map<string,string> m_params;
    void InitializeParams();
    /**
     * Abstraction of a single network from a GPS advice.
     * Contains a set of alternative routes between two points, called decision edges and notification edges.
     * All routes have information about maximum speed, capacity, length.
     * To initialize a vehicle with the scenario.
     */
    Network network;
    void SetNetwork(std::string networkId);

    /**
     * Routing settings
     */

	bool isVanet;
	Knowledge vanetsKnowledge; // A local VANET's database containing traffic information ONLY heard from other vehicles
    bool running;
	bool decisionTaken;
	bool notificationSent;
	double startReroute;
    bool isCheater;
    double expectedTravelTime;
    double selfishExpectedTravelTime;
    bool neededProbabilistic;
	string routeChoice;
	bool isDense;
	bool isCongested;
	string decisionEdgeId;
	void CalculateError(string currentEdgeId);
	void OnEdgeChanged(double now, string currentEdgeId);
	map<string, double> EstimateTravelCostBasedOnCentralised(double now, string currentEdgeId);
	map<string, double> EstimateTravelCostBasedOnVanets(double now, string currentEdgeId, string costFunction);
	string ChooseRoute(double now, string currentEdgeId, map<string, double> routeCost, string routingStrategy, double cheatersRatio);
	map<string, double> AnalyseRouteCorrelation();
	void OnReporting(double now, string currentEdgeId);

    std::map<long,int> packets; // packets counter
    const std::vector<std::string> split(std::string sentence);
  };

}

#endif /* FCEAPPLICATION_H_ */
