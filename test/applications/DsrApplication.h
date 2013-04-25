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
 * DsrApplication.h
 *
 *  Created on: Mar 26, 2010
 *      Author: Yoann Pigné
 */

#ifndef DSRAPPLICATION_H_
#define DSRAPPLICATION_H_

#include <vector>

#include "ns3/application.h"
#include "ns3/boolean.h"
#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/global-value.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
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
#include "scenario.h"
#include "route.h"
#include "ovnisPacket.h"

using namespace std;

namespace ns3
{

  class DsrApplication : public OvnisApplication
  {
  public:

    static TypeId GetTypeId(void);

    DsrApplication();
    virtual ~DsrApplication();

    /**
     * Gets address of the neighbor.
     * Gets Packet.
     */
    virtual void ReceiveData(Ptr<Socket> );


  protected:
    virtual void DoDispose(void);

  private:

    /**
     * Inherited from Application base class.
     */
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    /**
     * Gets information about current edge and vehicle speed from SUMO.
     *
     */
    void GetCurrentSimulationState();
    void GetCurrentVehicleState();

    /**
     * Sets m_jam_state according whether condition for traffic jam are met for the second time in a row.
     * If there is traffic jam then broadcasts alert message containing egde's name.
     */
    void AnalyseVehicleState();

    void PrintRouteGlobalRouteStats();
    void PrintRouteLocalRouteStats();

    std::string ChooseMinTravelTimeRoute(std::map<std::string,double> costs);
    std::string ChooseProbTravelTimeRoute(std::map<std::string,double> costs);
    std::string ChooseFlowAwareRoute(double flow, std::map<std::string,double> costs);
	std::string ChooseRandomRoute();

    /**
	 * Beaconing action.
	 * Broadcasts periodic message.
	 * One-hop
	 */
    void Beacon();

    void SendPacket(Ptr<Packet> packet);

    Ptr<Packet> CreateTravelTimePacket();

    void TryRebroadcast(OvnisPacket packet);
//    void CheckRebroadcast(OvnisPacket packet);

    string GetEvent(vector<pair<string, double> > probabilities);
    /**
     *
     */
    bool m_verbose;

    /**
     * Port used to communicate.
     */
    uint16_t m_port;

    /**
     * IP Address used to communicate.
     */
    Ipv4Address m_addr;

    Address realTo;

    /**
     * Socket used to communicate.
     */
    Ptr<Socket> m_socket;

    /**
     * Last time a packet was re-send.
     */
    double m_last_resend;

    /**
     * True if this vehicle is considered in a traffic jam.
     */
    bool m_jam_state;

    bool arrived;

    std::string trackedVehicleId;
    bool tracked;

    double flow;

    bool decisionTaken;
    bool notificationSent;

    EventId m_travelTimeEvent;

    Network scenario;

    const static double eps = 1e-9;

    const std::vector<std::string> split(std::string sentence);

  };

}

#endif /* DSRAPPLICATION_H_ */
