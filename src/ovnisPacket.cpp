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
	try {
		ReadHeader(packet);
	}
	catch (exception & e) {
		throw e;
	}
}

OvnisPacket::~OvnisPacket() {
}

	// double sendingTime, string senderId, double x, double y, int type, long id
	void OvnisPacket::ReadHeader(Ptr<Packet> packet) {
		try {
			this->packet = packet;
			t = (uint8_t*)(((malloc(200 * sizeof (uint8_t)))));
			int size = packet->CopyData(t, 200);
			tg = TagBuffer(t, t + size);
			sendingDate = tg.ReadDouble();
			uint32_t s_size = tg.ReadU32();
			senderId.clear();
			char *s = (char*)(((malloc(s_size * sizeof (char)))));
			tg.Read((uint8_t*)(((s))), s_size);
			senderId.assign(s, s_size);
			position.x = tg.ReadDouble();
			position.y = tg.ReadDouble();
			type = tg.ReadU32();
			id = tg.ReadU64();
			free(s);
		}
		catch (exception & e) {
			throw e;
		}
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
//        int size = sizeof (double) + sizeof (int) + senderIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) + sizeof (double) + sizeof (int) + vehicleIdSize * sizeof (char) + sizeof (int) + routeIdSize * sizeof (char) + sizeof (double);
//        try {
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

    Ptr<Packet> OvnisPacket::BuildPacket(double sendingTime, string senderId, double x, double y, int type, long id, double date, string vehicleId, string objectId, double objectValue) {
		int senderIdSize = senderId.size();
		int vehicleIdSize = vehicleId.size();
		int routeIdSize = objectId.size();
		int size = sizeof (double) + sizeof (int) + senderIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) + sizeof (double) + sizeof (int) + vehicleIdSize * sizeof (char) + sizeof (int) + routeIdSize * sizeof (char) + sizeof (double);
		try {
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

    Ptr<Packet> OvnisPacket::BuildTrafficInfoPacket(double sendingTime, string senderId, double x, double y, int type, long id, int numberOfRecords, Data records[]) {
		int senderIdSize = senderId.size();
		int dataRecordSize = numberOfRecords * (2 * sizeof (double) + sizeof(int) +sizeof(int));
		for (int i = 0; i < numberOfRecords; ++i) {
			dataRecordSize +=  records[i].edgeId.size() * sizeof (char);
		}
		int size = sizeof (double) + sizeof (int) + senderIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) +
			+ sizeof (int) + dataRecordSize;
		cout << "Traffic info packet: " << size << endl;

		try {
		uint8_t * t = (uint8_t*) malloc(size);
		TagBuffer tg(t, t + size);
		tg.WriteDouble(sendingTime);
		tg.WriteU32(senderIdSize);
		tg.Write((uint8_t*) senderId.c_str(), senderIdSize);
		tg.WriteDouble(x);
		tg.WriteDouble(y);
		tg.WriteU32(type);
		tg.WriteU64(id);

		tg.WriteU32(numberOfRecords);
		for (int i = 0; i < numberOfRecords; ++i) {
			// string edgeId;
			cout << "writing edge " << records[i].edgeId << " of size " << records[i].edgeId.size() << endl;
			int edgeIdSize = records[i].edgeId.size();
			tg.WriteU32(edgeIdSize);
			tg.Write((uint8_t*) records[i].edgeId.c_str(), edgeIdSize);
			// double travelTime;
			tg.WriteDouble(records[i].travelTime);
			// int numberOfVehicles;
			tg.WriteU32(records[i].numberOfVehicles);
			// double date;
			tg.WriteDouble(records[i].date);
		}

		Ptr<Packet> p = Create<Packet>(t, size);
		free(t);
		return p;
		}
		catch (exception & e) {
		return NULL;
		}
	}

//    Ptr<Packet> OvnisPacket::BuildTrafficInfoPacket(double sendingTime, string senderId, double x, double y, int type, long id, int numberOfRecords) {
//    		int senderIdSize = senderId.size();
//    		int size = sizeof (double) + sizeof (int) + senderIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) + sizeof (int) ;
//    		cout << "Traffic info packet: " << size << endl;
//
//    		try {
//				uint8_t * t = (uint8_t*) malloc(size);
//				TagBuffer tg(t, t + size);
//				tg.WriteDouble(sendingTime);
//				tg.WriteU32(senderIdSize);
//				tg.Write((uint8_t*) senderId.c_str(), senderIdSize);
//				tg.WriteDouble(x);
//				tg.WriteDouble(y);
//				tg.WriteU32(type);
//				tg.WriteU64(id);
//
//				tg.WriteU32(numberOfRecords);
//
//				Ptr<Packet> p = Create<Packet>(t, size);
//				free(t);
//				return p;
//    		}
//    		catch (exception & e) {
//    			return NULL;
//    		}
//    	}

    Ptr<Packet> OvnisPacket::BuildTravelTimePacket(double sendingTime, string senderId, double x, double y, int type, long id, string routeId, string currentEdgeId, double currentSpeed, double travelTime, double estimatedTravelTime, double estimationDate) {
	int senderIdSize = senderId.size();
	   int routeIdSize = routeId.size();
	   int edgeIdSize = currentEdgeId.size();
	   int size = sizeof (double) + sizeof (int) + senderIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) +
				+ sizeof (int) + routeIdSize * sizeof (char) + sizeof (int) + edgeIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (double) + sizeof (double);
	   try {
		uint8_t * t = (uint8_t*) malloc(size);
		TagBuffer tg(t, t + size);
		tg.WriteDouble(sendingTime);
		tg.WriteU32(senderIdSize);
		tg.Write((uint8_t*) senderId.c_str(), senderIdSize);
		tg.WriteDouble(x);
		tg.WriteDouble(y);
		tg.WriteU32(type);
		tg.WriteU64(id);

		tg.WriteU32(routeIdSize);
		tg.Write((uint8_t*) routeId.c_str(), routeIdSize);
		tg.WriteU32(edgeIdSize);
		tg.Write((uint8_t*) currentEdgeId.c_str(), edgeIdSize);

		tg.WriteDouble(currentSpeed);
		tg.WriteDouble(travelTime);
		tg.WriteDouble(estimatedTravelTime);
		tg.WriteDouble(estimationDate);

		Ptr<Packet> p = Create<Packet>(t, size);
		free(t);
		return p;
	}
	catch (exception & e) {
		return NULL;
	}
   }

    void OvnisPacket::print() const {
    	cout << "[" << sendingDate << "," << senderId << "," << position.x << "," << position.y << "] [" << id << "," << stringValue << "," << doubleValue << "]" << endl;
    }

    void OvnisPacket::setPosition(double x, double y)
    {
        this->position.x = position.x;
        this->position.y = position.y;
    }

    double OvnisPacket::getSendingDate() const
    {
        return sendingDate;
    }

} /* namespace ovnis */
