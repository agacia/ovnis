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

#include "ovnis-application.h"

#include <limits.h>
#include <math.h>
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
}

void OvnisApplication::SetParams(std::map <string,string> params) {
	_applicationParams = params;
}

void OvnisApplication::ReceiveData(Ptr<Socket> x) {
}

}


Vehicle* OvnisApplication::getData() {
	Vehicle* v = new Vehicle();
	return v;
}
