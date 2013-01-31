/*
 * commonHelper.h
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#ifndef COMMONHELPER_H_
#define COMMONHELPER_H_

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iterator>

class CommonHelper {
public:
	CommonHelper();
	virtual ~CommonHelper();
	static const std::vector<std::string> split(std::string sentence);
};

#endif /* COMMONHELPER_H_ */
