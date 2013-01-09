/*
 * vehicle.cpp
 *
 *  Created on: Mar 18, 2012
 *      Author: agata
 */

#include "vehicle.h"
#include "traci/structs.h"
#include "route.h"

using namespace std;

namespace ovnis {

	Vehicle::Vehicle() {
		initialize("", 0);
	}

	void Vehicle::initialize(string id, long departureTime) {
		this->id = id;
		this->departureTime = departureTime;
		this->arrivalTime = 0;
		this->travelTime = 0;
		this->currentSpeed = 0;
		this->lastSpeed = 0;
		if (id !="") {
			traci = Names::Find<ovnis::SumoTraciConnection>("SumoTraci");
			requestRoute("");
		}
	}

	Vehicle::~Vehicle() {
	}

	void Vehicle::setScenario(Scenario scenario) {
		this->scenario.setAlternativeRoutes(scenario.getAlternativeRoutes());
		this->scenario.setDecisionEdges(scenario.getDecisionEdges());
		this->scenario.setNotificationEdges(scenario.getNotificationEdges());

		for (map<string,Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
			it->second.computeLength();
			it->second.computeStaticCost();
			records[it->first].add(0,"",0,0);
		}
	}

	Scenario & Vehicle::getScenario() {
		return this->scenario;
	}

	void Vehicle::setId(string id) {
		this->id = id;
	}

	string Vehicle::getId() {
		return id;
	}

	Itinerary & Vehicle::getItinerary() {
		return itinerary;
	}

	map<string, Route> & Vehicle::getAlternativeRoutes() {
		return this->scenario.getAlternativeRoutes();
	}

	int Vehicle::getDepartureTime() const
	{
		return departureTime;
	}

	int Vehicle::getArrivalTime() const
	{
		return arrivalTime;
	}

	int Vehicle::getTravelTime() const
	{
		return travelTime;
	}

	void Vehicle::setArrivalTime(int arrivalTime)
	{
		this->arrivalTime = arrivalTime;
		itinerary.setArrivalTime(arrivalTime);

		this->travelTime = arrivalTime - this->departureTime;
		itinerary.getCurrentEdge().setLeftTime(arrivalTime);

		itinerary.computeLength();
		itinerary.computeStaticCost();
		itinerary.computeTravelTime();
	}

    double Vehicle::getLastSpeed() const
    {
        return lastSpeed;
    }

    void Vehicle::setLastSpeed(double lastSpeed)
    {
        this->lastSpeed = lastSpeed;}

	void Vehicle::requestRoute(string routeId) {
		double routeDepartureTime = Simulator::Now().GetSeconds();
		if (routeId=="") {
			routeId = traci->GetVehicleRouteId(id);
			routeDepartureTime = departureTime;
		}
		itinerary.setId(routeId);
		currentRoute = Route(routeId, traci->GetVehicleEdges(id));
		if (currentRoute.getRoute().size() > 0) {
			for (vector<string>::iterator it = scenario.getDecisionEdges().begin(); it != scenario.getDecisionEdges().end(); it++) {
				if (currentRoute.containsEdge(*it)) {
					itinerary.setDepartedEdge(*it);
				}
			}
			for (vector<string>::iterator it = scenario.getNotificationEdges().begin(); it != scenario.getNotificationEdges().end(); it++) {
				if (currentRoute.containsEdge(*it)) {
					itinerary.setArrivalEdge(*it);
				}
			}
			string newEdge = currentRoute.getRoute()[0];
			if (itinerary.getCurrentEdge().getId() == "" || newEdge != itinerary.getCurrentEdge().getId()) {
				itinerary.addEdge(newEdge);
				itinerary.setCurrentEdge(newEdge);
				itinerary.setDepartedTime(routeDepartureTime);
				itinerary.getCurrentEdge().setEnteredTime(routeDepartureTime);
			}
		}
	}

