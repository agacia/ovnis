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
#include "ns3/data-rate.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include <vector>
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

    virtual void SetParams(std::map <string,string> params);

    Vehicle getData();

  private:

    // inherited from Application base class.
    virtual void StartApplication(void);
    virtual void StopApplication(void);

  protected:
    virtual void DoDispose(void);

    TypeId m_tid;

    Ptr<ConstantVelocityMobilityModel> mobilityModel;

    EventId m_simulationEvent; // to obtain information about vehicle - current speed and edge

    Time m_realStartDate;

    double travelTimeStart;

	std::map <string,string> _applicationParams;


  };

}

#endif /* OVNIS_APPLICATION_H_ */
