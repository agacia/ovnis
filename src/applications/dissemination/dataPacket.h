/*
 * dataPacket.h
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#ifndef DATAPACKET_H_
#define DATAPACKET_H_
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
class DataPacket {
public:
	DataPacket();
	virtual ~DataPacket();
};
}
#endif /* DATAPACKET_H_ */
