/*
 * ovnisPacket.h
 *
 *  Created on: Oct 2, 2012
 *      Author: agata
 */

#ifndef OVNISPACKET_H_
#define OVNISPACKET_H_

#include <math.h>

#include "ns3/packet.h"
#include "traci/structs.h"
#include "ovnis-constants.h"
#include "test/my-constants.h"

namespace ovnis {

class OvnisPacket {
public:
	OvnisPacket();
	OvnisPacket(ns3::Ptr<ns3::Packet> packet);
	virtual ~OvnisPacket();
    double getDate() const;
    int getPacketType() const;
    std::string getSenderId() const;
    std::string getVehicleId() const;
    Position2D getPosition() const;
    double computeDistance(double x, double y);
    double computeWaitingTime(double x, double y);
    std::string readString();
    double readDouble();
    std::string getStringValue() const;
	double getDoubleValue() const;
	long getPacketId() const;
	ns3::Ptr<ns3::Packet> getPacket() const;
	double getWaitingTime() const;
//	static ns3::Ptr<ns3::Packet> BuildTravelTimePacket(double sendingTime, std::string senderId, double x, double y, int type, long id, double date, std::string vehicleId, std::string routeId, double travelTime);
	static ns3::Ptr<ns3::Packet> BuildPacket(double sendingTime, std::string senderId, double x, double y, int type, long id, double date, std::string vehicleId, std::string objectId, double objectValue);
	void setPosition(double x, double y);
    void print() const;

private:
    void ReadHeader(ns3::Ptr<ns3::Packet> packet);
    ns3::Ptr<ns3::Packet> packet;
    ns3::TagBuffer tg;
    double sendingTime;
    std::string senderId;
    Position2D position;
    double date;
    std::string vehicleId;
    int type;
    long id;
    std::string stringValue;
    double doubleValue;
    uint8_t *t;
    double waitingTime;
};

} /* namespace ovnis */
#endif /* OVNISPACKET_H_ */
