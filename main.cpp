/*
 * main.cpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#include <iostream>
#include <string>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include "chatframework.hpp"
#include "streamprinter.hpp"

int main(int argc, char **argv) {
	if (argc < 2) {
		cout << "Usage: ./OpenChat configFileName" << endl;
		return 0;
	}
	string fileName = argv[1];
	openchat::ChatFramework framework(fileName, cin);

	// add cout view
	boost::shared_ptr<openchat::StreamPrinter> coutView(new openchat::StreamPrinter(std::cout));
	framework.addViewObserver(coutView);

	// launch the service
	framework.run();

	return 0;
}
