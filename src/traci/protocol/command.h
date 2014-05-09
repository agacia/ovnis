/*
 * command.h
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <iostream>
#include <stdint.h>
//#include <foreign/tcpip/storage.h>
//#include <foreign/tcpip/socket.h>
#include "traci/storage.h"
#include "traci/socket.h"
#include <traci-server/TraCIConstants.h>

#include "traci/traciException.h"
#include "ovnis-constants.h"

namespace ovnis {

class Command {

public:
	Command();

	Command(bool isExtended);

	/**
	 * Creates a command with a given identifier and an empty content.
	 * @param id
	 * @param isExtended Indicates if the header is of extended form
	 */
	Command(int id, bool isExtended = true);

	virtual ~Command();

	/**
	 * Returns the type identifier.
	 * @return the id
	 */
	int Id();

	/**
	 * Returns the content.
	 * @return the content
	 */
	tcpip::Storage & Content();

	/**
	 * Writes header of the message to the storage.
	 * Regular header: [lenght : unsigned byte]
	 * Extended header: [0 : unsigned byte, length : int, commandId : unsigned byte]
	 * @param contentSize the size of the content of the message (without header)
	 */
	void WriteHeader(int contentSize);

protected:
	int id;
	tcpip::Storage content;
	int headerSize;
	bool isExtended;
	void Initialize(bool isExtended);
};

} /* namespace ovnis */

#endif /* COMMAND_H_ */
