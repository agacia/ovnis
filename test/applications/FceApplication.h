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
#include "knowledge.h"
#include "route.h"
#include "ovnisPacket.h"
#include "applications/trafficInformationSystem.h"

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


  protected:

    virtual void DoDispose(void);

  private:

    /**
     * Inherited from Application base class.
     */
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    virtual void ReceivePacket(Ptr<Socket> );
    void SendPacket(Ptr<Packet> packet);

    /**
     * Gets current information from SUMO.
     *
     */
    void SimulationRun();

    /**
	 * Periodic sending of traffic information
	 */
    void SendTrafficInformation();



    void ReceiveTravelTimePacket(OvnisPacket ovnisPacket);
    void ReceiveTravelTimeRoutePacket(OvnisPacket ovnisPacket);
    void ReceiveTravelTimeEdgePacket(OvnisPacket ovnisPacket);

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



    bool arrived;


    bool decisionTaken;
    bool notificationSent;
    string decisionEdgeId;

    EventId m_trafficInformationEvent;

    Vehicle vehicle;
    Scenario scenario;
    Knowledge knowledge;


    const std::vector<std::string> split(std::string sentence);
    TrafficInformationSystem tis;
    MacAddrMap m_neighborList;

  };

}

#endif /* FCEAPPLICATION_H_ */
