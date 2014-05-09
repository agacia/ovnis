/*
 * status.h
 *
 *  Created on: Mar 30, 2012
 *      Author: agata
 */

#ifndef STATUS_H_
#define STATUS_H_

#include <iostream>

namespace ovnis {

class Status {
public:
	Status();
    virtual ~Status();
    std::string getDescription() const;
    int getId() const;
    int getLength() const;
    int getStatus() const;
    void setDescription(std::string description);
    void setId(int id);
    void setLength(int length);
    void setStatus(int status);
private:
    int length;
    int id;
    int status;
    std::string description;
};

} /* namespace ovnis */
#endif /* STATUS_H_ */