    void Vehicle::requestCurrentEdge(double currentTime)
    {
    	try {
    		string newEdge = traci->GetVehicleEdge(id);
			if (newEdge != itinerary.getCurrentEdge().getId() && currentRoute.containsEdge(newEdge)) {
				itinerary.getCurrentEdge().setLeftTime(currentTime);
				itinerary.addEdge(newEdge);
				itinerary.setCurrentEdge(newEdge);
				itinerary.getCurrentEdge().setEnteredTime(currentTime);
				itinerary.getCurrentEdge().setSpeed(currentSpeed);
			}
    	}
    	catch (TraciException &e) {
    		throw e;
    	}
    }

    void Vehicle::requestCurrentVehicleState(double currentTime) {
    	try {
    		requestCurrentEdge(currentTime);
			requestCurrentSpeed();
		}
		catch (TraciException &e) {
			throw e;
		}
    }

    void Vehicle::requestCurrentPosition() {
    	try {
        	currentPosition = traci->GetVehiclePosition(id);
		}
		catch (TraciException &e) {
			throw e;
		}
    }

    Position2D Vehicle::getCurrentPosition() const {
       	return currentPosition;
    }

    void Vehicle::requestCurrentSpeed() {
    	try {
    		double newSpeed = traci->GetVehicleSpeed(id);
			if (currentSpeed != newSpeed) {
				currentSpeed = newSpeed;
			}
		}
		catch (TraciException &e) {
			throw e;
		}
    }

	double Vehicle::getCurrentSpeed() const {
		return currentSpeed;
	}

    std::map<std::string,RecordEntry> Vehicle::getRecords() const
    {
        return records;
    }

    void Vehicle::setRecords(std::map<std::string,RecordEntry> records)
    {
        this->records = records;
    }

    void Vehicle::setTravelTime(int travelTime)
    {
        this->travelTime = travelTime;
    }

	void Vehicle::requestCurrentTravelTimes() {
		for (map<string,Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
			it->second.updateCurrentTravelTime();
		}
	}

	void Vehicle::reroute(string routeId) {
		try {

			vector<string> chosenRoute = getAlternativeRoutes()[routeId].getRoute();
			vector<string> reroute = vector<string>();
			bool metCurrentEdge = false;
			for (vector<string>::iterator it = chosenRoute.begin(); it != chosenRoute.end(); ++it) {
				if (*it == itinerary.getCurrentEdge().getId()) {
					metCurrentEdge = true;
				}
				if (metCurrentEdge) {
					reroute.push_back(*it);
				}
			}

			traci->ChangeVehicleEdges(id, reroute);
			requestRoute(routeId);
		}
		catch (TraciException & e) {
			cerr << "Cannot reroute vehicle " << id << " from:" << endl;
			cerr << currentRoute.printRoute();
			cerr << "to:" << endl;
			cerr << getAlternativeRoutes()[routeId].printRoute();
			cerr << e.what() << endl;
		}
	}

	void Vehicle::printLocalDatabase() {
		int size = records.size();
		if (size > 0) {
//			cout << "local: " ;
			map<string,RecordEntry> localrecords = records;
			for (map<string, RecordEntry>::iterator it = localrecords.begin(); it != localrecords.end(); ++it) {
//				it->second.printValues();
				Log::getInstance().getStream(it->first+"_vanet") << it->second.getLatestTime() << "\t" << it->second.getLatestValue() <<  endl;
//				cout << it->first << "," << it->second.getLatestTime() << "," << it->second.getLatestValue() << "\t";
			}
//			cout << endl;
		}
	}

	map<string, double> Vehicle::getGlobalCosts() {
		map<string, double> costs;
		map<string, double> packetAges;

		double now = Simulator::Now().GetSeconds();

		Log::getInstance().getStream("global_knowledge") << id << "\t" << now << "\t";

		for (map<string, Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
			costs[it->first] = it->second.getStaticCost();
		}
		map<string,int> vehicles = Log::getInstance().getVehiclesOnRoute();
		map<string,double> travelTimes = Log::getInstance().getTravelTimesOnRoute();
		map<string,double> travelTimeDates = Log::getInstance().getTravelTimeDateOnRoute();

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

		for (map<string, Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
			Log::getInstance().getStream("global_knowledge") << it->first << ":" << costs[it->first] << "," << packetAges[it->first] << "," << vehicles[it->first] << "\t";
		}
		Log::getInstance().getStream("global_knowledge") << endl;

		return costs;
	}

