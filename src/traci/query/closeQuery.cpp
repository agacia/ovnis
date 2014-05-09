/*
 * closeQuery.cpp
 *
 *  Created on: Mar 29, 2012
 *      Author: agata
 */

#include "closeQuery.h"

namespace ovnis {

CloseQuery::CloseQuery() : Query() {
}

CloseQuery::CloseQuery(tcpip::Socket * socket)
: Query(socket) {
}

CloseQuery::~CloseQuery() {
}


void CloseQuery::InitializeCommand(Command & command) {
	command = Command(CMD_CLOSE);
	int contentSize = 0;
	command.WriteHeader(contentSize);
}

void CloseQuery::ReadResponse(tcpip::Storage & content) {

}

} /* namespace ovnis */
