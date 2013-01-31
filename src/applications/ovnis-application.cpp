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
 * @file ovnis-application.cpp
 *
 * @author Yoann Pigné <yoann@pigne.org>
 *
 */

//#include "ovnis-application.h"


#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/udp-socket-factory.h"
#include "ovnis-application.h"
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
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tag-buffer.h"

#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"

#include "ns3/wifi-net-device.h"

#include <limits.h>
#include <math.h>
#include "ns3/config.h"
#include "ns3/integer.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/callback.h"

#include <traci-server/TraCIConstants.h>

#include "ovnis.h"
#include "ovnis-constants.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("OvnisApplication");
NS_OBJECT_ENSURE_REGISTERED(OvnisApplication);

TypeId OvnisApplication::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::OvnisApplication").SetParent<Application>().AddConstructor<OvnisApplication>();
	return tid;
}

OvnisApplication::OvnisApplication() {
}

OvnisApplication::~OvnisApplication() {
}

void OvnisApplication::DoDispose(void) {
	// chain up
	Application::DoDispose();
}

void OvnisApplication::SetStopTime (Time stop) {
       m_stopTime = stop;
       if (m_stopTime != TimeStep (0)) {
    	   m_stopEvent = Simulator::Schedule (TimeStep(fabs(stop.GetSeconds()-Simulator::Now().GetSeconds())), &OvnisApplication::StopApplication, this);
       }
}

void OvnisApplication::StartApplication(void) {
//    NS_LOG_FUNCTION (m_name);

	if (m_realStartDate == TimeStep(0)) {
		m_realStartDate = Simulator::Now();
	}

	Ptr<Object> object = GetNode();
	mobilityModel = object->GetObject<ConstantVelocityMobilityModel>();
}

void OvnisApplication::StopApplication(void) {
	cout << "OvnisApplication::StopApplication" << endl;
}

void OvnisApplication::ReceiveData(Ptr<Socket> x) {
}

}
