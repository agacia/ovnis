/*
 * scenario.cpp
 *
 *  Created on: Dec 16, 2012
 *      Author: agata
 */

#include "scenario.h"

using namespace std;

namespace ovnis {

Scenario::Scenario() {

}

Scenario::~Scenario() {
	// TODO Auto-generated destructor stub
}

std::map<std::string,Route>  & Scenario::getAlternativeRoutes()
{
	return alternativeRoutes;
}

std::vector<std::string>  & Scenario::getDecisionEdges()
{
	return decisionEdges;
}

std::vector<std::string>  & Scenario::getNotificationEdges()
{
	return notificationEdges;
}

void Scenario::setAlternativeRoutes(std::map<std::string,Route> alternativeRoutes)
{
	this->alternativeRoutes = alternativeRoutes;
}

void Scenario::setDecisionEdges(std::vector<std::string> decisionEdges)
{
	this->decisionEdges = decisionEdges;
}

void Scenario::setNotificationEdges(std::vector<std::string> notificationEdges)
{
	this->notificationEdges = notificationEdges;
}

void Scenario::print() {

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
