/*
 * basicchatclient.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <algorithm>
#include <iostream>

#include "server.hpp"
#include "peerlist.hpp"

using namespace std;

namespace openchat {

class Controller;

/**
 * BasicChatClient implements the fundamental framework for chat
 */
class BasicChatClient : public Server {
public:
	BasicChatClient(const string &id, int port) : Server(port), id_(id) { }
	virtual ~BasicChatClient() { }

	void addFriend(const string &id, const string &hostname, const string &port) {
		friends_.addPeer(id, hostname, port, ioService_);
	}

	void deleteFriend(const string &id) {
		friends_.deletePeer(id);
	}

	void addStranger(const string &id, const string &hostname, const string &port) {
		strangers_.addPeer(id, hostname, port, ioService_);
	}

	void deleteStranger(const string &id) {
		friends_.deletePeer(id);
	}

	void addGroup(const string &id, const PeerList &group = PeerList()) {
		groups_.insert(make_pair(id, group));
	}

	void deleteGroup(const string &id) {
		groups_.erase(id);
	}

	void addGroupMember(const string &groupID, const string &memberID) {
		groups_[groupID].addPeer(memberID, friends_.getPeer(memberID));
	}

	void deleteGroupMember(const string &groupID, const string &memberID) {
		groups_[groupID].deletePeer(memberID);
	}

	void sendToFriend(const string &id, const string &message) {
		friends_.sendTo(id, serverSocket_, message);
	}

	void sendToAllFriends(const string &message) {
		friends_.sendToAll(serverSocket_, message);
	}

	void sendToStranger(const string &id, const string &message) {
		strangers_.sendTo(id, serverSocket_, message);
	}

	void sendToAllStrangers(const string &message) {
		strangers_.sendToAll(serverSocket_, message);
	}

	void sendToGroup(const string &groupID, const string &message) {
		groups_[groupID].sendToAll(serverSocket_, message);
	}

	void sendToAll(const string &message) {
		sendToAllFriends(message);
		sendToAllStrangers(message);
	}

	vector<string> getFriendsIDs() const {
		return friends_.getPeerIDs();
	}

	vector<string> getStrangersIDs() const {
		return strangers_.getPeerIDs();
	}

	vector<string> getGroupsIDs() const {
		vector<string> ids;
		for (map<string, PeerList>::const_iterator iter = groups_.begin(); iter != groups_.end(); ++iter) {
			ids.push_back(iter->first);
		}
		return ids;
	}

	vector<string> getGroupMemberIDs(const string &id) const {
		map<string, PeerList>::const_iterator iter = groups_.find(id);
		return iter->second.getPeerIDs();
	}

	vector<string> getAllIDs() const {
		vector<string> ids = getFriendsIDs();
		vector<string> sids = getStrangersIDs();
		for (size_t i = 0; i < sids.size(); ++i) {
			ids.push_back(sids[i]);
		}
		sort(ids.begin(), ids.end());
		return ids;
	}

	vector<pair<string, pair<string, string> > > getFriendListInformation() const {
		return friends_.getPeerListInformation();
	}

	pair<string, string> getFriendHostnameAndPort(const string &id) const {
		PeerList::peerPointerType peer = friends_.getPeer(id);
		return make_pair(peer->getHostname(), peer->getPort());
	}

	bool hasFriend(const string &id) const {
		return friends_.hasPeer(id);
	}

	bool hasStranger(const string &id) const {
		return strangers_.hasPeer(id);
	}

	bool hasGroup(const string &id) const {
		return groups_.find(id) != groups_.end();
	}

	bool hasGroupMember(const string &groupID, const string &memberID) const {
		return groups_.find(groupID)->second.hasPeer(memberID);
	}

	void setMessageProcesser(boost::shared_ptr<Controller> controller) {
		controller_ = controller;
	}

	friend istream &operator>> (istream &is, BasicChatClient &client) {
		// parsing the configuration file
		size_t friendCnt;
		string id, hostname, port;
		is >> friendCnt;
		for (size_t i = 0; i < friendCnt; ++i) {
			is >> id >> hostname >> port;
			client.addFriend(id, hostname, port);
		}
		size_t groupCnt;
		string groupID;
		is >> groupCnt;
		for (size_t i = 0; i < groupCnt; ++i) {
			is >> groupID;
			size_t groupMemberCnt;
			string memberID;
			is >> groupMemberCnt;
			PeerList group;
			for (size_t j = 0; j < groupMemberCnt; ++j) {
				is >> memberID;
				group.addPeer(memberID, client.friends_.getPeer(memberID));
			}
			client.addGroup(groupID, group);
		}

		return is;
	}

	friend ostream &operator<< (ostream &os, BasicChatClient &client) {
		os << client.friends_.getSize() << endl;
		vector<pair<string, pair<string, string> > > fInfo = client.friends_.getPeerListInformation();
		for (size_t i = 0; i < fInfo.size(); ++i) {
			os << fInfo[i].first << " " << fInfo[i].second.first << " " << fInfo[i].second.second << endl;
		}
		os << endl;
		os << client.groups_.size() << endl;
		for (map<string, PeerList>::const_iterator iter = client.groups_.begin(); iter != client.groups_.end(); ++iter) {
			os << iter->first << " " << iter->second.getSize() << " ";
			vector<string> members = iter->second.getPeerIDs();
			for (size_t i = 0; i < members.size(); ++i) {
				os << members[i] << " ";
			}
			os << endl;
		}
		return os;
	}

	string getID() const { return id_; }
protected:
	virtual void handleRequest(const string &rawMessage, boost::shared_ptr<udp::endpoint> remoteEndpoint);

	string id_;

	PeerList friends_;
	PeerList strangers_;

	// groups
	map<string, PeerList> groups_;

	boost::shared_ptr<Controller> controller_;
};

}
