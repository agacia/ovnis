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
		initialize("");
	}

	void Vehicle::initialize(string id) {
		this->id = id;
		this->currentSpeed = 0;
		if (id!="") {
			traci = Names::Find<ovnis::SumoTraciConnection>("SumoTraci");
			requestRoute("");

		}
	}

	Vehicle::~Vehicle() {
	}

	string Vehicle::getId() {
		return id;
	}

	void Vehicle::setScenario(Network scenario) {
		this->scenario.setAlternativeRoutes(scenario.getAlternativeRoutes());
		this->scenario.setDecisionEdges(scenario.getDecisionEdges());
		this->scenario.setNotificationEdges(scenario.getNotificationEdges());
		for (map<string,Route>::iterator it = getAlternativeRoutes().begin(); it != getAlternativeRoutes().end(); ++it) {
			it->second.computeLength();
			it->second.computeStaticCost();
		}
	}

	Network & Vehicle::getScenario() {
		return this->scenario;
	}

	Itinerary & Vehicle::getItinerary() {
		return this->itinerary;
	}

	map<string, Route> & Vehicle::getAlternativeRoutes() {
		return this->scenario.getAlternativeRoutes();
	}

    string Vehicle::getDestinationEdgeId() {
    	int size = currentRoute.getEdgeIds().size() ;
    	if (size > 0) {
    		return currentRoute.getEdgeIds()[size - 1];
    	}
    	return "";
    }

    string Vehicle::getOriginEdgeId() {
    	int size = itinerary.getEdgeIds().size() ;
		if (size > 0) {
			return itinerary.getEdgeIds()[0];
		}
		return "";
    }

	void Vehicle::requestRoute(string routeId) {
		double routeDepartureTime = Simulator::Now().GetSeconds();
		if (routeId=="") {
			routeId = traci->GetVehicleRouteId(id);
		}
		itinerary.setId(routeId);
		currentRoute = Route(routeId, traci->GetVehicleEdges(id));
		if (currentRoute.getEdgeIds().size() > 0) {
			string newEdge = currentRoute.getEdgeIds()[0];
			if (itinerary.getCurrentEdge().getId() == "" || newEdge != itinerary.getCurrentEdge().getId()) {
				itinerary.addEdge(newEdge);
				itinerary.setCurrentEdge(newEdge);
				itinerary.getCurrentEdge().setEnteredTime(routeDepartureTime);
			}
		}
	}

	double Vehicle::getEdgeTravelTime(string edgeId) {
		double edgeTravelTime = traci->GetEdgeTravelTime(edgeId);
		cout << Simulator::Now().GetSeconds() << "\t" << edgeId << "\t" << edgeTravelTime << endl;
	}

    bool Vehicle::requestCurrentEdge(double currentTime)
    {
    	try {
    		string newEdge = traci->GetVehicleEdge(id);
    		if (newEdge == "") {
    			return false;
    		}
    		bool edgeChanged = false;
			if (newEdge != itinerary.getCurrentEdge().getId() && currentRoute.containsEdge(newEdge)) {
				edgeChanged = true;
				itinerary.getCurrentEdge().setLeftTime(currentTime);
				itinerary.addEdge(newEdge);
				itinerary.setCurrentEdge(newEdge);
				itinerary.getCurrentEdge().setEnteredTime(currentTime);
				itinerary.getCurrentEdge().setSpeed(currentSpeed);
			}
			return edgeChanged;
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

	void Vehicle::reroute(string routeId) {
		vector<string> reroute = vector<string>();
		try {
			vector<string> chosenRoute = getAlternativeRoutes()[routeId].getEdgeIds();
			bool metCurrentEdge = false;
			for (vector<string>::iterator it = chosenRoute.begin(); it != chosenRoute.end(); ++it) {
				if (itinerary.getCurrentEdge().getId().compare(*it)==0) {
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
			cerr << "Cannot reroute vehicle " << id << " from " << routeId << ": " << endl;
			cerr << currentRoute.printRoute();
			cerr << "to:" << endl;
			cerr << e.what() << endl;
		}
	}

	/*
	 * Including current edge (in case it's much longer than communication range)
	 */
	std::vector<std::string> Vehicle::getEdgesAhead() {
		bool metCurrentEdge = false;
		vector<string> routeAhead;
		for (vector<string>::iterator it = currentRoute.getEdgeIds().begin(); it != currentRoute.getEdgeIds().end(); ++it) {
			if (*it == itinerary.getCurrentEdge().getId()) {
				metCurrentEdge = true;
			}
			if (metCurrentEdge) {
				routeAhead.push_back(*it);
			}

		}
		return routeAhead;
	}

	map<string, double> Vehicle::getSumoCosts(string startEdgeId) {
		map<string, double> sumoCosts;
		string endEdgeId = getDestinationEdgeId();
		for (map<string, Route>::iterator itRoutes = scenario.getAlternativeRoutes().begin(); itRoutes != scenario.getAlternativeRoutes().end(); ++itRoutes) {
			for (vector<string>::iterator itEdges = itRoutes->second.getEdgeIds().begin(); itEdges != itRoutes->second.getEdgeIds().end(); ++itEdges) {
				if (itRoutes->second.containsEdgeExcludedMargins(*itEdges, startEdgeId, endEdgeId)) {
					if (sumoCosts.find(*itEdges) == sumoCosts.end()) {
						// add info about the edge
						sumoCosts[*itEdges] = traci->GetEdgeTravelTime(*itEdges);
					}
				}
			}
		}
		return sumoCosts;
	}

	//    void Vehicle::tryReroute(string unavailableEdge) {
	//    	set<string> ignoredEdges; // Edges that are ignored in the changing route process because not avoidable (no alternative route can be taken)
	//		// try to reroute if the jammed edge hasn't been marked as unavoidable (no alternative path exists)
	//		if (ignoredEdges.find(unavailableEdge) == ignoredEdges.end()) {
	//			// check if the jammed edge is in the planned route
	//			// TODO iterating should start from m_edge+1
	//			vector<string> route = this->route.getEdgeIds();
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
