/*
 * trafficInformationSystem.cpp
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#include "trafficInformationSystem.h"

using namespace std;

namespace ovnis
{
TrafficInformationSystem::TrafficInformationSystem() {
	// TODO Auto-generated constructor stub

}

TrafficInformationSystem::~TrafficInformationSystem() {
	// TODO Auto-generated destructor stub
}


void TrafficInformationSystem::ReportRoute(string routeId, string startEdgeId, string endEdgeId, double travelTime, int numberOfVehicles) {
	double date = Simulator::Now().GetSeconds();
	Log::getInstance().getStream("fixedTIS") << Simulator::Now().GetSeconds() << "\t" << routeId << "\t" << travelTime << "\t" << startEdgeId << "\t" << endEdgeId << "\t" << numberOfVehicles;
	Log::getInstance().getStream("fixedTIS") << endl;
}

void TrafficInformationSystem::DetectJam(double currentSpeed, double maxSpeed, string currentEdge) {

//	double maxLaneSpeed = vehicle.getItinerary().getEdgeMaxSpeed(currentEdge);
	//decide if JAM or not according to a threshold
	if (currentSpeed < (SPEED_THRESHOLD * maxSpeed)) {
		if (m_time_jammed == 0) {
			m_time_jammed = Simulator::Now().GetSeconds();
		}
		if (Simulator::Now().GetSeconds() - m_time_jammed > JAMMED_TIME_THRESHOLD) {
			// I am in a jam for more than JAMMED_TIME_THRESHOLD send only ofr the first time
			if (!m_jam_state) {
//				Ptr<Packet> packet = CreateWarningPacket();
//				SendPacket(packet);
	//			m_sendEvent = Simulator::ScheduleNow(&TestApplication::SendMyState,this, Simulator::Now().GetSeconds(), vehicle.getCurrentEdge());
			}
			m_jam_state = true;
			}
	}
	else {
		// if ever I was in JAM state, then I am not anymore
		if (m_jam_state) {
//			Ptr<Packet> packet = CreateWarningPacket();
//			SendPacket(packet);
			m_jam_state = false;
			m_time_jammed = 0;
		}
	}
}

void TrafficInformationSystem::setScenario(Scenario scenario) {
		this->scenario.setAlternativeRoutes(scenario.getAlternativeRoutes());
	}

string TrafficInformationSystem::TakeDecision(Knowledge knowledge, string currentEdge, string destinationEdge) {

		double now = Simulator::Now().GetSeconds();

		// costs global (vehicles report their travel time when they finish their route)
		map<string, double> globalCosts = knowledge.getGlobalCosts(scenario.getAlternativeRoutes(), currentEdge, destinationEdge);

		string global_minTravelTimeChoice = ChooseMinTravelTimeRoute(globalCosts);
		string global_proportionalProbabilisticChoice = ChooseProbTravelTimeRoute(globalCosts);

//		double capacity = scenario.getAlternativeRoutes()[chosenRouteId].getCapacity();
		string global_routeChoice = ChooseFlowAwareRoute(flow, globalCosts);

		map<string, double> vanetCosts = knowledge.getVanetCosts(scenario.getAlternativeRoutes(), currentEdge, destinationEdge);
		// costs vanets
		string vanet_minTravelTimeChoice = ChooseMinTravelTimeRoute(vanetCosts);
		string vanet_proportionalProbabilisticChoice = ChooseProbTravelTimeRoute(vanetCosts);
		string vanet_routeChoice = ChooseFlowAwareRoute(flow, vanetCosts);

		// select strategy
		string routeChoice = global_minTravelTimeChoice;
//		routeChoice = vanet_minTravelTimeChoice;
		// TODO temp
//		routeChoice = vehicle.getItinerary().getId();

		Log::getInstance().getStream("global_routing_strategies") << now << "\t" << global_minTravelTimeChoice << "\t" << global_proportionalProbabilisticChoice << "\t" << global_routeChoice << endl;
		Log::getInstance().getStream("vanet_routing_strategies") << now << "\t" << vanet_minTravelTimeChoice << "\t" << vanet_proportionalProbabilisticChoice << "\t" << vanet_routeChoice << endl;
		Log::getInstance().getStream("routing") << now << "\t" << routeChoice << endl;



		return routeChoice;
}

string TrafficInformationSystem::GetEvent(vector<pair<string, double> > probabilities) {
    double r = TrafficInformationSystem::rando.GetValue(0, 1);

//    Log::getInstance().getStream("prob") << r << "\t" << endl;
//	for (vector<pair<string, double> >::iterator it = probabilities.begin(); it != probabilities.end(); ++it)
//	{
//		Log::getInstance().getStream("prob") << it->first << "," << it->second << " ";
//	}
//	Log::getInstance().getStream("prob") << endl;

    vector<pair<string, double> >::iterator it;
    //cout << r << endl;
    for (it = probabilities.begin(); it != probabilities.end(); ++it) {
    	r -= it->second;
//    	cout << "r: " << r << " " << it->first << "," << it->second << endl;
    	if (r < eps) {
			return it->first;
		}
    }
    return "";
}


string TrafficInformationSystem::ChooseMinTravelTimeRoute(map<string,double> costs) {
	double minCost = numeric_limits<double>::max();
	string chosenRouteId = "";
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		double value = it->second;
		if (value > 0 && value < minCost) {
			minCost = value;
			chosenRouteId = it->first;
		}
	}
	return chosenRouteId;
}

string TrafficInformationSystem::ChooseRandomRoute() {
	string chosenRouteId = "";
	int chosenIndex = rand() % scenario.getAlternativeRoutes().size();
	int i = 0;
	for (map<string,Route>::iterator it = scenario.getAlternativeRoutes().begin(); it != scenario.getAlternativeRoutes().end(); ++it) {
		if (i == chosenIndex) {
			chosenRouteId = it->second.getId();
		}
		++i;
	}
	return chosenRouteId;
}


string TrafficInformationSystem::ChooseFlowAwareRoute(double flow, map<string,double> costs) {
	string chosenRouteId = ChooseMinTravelTimeRoute(costs);
	double capacity = scenario.getAlternativeRoutes()[chosenRouteId].getCapacity();
	double flowRatioNeededToUseOtherAlternatives = (flow - capacity) / flow;
	if (flowRatioNeededToUseOtherAlternatives <= 0) {
		flowRatioNeededToUseOtherAlternatives = 0;
	}
	else if (flowRatioNeededToUseOtherAlternatives >= 1) {
		flowRatioNeededToUseOtherAlternatives = 1;
	}
	double random = TrafficInformationSystem::rando.GetValue(0, 1);

	if (random < flowRatioNeededToUseOtherAlternatives) {
		map<string, double>::iterator it = costs.find(chosenRouteId);
		if (it != costs.end()) {
			costs.erase(it);
		}
		chosenRouteId = ChooseProbTravelTimeRoute(costs);
	}
	return chosenRouteId;
}

bool comp_prob(const pair<string,double>& v1, const pair<string,double>& v2)
{
	return v1.second < v2.second;
}
string TrafficInformationSystem::ChooseProbTravelTimeRoute(map<string,double> costs) {
	double minCost = numeric_limits<double>::max();
	double sumCost = 0;

	string chosenRouteId = "";

	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		sumCost += it->second;
	}
	int costsSize = costs.size();
	if (sumCost == 0 || costsSize == 0) {
		return "";
	}
	if (costsSize == 1) {
		return costs.begin()->first;
	}
	vector<pair<string, double> > sortedProbabilities = vector<pair<string, double> >();
	for (map<string, double>::iterator it = costs.begin(); it != costs.end(); ++it) {
		double probability = (sumCost-it->second)/((costsSize-1)*sumCost);
		sortedProbabilities.push_back(pair<string,double>(it->first, probability));
	}
//	sort(sortedProbabilities.begin(), sortedProbabilities.end(), comp_prob);

	chosenRouteId = GetEvent(sortedProbabilities);
	return chosenRouteId;
}

}
