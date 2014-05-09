/*
 * command.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: agata
 */

#include "command.h"

using namespace tcpip;
using namespace  std;

namespace ovnis {

Command::Command() {
	Initialize(true);
}

Command::Command(bool isExtended) {
	Initialize(isExtended);
}

Command::~Command() {
}

Command::Command(int id, bool isExtended) {
	if (id > 255) {
		throw TraciException("id should fit in a byte");
	}
	Initialize(isExtended);
	content.reset();
	this->id = id;
}

void Command::Initialize(bool isExtended) {
	this->isExtended = isExtended;
	headerSize = ((isExtended == true) ? COMMAND_HEADER_EXTENDED_SIZE : COMMAND_HEADER_SIZE);
}

int Command::Id() {
	return id;
}

Storage & Command::Content() {
	return content;
}

void Command::WriteHeader(int contentSize) {
	int size = headerSize + contentSize;
	if (isExtended) {
		content.writeUnsignedByte(0);
		content.writeInt(size);
	}
	else {
		content.writeUnsignedByte(size);
	}
	content.writeUnsignedByte(id);
}

} /* namespace ovnis */