	map<string, double> Vehicle::getVanetCosts() {
		map<string, double> costs;
		map<string, double> packetAges;

		double vehsOnRoute = 0;
		double now = Simulator::Now().GetSeconds();

		Log::getInstance().getStream("local_knowledge") << id << "\t" << now << "\t";

		for (map<string, Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
			costs[it->first] = it->second.getStaticCost();
		}

		int size = records.size();
		if (size > 0) {
			map<string,RecordEntry> localrecords = records;
			for (map<string, RecordEntry>::iterator it = localrecords.begin(); it != localrecords.end(); ++it) {
				double packetAge = 0;
				if (it->second.getLatestValue() != 0) {
					packetAge = it->second.getLatestTime() == 0 ? 0 : now - it->second.getLatestTime();
					if (packetAge < PACKET_TTL) {
						costs[it->first] = it->second.getLatestValue();
					}
					else {
						packetAge = 0;
						records[it->first].reset();
					}
				}
				packetAges[it->first] = packetAge;
			}
		}

		for (map<string, Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
			Log::getInstance().getStream("local_knowledge") << it->first << ":" << costs[it->first] << "," << packetAges[it->first] << "," << vehsOnRoute << "\t";
		}
		Log::getInstance().getStream("local_knowledge") << endl;

		return costs;
	}

	void Vehicle::recordPacket(long id) {
		if (packets.find(id) == packets.end()) {
			packets[id] = 0;
		}
		++packets[id];
	}

	int Vehicle::getPacketCount(long id) {
		return packets[id];
	}

	void Vehicle::printPacketCounts(ostream & out) {
		out << "size: \t" << packets.size() << endl;
		for (map<long,int>::iterator it = packets.begin(); it != packets.end(); ++it) {
			out << it->first << "," << it->second << " " ;
		}
		out << endl;
	}

	void Vehicle::recordDouble(string id, long packetId, string senderId, double time, double value) {
		if (records[id].getLatestTime() < time) {
			records[id].add(packetId, senderId, time, value);
		}
	}

	//    void Vehicle::tryReroute(string unavailableEdge) {
	//    	set<string> ignoredEdges; // Edges that are ignored in the changing route process because not avoidable (no alternative route can be taken)
	//		// try to reroute if the jammed edge hasn't been marked as unavoidable (no alternative path exists)
	//		if (ignoredEdges.find(unavailableEdge) == ignoredEdges.end()) {
	//			// check if the jammed edge is in the planned route
	//			// TODO iterating should start from m_edge+1
	//			vector<string> route = this->route.getRoute();
	//			vector<string>::iterator it = route.begin();
	//			while (it != route.end() && (*it) != unavailableEdge) {
	//				++it;
	//			}
	//			if (it != route.end()) {
	//				// the jammed edge is in planned route
	//				// ask SUMO for rerouting
	////				traciClient->changeRoad(id, unavailableEdge, (float) INT_MAX);
	//				// get new route
	//				route.clear();
	//				vector<string>  newRoute;
	////				traciClient->getStringList(CMD_GET_VEHICLE_VARIABLE, VAR_EDGES, id, newRoute);
	//				requestRoute();
	//				vector<string> route = this->route.getRoute();
	//				// check if jammed edge is again in the new route -> it means that this is this edge is avoidable
	//				vector<string>::iterator it = newRoute.begin();
	//				while (it != newRoute.end() && (*it) != unavailableEdge) {
	//					++it;
	//				}
	//				if (it != newRoute.end()) {
	//					ignoredEdges.insert(unavailableEdge);
	//				} else {
	//					// cerr << m_name << " REROUTED: " << outList(m_route) << endl;
	//				}
	//			}
	//		}
	//    }

} /* namespace ovnis */
