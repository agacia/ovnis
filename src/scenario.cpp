/*
 * scenario.cpp
 *
 *  Created on: Dec 16, 2012
 *      Author: agata
 */

#include "scenario.h"

using namespace std;

namespace ovnis {

Network::Network() {

}

Network::~Network() {
	// TODO Auto-generated destructor stub
}

std::map<std::string,Route>  & Network::getAlternativeRoutes()
{
	return alternativeRoutes;
}

std::vector<std::string>  & Network::getDecisionEdges()
{
	return decisionEdges;
}

std::vector<std::string>  & Network::getNotificationEdges()
{
	return notificationEdges;
}

void Network::setAlternativeRoutes(std::map<std::string,Route> alternativeRoutes)
{
	this->alternativeRoutes = alternativeRoutes;
}

void Network::setDecisionEdges(std::vector<std::string> decisionEdges)
{
	this->decisionEdges = decisionEdges;
}

void Network::setNotificationEdges(std::vector<std::string> notificationEdges)
{
	this->notificationEdges = notificationEdges;
}

void Network::print() {

	cout << "\nDecision: ";
	for (vector<string>::iterator it = decisionEdges.begin(); it != decisionEdges.end(); ++it) {
		cout << *it << " ";
	}
	cout << "\nNotification: ";
	for (vector<string>::iterator it = notificationEdges.begin(); it != notificationEdges.end(); ++it) {
		cout << *it << " ";
	}
	cout << "\nAlternative routes: ";
	for (map<string,Route>::iterator it = alternativeRoutes.begin(); it != alternativeRoutes.end(); ++it) {
		cout << it->first << " " << it->second.printRoute();
	}
}

} /* namespace ovnis */
