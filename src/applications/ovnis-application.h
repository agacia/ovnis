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
 * OvnisApplication.h
 *
 *  Created on: Mar 26, 2010
 *      Author: Yoann Pigné
 */

#ifndef OVNIS_APPLICATION_H_
#define OVNIS_APPLICATION_H_

//#include <limits.h>
//
//#include "ns3/application.h"
//#include "ns3/nstime.h"
//#include "ns3/boolean.h"
//#include "ns3/simulator.h"
//#include "ns3/packet.h"
//#include "ns3/uinteger.h"
//#include "ns3/mac48-address.h"
//#include "ns3/address.h"
//#include "ns3/event-id.h"
//#include "ns3/traced-callback.h"
//#include "ns3/callback.h"
//#include "ns3/global-value.h"
//#include "ns3/data-rate.h"
//#include "ns3/ptr.h"
//#include "ns3/ipv4-address.h"
//#include "ns3/core-module.h"
//#include "ns3/common-module.h"
//#include "ns3/node-module.h"
//#include "ns3/helper-module.h"
//#include "ns3/mobility-module.h"
//#include "ns3/contrib-module.h"
//#include "ns3/udp-socket-factory.h"
//#include "ns3/node-list.h"
//#include "ns3/log.h"
//#include "ns3/node.h"
//#include "ns3/random-variable.h"
//#include "ns3/socket.h"
//#include "ns3/ipv4.h"
//#include "ns3/trace-source-accessor.h"
//#include "ns3/tag-buffer.h"
//#include "ns3/wifi-net-device.h"
//#include "ns3/config.h"
//#include "ns3/integer.h"
//#include "ns3/assert.h"
//
//#include "ovnis-constants.h"
//#include "applications/Vehicle.h"
//#include "log.h"

#include "ns3/application.h"

#include "ns3/nstime.h"
#include "ns3/boolean.h"

#include "ns3/simulator.h"

#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/mac48-address.h"

#include "ns3/address.h"

#include "ns3/event-id.h"
#include "ns3/traced-callback.h"
#include "ns3/callback.h"
#include "ns3/global-value.h"
#include <vector>
#include "ns3/data-rate.h"

#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include <set>

#include "traci/traci-client.h"
#include "ovnis.h"

#include "vehicle.h"


using namespace traciclient;
using namespace std;
using namespace ovnis;

namespace ns3
{

  class OvnisApplication : public Application {

  public:

	static TypeId GetTypeId(void);

    UniformVariable  rando;

    OvnisApplication();
    virtual ~OvnisApplication();

    virtual void ReceiveData(Ptr<Socket> );

    void SetStopTime (Time stop);

  private:

    // inherited from Application base class.
    virtual void StartApplication(void);
    virtual void StopApplication(void);

  protected:
    virtual void DoDispose(void);

    void GetEdgeInfo();

    void Action();

    TypeId m_tid;

    //Ptr<TraciClient> traciClient;

    Ptr<ConstantVelocityMobilityModel> mobilityModel;

    ovnis::Vehicle vehicle;

    /**
     * The vehicle's actual route
     */
    //vector<string> m_route;

    /**
     * Vehicle's name in SUMO. accessible through Names::[...]
     */
    //string m_name;

    /**
     * Vehicle's current edge
     */
    //string m_edge;

    /**
     * Vehicle's current max speed according to it's current lane.
     */
    //float m_max_speed;

    /**
     * Vehicle's current speed.
     */
    //float m_speed;

    EventId m_simulationEvent; // to obtain information about vehicle - current speed and edge

    EventId m_beaconEvent; // to send periodic packets

    EventId m_actionEvent; // to send action based packets


    Time m_realStartDate;

    double travelTimeStart;

  };

}

#endif /* OVNIS_APPLICATION_H_ */
