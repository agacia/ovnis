/*
 * closeQuery.h
 *
 *  Created on: Mar 29, 2012
 *      Author: agata
 */

#ifndef CLOSEQUERY_H_
#define CLOSEQUERY_H_

#include "query.h"

namespace ovnis {

class CloseQuery : public Query {
public:
	CloseQuery();
	CloseQuery(tcpip::Socket * socket);
	virtual ~CloseQuery();

	virtual void InitializeCommand(Command & command);
	virtual void ReadResponse(tcpip::Storage & content);
};

} /* namespace ovnis */
#endif /* CLOSEQUERY_H_ */
