/*
 * edge.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: agata
 */

#include "edge.h"
#include <sstream>
#include <string>

namespace ovnis {

using namespace std;

Edge::Edge() :
	id(""), leftTime(0), enteredTime(0) {
}

Edge::Edge(string id) :
	id(id), leftTime(0), enteredTime(0) {
}

Edge::~Edge() {
}

string Edge::print() {
	stringstream out;
	out << id << "\t" << "\t" << enteredTime << "\t" << leftTime << "\t" << travelTime << endl;;
	return out.str();
}

std::string Edge::getId() {
	return id;
}

double Edge::getEnteredTime() {
	return enteredTime;
}

double Edge::getLeftTime() {
	return leftTime;
}

void Edge::setEnteredTime(double time) {
	this->enteredTime = time;
}

void Edge::setLeftTime(double time) {
	this->leftTime = time;
	this->travelTime = this->leftTime - this->enteredTime;
}

double Edge::getTravelTime() {
	return travelTime;
}

    double Edge::getSpeed() const
    {
        return speed;
    }

    void Edge::setSpeed(double speed)
    {
        this->speed = speed;}

} /* namespace ovnis */
