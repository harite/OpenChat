/*
 * peerlist.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "peerproxy.hpp"

using namespace std;

namespace openchat {

/**
 * PeerList stores a list of PeerProxies and supports queries and delegations regarding them
 */
class PeerList {
public:
	typedef boost::shared_ptr<PeerProxy> peerPointerType;

	void sendTo(const string &id, udp::socket &serverSocket, const string &message) const {
		peers_.find(id)->second->sendMessage(serverSocket, message);
	}

	void sendToAll(udp::socket &serverSocket, const string &message) const {
		for (map<string, boost::shared_ptr<PeerProxy> >::const_iterator iter = peers_.begin(); iter != peers_.end(); ++iter) {
			iter->second->sendMessage(serverSocket, message);
		}
	}

	void addPeer(const string &id, const string &hostname, const string &port, boost::asio::io_service &ioService) {
		boost::shared_ptr<PeerProxy> peer(new PeerProxy(id, hostname, port, ioService));
		addPeer(id, peer);
 	}

	void addPeer(const string &id, boost::shared_ptr<PeerProxy> peer) {
		peers_.insert(make_pair(id, peer));
	}

	void deletePeer(const string &id) {
		peers_.erase(id);
	}

	bool hasPeer(const string &id) const {
		return peers_.find(id) != peers_.end();
	}

	peerPointerType getPeer(const string &id) const {
		return peers_.find(id)->second;
	}

	vector<string> getPeerIDs() const {
		vector<string> ids;
		for (map<string, boost::shared_ptr<PeerProxy> >::const_iterator iter = peers_.begin(); iter != peers_.end(); ++iter) {
			ids.push_back(iter->first);
		}
		return ids;
	}

	vector<pair<string, pair<string, string> > > getPeerListInformation() const {
		vector<pair<string, pair<string, string> > > info;
		for (map<string, boost::shared_ptr<PeerProxy> >::const_iterator iter = peers_.begin(); iter != peers_.end(); ++iter) {
			string id = iter->first;
			string hostname = iter->second->getHostname();
			string port = iter->second->getPort();
			info.push_back(make_pair(id, make_pair(hostname, port)));
		}
		return info;
	}

	size_t getSize() const { return peers_.size(); }
private:
	map<string, boost::shared_ptr<PeerProxy> > peers_;
};

}
