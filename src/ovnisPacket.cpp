/*
 * ovnisPacket.cpp
 *
 *  Created on: Oct 2, 2012
 *      Author: agata
 */

#include "ovnisPacket.h"

using namespace ns3;
using namespace std;

namespace ovnis {

OvnisPacket::OvnisPacket() : tg(TagBuffer(0,0)) {
}

OvnisPacket::OvnisPacket(Ptr<Packet> packet) : tg(TagBuffer(0,0)) {
	ReadHeader(packet);
}

OvnisPacket::~OvnisPacket() {
}

// [double date, int sizeof(senderId), string senderId, double x, double y, int sizeof(packetId), string packetId]
void OvnisPacket::ReadHeader(Ptr<Packet> packet) {
	this->packet = packet;
	t = (uint8_t*)(((malloc(200 * sizeof (uint8_t)))));
        int size = packet->CopyData(t, 200);
        tg = TagBuffer(t, t + size);
        sendingTime = tg.ReadDouble();
        uint32_t s_size = tg.ReadU32();
        senderId.clear();
        char *s = (char*)(((malloc(s_size * sizeof (char)))));
        tg.Read((uint8_t*)(((s))), s_size);
        senderId.assign(s, s_size);
        position.x = tg.ReadDouble();
        position.y = tg.ReadDouble();
        type = tg.ReadU32();
        id = tg.ReadU64();
        date = tg.ReadDouble();
		s_size = tg.ReadU32();
		vehicleId.clear();
		char *s2 = (char*)(((malloc(s_size * sizeof (char)))));
		tg.Read((uint8_t*)(((s2))), s_size);
		vehicleId.assign(s2, s_size);
        free(s); free(s2);
    }

    double OvnisPacket::computeDistance(double x, double y)
    {
        double dx = this->position.x - x; //horizontal difference
        double dy = this->position.y - y; //vertical difference
        double distance = sqrt(dx * dx + dy * dy);
        return distance;
    }

    double OvnisPacket::computeWaitingTime(double x, double y)
    {
        double distance = computeDistance(x, y);
        waitingTime = RESEND_INTERVAL * (1 - distance/MAX_COMMUNICATION_RANGE);
        return waitingTime;
    }

    double OvnisPacket::getDate() const
    {
        return date;
    }

    int OvnisPacket::getPacketType() const
    {
        return type;
    }

    long OvnisPacket::getPacketId() const
	{
		return id;
	}

    double OvnisPacket::getWaitingTime() const
    {
        return waitingTime;
    }

    std::string OvnisPacket::getSenderId() const
    {
        return senderId;
    }

    std::string OvnisPacket::getVehicleId() const
    {
        return vehicleId;
    }

    Position2D OvnisPacket::getPosition() const
    {
        return position;
    }

    Ptr<Packet> OvnisPacket::getPacket() const
    {
        return packet;
    }

    string OvnisPacket::readString()
    {
        uint32_t s_size = tg.ReadU32();
        char *s = (char*)((malloc(s_size * sizeof (char))));
        tg.Read((uint8_t*)((s)), s_size);
        stringValue.assign(s, s_size);
        free(s);
        return stringValue;
    }

    double OvnisPacket::readDouble()
    {
        doubleValue = tg.ReadDouble();
        return doubleValue;
    }

    double OvnisPacket::getDoubleValue() const
    {
        return doubleValue;
    }

    string OvnisPacket::getStringValue() const
    {
        return stringValue;
    }

//    Ptr<Packet> OvnisPacket::BuildTravelTimePacket(double sendingTime, string senderId, double x, double y, int type, long id, double date, string vehicleId, string routeId, double travelTime)
//    {
//    	int senderIdSize = senderId.size();
//        int vehicleIdSize = vehicleId.size();
//        int routeIdSize = routeId.size();
//        int size = sizeof (double) + sizeof (int) + vehicleIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) + sizeof (double) + sizeof (int) + vehicleIdSize * sizeof (char) + sizeof (int) + routeIdSize * sizeof (char) + sizeof (double);
//        try {
//        	// xxx
//        	size += 2;
//			uint8_t * t = (uint8_t*) malloc(size);
//			TagBuffer tg(t, t + size);
//			tg.WriteDouble(sendingTime);
//			tg.WriteU32(senderIdSize);
//			tg.Write((uint8_t*) senderId.c_str(), senderIdSize);
//			tg.WriteDouble(x);
//			tg.WriteDouble(y);
//			tg.WriteU32(type);
//			tg.WriteU64(id);
//			tg.WriteDouble(date);
//			tg.WriteU32(vehicleIdSize);
//			tg.Write((uint8_t*) vehicleId.c_str(), vehicleIdSize);
//			tg.WriteU32(routeIdSize);
//			tg.Write((uint8_t*) routeId.c_str(), routeIdSize);
//			tg.WriteDouble(travelTime);
//			Ptr<Packet> p = Create<Packet>(t, size);
//			free(t);
//			return p;
//		}
//		catch (exception & e) {
//			return NULL;
//		}
//    }

    Ptr<Packet> OvnisPacket::BuildPacket(double sendingTime, string senderId, double x, double y, int type, long id, double date, string vehicleId, string objectId, double objectValue)
        {
        	int senderIdSize = senderId.size();
            int vehicleIdSize = vehicleId.size();
            int routeIdSize = objectId.size();
            int size = sizeof (double) + sizeof (int) + vehicleIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) + sizeof (double) + sizeof (int) + vehicleIdSize * sizeof (char) + sizeof (int) + routeIdSize * sizeof (char) + sizeof (double);
            try {
            	// xxx
            	size += 2;
    			uint8_t * t = (uint8_t*) malloc(size);
    			TagBuffer tg(t, t + size);
    			tg.WriteDouble(sendingTime);
    			tg.WriteU32(senderIdSize);
    			tg.Write((uint8_t*) senderId.c_str(), senderIdSize);
    			tg.WriteDouble(x);
    			tg.WriteDouble(y);
    			tg.WriteU32(type);
    			tg.WriteU64(id);
    			tg.WriteDouble(date);
    			tg.WriteU32(vehicleIdSize);
    			tg.Write((uint8_t*) vehicleId.c_str(), vehicleIdSize);
    			tg.WriteU32(routeIdSize);
    			tg.Write((uint8_t*) objectId.c_str(), routeIdSize);
    			tg.WriteDouble(objectValue);
    			Ptr<Packet> p = Create<Packet>(t, size);
    			free(t);
    			return p;
    		}
    		catch (exception & e) {
    			return NULL;
    		}
        }

    void OvnisPacket::print() const {
    	cout << "[" << sendingTime << "," << senderId << "," << position.x << "," << position.y << "] [" << id << "," << date << "," << vehicleId << "," << stringValue << "," << doubleValue << "]" << endl;
    }

    void OvnisPacket::setPosition(double x, double y)
    {
        this->position.x = position.x;
        this->position.y = position.y;
    }

//    int OvnisPacket::getId() const
//    {
//        return id;
//}

} /* namespace ovnis */
