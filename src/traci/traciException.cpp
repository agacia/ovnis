/*
 * traciException.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#include "traciException.h"

namespace ovnis {

TraciException::TraciException() {
	// TODO Auto-generated constructor stub
}

TraciException::TraciException(string msg) {
	this->msg = msg;
}

TraciException::~TraciException() throw() {
	// TODO Auto-generated destructor stub
}

const char* TraciException::what() const throw () {
	return this->msg.c_str();
}

//UnexpectedDataException::UnexpectedDataException(string what, int expected, int got) :
//		TraciException("Unexpected " + what + ": expected " + expected + ", got "+ got) {
//}
//
//UnexpectedDatatypeException::UnexpectedDatatypeException(int expected, int got) :
//		UnexpectedDataException("datatype", expected, got) {
//}
//
//UnexpectedResponseException::UnexpectedResponseException(int expected, int got) :
//		UnexpectedDataException("response", expected, got) {
//}

} /* namespace ovnis */
