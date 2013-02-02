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
			t = (uint8_t*)(((malloc(MAX_PACKET_SIZE * sizeof (uint8_t)))));
			int size = packet->CopyData(t, MAX_PACKET_SIZE);
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

    Ptr<Packet> OvnisPacket::BuildPacket(double sendingTime, string senderId, double x, double y, int type, long id, string objectId, double objectValue) {
		int senderIdSize = senderId.size();
		int objectIdSize = objectId.size();
		int size = sizeof (double) + sizeof (int) + senderIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) + sizeof (int) + objectIdSize * sizeof (char) + sizeof (double);
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
			tg.WriteU32(objectIdSize);
			tg.Write((uint8_t*) objectId.c_str(), objectIdSize);
			tg.WriteDouble(objectValue);
			Ptr<Packet> p = Create<Packet>(t, size);
			free(t);
			return p;
		}
		catch (exception & e) {
			return NULL;
		}
	}

    Ptr<Packet> OvnisPacket::BuildTrafficInfoPacket(double sendingTime, string senderId, double x, double y, int type, long id, int numberOfRecords, vector<Data> records) {
		int senderIdSize = senderId.size();
		int dataRecordSize = numberOfRecords * (2 * sizeof (double) + sizeof(int));
		for (int i = 0; i < numberOfRecords; ++i) {
			dataRecordSize +=  records[i].edgeId.size() * sizeof (char);
		}
		int size = sizeof (double) + sizeof (int) + senderIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long) +
			+ sizeof (int) + dataRecordSize;

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
			int edgeIdSize = records[i].edgeId.size();
			tg.WriteU32(edgeIdSize);
			tg.Write((uint8_t*) records[i].edgeId.c_str(), edgeIdSize);
			tg.WriteDouble(records[i].travelTime);
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

    vector<Data> OvnisPacket::ReadTrafficInfoPacket() {
    	vector<Data> records;
		int numberOfRecords = tg.ReadU32();
		for (int i = 0; i < numberOfRecords; ++i) {
			Data record;
			record.edgeId = readString();
			record.travelTime = tg.ReadDouble();
			record.date = tg.ReadDouble();
			records.push_back(record);
		}
		return records;
    }


    Ptr<Packet> OvnisPacket::BuildChangedEdgePacket(double sendingTime, string senderId, double x, double y, int type, long id, string lastEdgeId, double travelTime, string currentEdgeId) {
		int senderIdSize = senderId.size();
		int lastEdgeIdSize = lastEdgeId.size();
		int currentEdgeIdSize = currentEdgeId.size();
		int size = sizeof (double) + sizeof (int) + senderIdSize * sizeof (char) + sizeof (double) + sizeof (double) + sizeof (int) + sizeof (long)
				+ sizeof (int) + lastEdgeIdSize * sizeof (char) + sizeof (double) + sizeof (int) + currentEdgeIdSize * sizeof (char) ;
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

			tg.WriteU32(lastEdgeIdSize);
			tg.Write((uint8_t*) lastEdgeId.c_str(), lastEdgeIdSize);
			tg.WriteDouble(travelTime);
			tg.WriteU32(currentEdgeIdSize);
			tg.Write((uint8_t*) currentEdgeId.c_str(), currentEdgeIdSize);

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
