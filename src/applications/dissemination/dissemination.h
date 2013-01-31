/*
 * dissemination.h
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#ifndef DISSEMINATION_H_
#define DISSEMINATION_H_

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <limits.h>
#include <stdint.h>

#include "ns3/ptr.h"
#include "applications/ovnis-application.h"
#include "vehicle.h"
#include "scenario.h"
#include "knowledge.h"
#include "route.h"
#include "ovnisPacket.h"
#include "itinerary.h"
#include "recordEntry.h"

using namespace std;

namespace ovnis
{
class Dissemination {
public:
	Dissemination();
	virtual ~Dissemination();

	map<string,RecordEntry> getTrafficInformation();
	 void TryRebroadcast(OvnisPacket packet, double packetDate, std::string vehicleId);
};
}
#endif /* DISSEMINATION_H_ */
