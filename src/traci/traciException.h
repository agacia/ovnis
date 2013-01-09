/*
 * traciException.h
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#ifndef TRACIEXCEPTION_H_
#define TRACIEXCEPTION_H_

#include <iostream>
#include <exception>

using namespace std;

namespace ovnis {

class TraciException: exception {

public:
	TraciException();
	TraciException(string msg);
	virtual ~TraciException() throw();
	virtual const char * what() const throw();

protected:
	string msg;
};

//class UnexpectedDataException: TraciException {
//public:
//	UnexpectedDataException(string what, int expected, int got);
//};
//
//class UnexpectedDatatypeException: UnexpectedDataException {
//public:
//	UnexpectedDatatypeException(int expected, int got);
//};
//
//class UnexpectedResponseException: UnexpectedDataException {
//public:
//	UnexpectedResponseException(int expected, int got);
//};

} /* namespace ovnis */
#endif /* TRACIEXCEPTION_H_ */
