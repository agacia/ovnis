/*
 * simStepQuery.h
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#ifndef SIMSTEPQUERY_H_
#define SIMSTEPQUERY_H_

#include "query.h"

namespace ovnis {

class SimStepQuery : public Query {

public:
	SimStepQuery();
	SimStepQuery(tcpip::Socket * socket, int time);
	virtual ~SimStepQuery();

	virtual void ReadResponse(tcpip::Storage & content);
	virtual void InitializeCommand(Command & command);

	int GetSubscriptionCount();
    int GetCurrentTime() const;
    void SetCurrentTime(const int currentTime);
    std::vector<std::string> getArrivedVehicles() const;
    std::vector<std::string> getDepartedVehicles() const;

protected:
    void ReadSimulationSubscriptionResponse(tcpip::Storage & content);
    void ReadVehicleSubscriptionResponse(tcpip::Storage & content);

private:
	int currentTime;
	int subscriptionCount;
	std::vector<std::string> departedVehicles;
	std::vector<std::string> arrivedVehicles;
};

} /* namespace ovnis */

#endif /* SIMSTEPQUERY_H_ */
