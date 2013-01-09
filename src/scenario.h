/*
 * scenario.h
 *
 *  Created on: Dec 16, 2012
 *      Author: agata
 */

#ifndef SCENARIO_H_
#define SCENARIO_H_


#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "route.h"

namespace ovnis {

class Scenario {
public:
	Scenario();
	virtual ~Scenario();
    std::map<std::string,Route>  & getAlternativeRoutes();
    std::vector<std::string>  & getDecisionEdges();
    std::vector<std::string>  & getNotificationEdges();
    void setAlternativeRoutes(std::map<std::string,Route> alternativeRoutes);
    void setDecisionEdges(std::vector<std::string> decisionEdges);
    void setNotificationEdges(std::vector<std::string> notificationEdges);
    void print();

private:
    std::vector<std::string> decisionEdges;
    std::vector<std::string> notificationEdges;
    std::map<std::string,Route> alternativeRoutes;
};

} /* namespace ovnis */

#endif /* SCENARIO_H_ */
