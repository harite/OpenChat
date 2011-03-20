/*
 * peerproxy.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <boost/asio.hpp>
#include <string>

using namespace std;
using namespace boost::asio::ip;

namespace openchat {

/**
 * PeerProxy is a placeholder for remote peer, using Proxy Pattern
 */
class PeerProxy {
public:
	PeerProxy(const string &id, const string &hostname, const string &port, boost::asio::io_service &ioService) :
		id_(id), hostname_(hostname), port_(port) {
		try {
			udp::resolver resolver(ioService);
			udp::resolver::query query(udp::v4(), hostname, port);
			receiverEndpoint_ = *resolver.resolve(query);
			udp::socket socket(ioService);
			socket.open(udp::v4());
		} catch (exception &e) {
			cerr << "Exception in player_proxy::player_proxy(): " << e.what() << endl;
		}
	}

	void sendMessage(udp::socket &serverSocket, const string &message) {
		serverSocket.send_to(boost::asio::buffer(message), receiverEndpoint_);
	}

	// getters
	string getID() const { return id_; }
	string getHostname() const { return hostname_; }
	string getPort() const { return port_; }
private:
	string id_;
	string hostname_;
	string port_;
	udp::endpoint receiverEndpoint_;
};

}
