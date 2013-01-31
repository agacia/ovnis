/*
 * knowledge.cpp
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#include "knowledge.h"

using namespace std;

namespace ovnis {

Knowledge::Knowledge() {
	// TODO Auto-generated constructor stub

}

Knowledge::~Knowledge() {
	// TODO Auto-generated destructor stub
}

std::map<std::string,RecordEntry> Knowledge::getRecords() const
{
	return records;
}

void Knowledge::setRecords(std::map<std::string,RecordEntry> records)
{
	this->records = records;
}

map<string,RecordEntry> Knowledge::getlocalKnowledge() const
{
	return localKnowledge;
}


map<string, double> Knowledge::getGlobalCosts(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	map<string, double> costs;
	map<string, double> packetAges;
	map<string,int> vehicles = Log::getInstance().getVehiclesOnRoute();
	map<string,double> travelTimes = Log::getInstance().getTravelTimesOnRoute();
	map<string,double> travelTimeDates = Log::getInstance().getTravelTimeDateOnRoute();
	double now = Simulator::Now().GetSeconds();

	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		costs[it->first] = it->second.computeStaticCostExcludingMargins(startEdgeId, endEdgeId);
	}
	for (map<string,double>::iterator it = travelTimes.begin(); it != travelTimes.end(); ++it) {
		double packetAge = 0;
		if (it->second != 0) {
			packetAge = travelTimeDates[it->first] == 0 ? 0 : now-travelTimeDates[it->first];
			if (packetAge < PACKET_TTL) {
				costs[it->first] = it->second;
			}
			else {
				packetAge = 0;
			}
		}
		packetAges[it->first] = packetAge;
	}

	Log::getInstance().getStream("global_costs") << now << "\t";
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		Log::getInstance().getStream("global_costs") << it->first << "," << costs[it->first] << "," << packetAges[it->first] << "," << vehicles[it->first] << "\t";
	}
	Log::getInstance().getStream("global_costs") << endl;

	return costs;
}
//
//void Knowledge::setScenario(Scenario scenario) {
//	this->scenario.setAlternativeRoutes(scenario.getAlternativeRoutes());
//}

map<string, double> Knowledge::getVanetCosts(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	map<string, double> costs;
	map<string, double> packetAges;

	double vehsOnRoute = 0;
	double now = Simulator::Now().GetSeconds();

	// update localKnowledge by reseting outdated packets
	int size = localKnowledge.size();
	if (size > 0) {
		map<string,RecordEntry> localrecords = localKnowledge;
		for (map<string, RecordEntry>::iterator it = localrecords.begin(); it != localrecords.end(); ++it) {
			double packetAge = it->second.getLatestTime() == 0 ? 0 : now - it->second.getLatestTime();
			if (packetAge < PACKET_TTL) {
				costs[it->first] = it->second.getLatestValue();
			}
			else {
				packetAge = 0;
				localKnowledge[it->first].reset();
			}
		}
	}

	costs = computeTravelTimesOnRoutes(routes, startEdgeId, endEdgeId);

//		int size = records.size();
//		if (size > 0) {
//			map<string,RecordEntry> localrecords = records;
//			for (map<string, RecordEntry>::iterator it = localrecords.begin(); it != localrecords.end(); ++it) {
//				double packetAge = 0;
//				if (it->second.getLatestValue() != 0) {
//					packetAge = it->second.getLatestTime() == 0 ? 0 : now - it->second.getLatestTime();
//					if (packetAge < PACKET_TTL) {
//						costs[it->first] = it->second.getLatestValue();
//					}
//					else {
//						packetAge = 0;
//						records[it->first].reset();
//					}
//				}
//				packetAges[it->first] = packetAge;
//			}
//		}
//
//		for (map<string, Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
//			Log::getInstance().getStream("local_knowledge") << it->first << ":" << costs[it->first] << "," << packetAges[it->first] << "," << vehsOnRoute << "\t";
//		}
//		Log::getInstance().getStream("local_knowledge") << endl;

	return costs;
}


void Knowledge::InitializeGlobalKnowledge(std::map<std::string,Route> routes) {
	for (map<string,Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
			Log::getInstance().vehicleOnRoadsInitialize(it->first);
		}
}

void Knowledge::InitializeStaticKnowledge(map<string, Route> routes) {
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		Route route = it->second;
		for (vector<string>::iterator it2 = route.getEdgeIds().begin(); it2 != route.getEdgeIds().end(); ++it2) {
			if (staticRecords.find(*it2) != staticRecords.end()) {
				// add info about the edge
				staticRecords[*it2] = route.getEdgeInfo(*it2);
			}
		}
	}
}

void Knowledge::recordDouble(string id, long packetId, string senderId, double time, double value) {
	if (records[id].getLatestTime() < time) {
		records[id].add(packetId, senderId, time, value);
	}
}

void Knowledge::recordEdge(string edgeId, long packetId, string senderId, double time, double travelTime) {
	// check if the time of hearing the packet is not too long
	if (time > PACKET_TTL) {
		return;
	}
	if (localKnowledge.find(edgeId) == localKnowledge.end() || localKnowledge[edgeId].getLatestTime() < time) {
		localKnowledge[edgeId].add(packetId, senderId, time, travelTime);
	}
}

void Knowledge::PrintCosts(string fileName, string vehicleId, double time, map<string, double> costs) {
	Log::getInstance().getStream(fileName) << vehicleId << "\t" << time << "\t";
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		Log::getInstance().getStream(fileName) << it->first << "," << it->second << "\t" ;
	}
	Log::getInstance().getStream(fileName) << endl;
}


void Knowledge::recordPacket(long id) {
	if (packets.find(id) == packets.end()) {
		packets[id] = 0;
	}
	++packets[id];
}

int Knowledge::getPacketCount(long id) {
	return packets[id];
}

void Knowledge::printPacketCounts(ostream & out) {
	out << "size: \t" << packets.size() << endl;
	for (map<long,int>::iterator it = packets.begin(); it != packets.end(); ++it) {
		out << it->first << "," << it->second << " " ;
	}
	out << endl;
}

map<string, double> Knowledge::computeTravelTimesOnRoutes(map<string, Route> routes, string startEdgeId, string endEdgeId) {
	map<string, double> costs;
	map<string, double> packetAges;
	map<string,int> vehicles;
	double now = Simulator::Now().GetSeconds();
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		costs[it->first] = it->second.computeStaticCost(startEdgeId, endEdgeId);
		packetAges[it->first] = 0;
		vehicles[it->first] = 0;
	}

	for (map<string, RecordEntry>::iterator it = localKnowledge.begin(); it != localKnowledge.end(); ++it) {
		string edgeId = it->first;
//			double travelTime = it->second.getLatestValue();
//			double packetDate = it->second.getLatestTime();
//			cout << edgeId << ": \t";
//			it->second.printValues();
//			cout << endl;
		double travelTime = it->second.getAverageValue();
		double packetDate = it->second.getAverageTime();

		int vehs = 0;
		double staticCost = 0;
		for (map<string, Route>::iterator itRoutes = routes.begin(); itRoutes != routes.end(); ++itRoutes) {
			if (itRoutes->second.containsEdgeExcludedMargins(edgeId, startEdgeId, endEdgeId) && packetDate > 0) {
				staticCost = itRoutes->second.getEdgeInfo(edgeId).getStaticCost();
				costs[itRoutes->first] = costs[itRoutes->first] - staticCost + travelTime;
				packetAges[itRoutes->first] += now - packetDate;
				vehicles[itRoutes->first] += vehs;
//					if (it->first == "main_2a" && travelTime > 5) {
//						cout << " !!! " << travelTime << endl;
//					}
//					cout << it->first << ": " << staticCost << "," << travelTime << "," << now - packetDate << "\n";
			}
		}
//			cout << it->first << ": " << staticCost << "," << travelTime << ", " << packetDate << "\t";
	}
//		cout << endl;
	// calculate the average information age
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		packetAges[it->first] = packetAges[it->first] / packetAges.size();
	}

//		cout << "afterUpdate: ";
//		for (map<string, Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
//			cout << it->first << ": " << costs[it->first] << " ";
//		}
//		cout << "\n\n ";

	Log::getInstance().getStream("local_costs") <<  now << "\t";
	for (map<string, Route>::iterator it = routes.begin(); it != routes.end(); ++it) {
		Log::getInstance().getStream("local_costs") << it->first << "," << costs[it->first] << "," << packetAges[it->first] << "," << vehicles[it->first] << "\t";
	}
	Log::getInstance().getStream("local_costs") << endl;

	return costs;
}

}
