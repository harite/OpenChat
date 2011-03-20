/*
 * viewbbserver.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <string>

using namespace std;

namespace openchat {

/**
 * ViewObserver is the observer in the Observer Pattern, responsible for showing the results to users
 */
class ViewObserver {
public:
	virtual ~ViewObserver() { }
	/**
	 * Define the behavior of HOW to present the information
	 */
	virtual void present(const string &message) = 0;
	virtual void presentLine(const string &message) = 0;
};

}
