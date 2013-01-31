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

typedef struct data {
	std::string edgeId;
	double travelTime;
	int numberOfVehicles;
	double date;
} Data;


class OvnisPacket {
public:
	OvnisPacket();
	OvnisPacket(ns3::Ptr<ns3::Packet> packet);
	virtual ~OvnisPacket();
    int getPacketType() const;
    std::string getSenderId() const;
    double getSendingDate() const;
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
	static ns3::Ptr<ns3::Packet> BuildTravelTimePacket(double sendingTime, std::string senderId, double x, double y, int type, long id, std::string routeId, std::string currentEdgeId, double currentSpeed, double travelTime, double estimatedTravelTime, double estimationDate);
//	static ns3::Ptr<ns3::Packet> BuildTrafficInfoPacket(double sendingTime, std::string senderId, double x, double y, int type, long id, int numberOfRecords);
	static ns3::Ptr<ns3::Packet> BuildTrafficInfoPacket(double sendingTime, std::string senderId, double x, double y, int type, long id, int numberOfRecords, Data records[]);
	void setPosition(double x, double y);

    void print() const;

private:
    void ReadHeader(ns3::Ptr<ns3::Packet> packet);
    ns3::Ptr<ns3::Packet> packet;
    ns3::TagBuffer tg;
    double sendingDate;
    std::string senderId;
    Position2D position;
    int type;
    long id;
    std::string stringValue;
    double doubleValue;
    uint8_t *t;
    double waitingTime;
};

} /* namespace ovnis */
#endif /* OVNISPACKET_H_ */
