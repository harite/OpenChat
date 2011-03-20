/*
 * server.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio::ip;

namespace openchat {

/**
 * Server is an abstract class that defines the interface of a basic concurrent server
 */
class Server {
public:
	Server(int port) : port_(port), serverSocket_(ioService_, udp::endpoint(udp::v4(), port)) { }
	virtual ~Server() { }
	void run() {
		// main server loop
		try {
			while (true) {
				boost::array<char, bufSize_> recvBuf;
				boost::shared_ptr<udp::endpoint> remoteEndpoint(new udp::endpoint);
				boost::system::error_code error;
				size_t len = serverSocket_.receive_from(boost::asio::buffer(recvBuf), *remoteEndpoint, 0, error);
				if (error && error != boost::asio::error::message_size)
					throw boost::system::system_error(error);
				// launch new thread to handle request
				boost::thread(boost::bind(&Server::handleRequest, this, string(recvBuf.begin(), recvBuf.begin() + len), remoteEndpoint));
			}
		} catch (exception &e) {
			// ignore this exception
		}
	}
	void stop() {
		// very brutal way to terminate the server, but it is OK in this program.
		serverSocket_.close();
	}

	// send a message to an endpoint
	void sendTo(const string &rawMessage, boost::shared_ptr<udp::endpoint> remoteEndpoint) {
		serverSocket_.send_to(boost::asio::buffer(rawMessage), *remoteEndpoint);
	}

	int getPort() const {
		return port_;
	}

	string getHostname() const {
		return boost::asio::ip::address().to_string();
	}
protected:
	/**
	 * this function is pure virtual and defines the behavior to handle incoming request.
	 */
	virtual void handleRequest(const string &rawMessage, boost::shared_ptr<udp::endpoint> remoteEndpoint) = 0;

	int port_;
	boost::asio::io_service ioService_;
	udp::socket serverSocket_;
private:
	static const size_t bufSize_ = 1 << 10;
};

}
