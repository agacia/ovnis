/*
 * status.cpp
 *
 *  Created on: Mar 30, 2012
 *      Author: agata
 */

#include "status.h"

namespace ovnis {

Status::Status() : id(0), length(0), status(0), description("") {
}

Status::~Status() {
}

std::string Status::getDescription() const {
	return description;
}

int Status::getId() const {
	return id;
}

int Status::getLength() const {
	return length;
}

int Status::getStatus() const {
	return status;
}

void Status::setDescription(std::string description) {
	this->description = description;
}

void Status::setId(int id) {
	this->id = id;
}

void Status::setLength(int length) {
	this->length = length;
}

void Status::setStatus(int status) {
	this->status = status;
}

} /* namespace ovnis */
