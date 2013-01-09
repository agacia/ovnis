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
 * TestApplication.h
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

#include "my-constants.h"
#include "applications/ovnis-application.h"
#include "vehicle.h"

using namespace std;

namespace ns3
{

  class TestApplication : public OvnisApplication
  {
  public:

    static TypeId GetTypeId(void);

    //UniformVariable  rando;

    TestApplication();
    virtual ~TestApplication();

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
     * Schedules Simulator for sending packet (SendState)
     */
    void SendMyState();

    /**
     * Beaconing action.
     * Gets information about current edge and vehicle position from SUMO.
     * Sets m_jam_state according whether condition for traffic jam are met for the second time in a row.
     * If there is traffic jam then broadcasts alert message containing egde's name.
     */
    void Action();

    /**
     * Creates packet with current simulation time and vehicle information.
     * Each network packet contains a byte buffer, a set of byte tags, a set of packet tags, and metadata.
     */
    Ptr<Packet> CreateStatePacket();

    /**
     *
     */
    void RetrieveStatePacket(Ptr<Packet> & packet, double & date, string & edge, ovnis::Position2D & pos);

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

    string trackedVehicle;

  };

}

#endif /* DSRAPPLICATION_H_ */
