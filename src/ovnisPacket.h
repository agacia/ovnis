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
	double date;
} Data;


class OvnisPacket {
public:
	OvnisPacket();
	OvnisPacket(ns3::Ptr<ns3::Packet> packet);
	virtual ~OvnisPacket();
    int getPacketType() const;
    std::string getSenderId() const;
    double getTimeStamp() const;
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
	static ns3::Ptr<ns3::Packet> BuildPacket(double sendingTime, std::string senderId, double x, double y, int type, std::string objectId, double objectValue);
	static ns3::Ptr<ns3::Packet> BuildChangedEdgePacket(double sendingTime, std::string senderId, double x, double y, int type, std::string lastEdgeId, double travelTime, std::string currentEdgeId);
	static ns3::Ptr<ns3::Packet> BuildTrafficInfoPacket(double sendingTime, std::string senderId, double x, double y, int type, int numberOfRecords, std::vector<Data> records);
	void setPosition(double x, double y);
	std::vector<Data> ReadTrafficInfoPacket();
    void print() const;

private:
    void ReadHeader(ns3::Ptr<ns3::Packet> packet);
    ns3::Ptr<ns3::Packet> packet;
    ns3::TagBuffer tg;
    double timeStamp;
    std::string senderId;
    Position2D position;
    int type;
    int packetSize;
    std::string stringValue;
    double doubleValue;
    uint8_t *t;
    double waitingTime;
};

} /* namespace ovnis */
#endif /* OVNISPACKET_H_ */
