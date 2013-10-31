/*
 * commonHelper.cpp
 *
 *  Created on: Jan 31, 2013
 *      Author: agata
 */

#include "commonHelper.h"

using namespace std;

CommonHelper::CommonHelper() {
	// TODO Auto-generated constructor stub

}

CommonHelper::~CommonHelper() {
	// TODO Auto-generated destructor stub
}

const vector<string> CommonHelper::split(string input, char delimeter) {
	vector<string> tokens;
//	istringstream iss(sentence);
//	copy(istream_iterator<string>(iss),
//	istream_iterator<string>(),
//	back_inserter<vector<string> >(tokens));
//	return tokens;

	std::istringstream ss(input);
	std::string token;

	while(std::getline(ss, token, delimeter)) {
	    tokens.push_back(token);
	}
	return tokens;
}
