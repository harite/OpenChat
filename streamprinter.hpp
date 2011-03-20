/*
 * streamprinter.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <iostream>
#include <boost/asio.hpp>

#include "viewobserver.hpp"

using namespace std;

namespace openchat {

/**
 * StreamPrinter presents messages to an output stream
 */
class StreamPrinter : public ViewObserver {
public:
	StreamPrinter(ostream &os) : os_(os) { }
	virtual ~StreamPrinter() { }

	virtual void present(const string &message) {
		// use lock to prevent messy output
		stream_lock_.lock();
		os_ << message;
		os_.flush();
		stream_lock_.unlock();
	}

	virtual void presentLine(const string &message) {
		present(message + "\n");
	}
private:
	ostream &os_;
	boost::mutex stream_lock_;
};

}

