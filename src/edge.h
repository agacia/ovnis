/*
 * edge.h
 *
 *  Created on: Aug 28, 2012
 *      Author: agata
 */

#ifndef EDGE_H_
#define EDGE_H_

#include <cstdlib>
#include <iostream>

namespace ovnis {

class Edge {
public:
	Edge();
	Edge(std::string id);
	virtual ~Edge();
	std::string print();
	std::string getId();
	double getEnteredTime();
	double getLeftTime();
	void setEnteredTime(double time);
	void setLeftTime(double time);
	double getTravelTime();
    double getSpeed() const;
    void setSpeed(double speed);
private:
    std::string id;
    double enteredTime;
    double leftTime;
    double travelTime;
    double speed;
};

} /* namespace ovnis */
#endif /* EDGE_H_ */
