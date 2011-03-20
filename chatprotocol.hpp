/*
 * chatprotocol.hpp
 *
 *  Created on: Dec 7, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <string>
#include <sstream>
#include <utility>

using namespace std;

namespace {

/**
 * ChatProtocol defines the protocol used in chat.  It provides a list of virtual methods to
 * process and parse strings used during chat.
 */
class ChatProtocol {
public:
	/**
	 * Plain chat message
	 */
	static string wrapPlainChatMessage(const string &fromID, const string &message) {
		return getPlainMessageTag() + " " + fromID + " " + message;
	}

	static bool isPlainChatMessage(const string &rawMessage) {
		return rawMessage.size() > getPlainMessageTag().size() &&
		       rawMessage.substr(0, getPlainMessageTag().size()) == getPlainMessageTag();
	}

	static pair<string, string> parsePlainChatMessage(const string &rawMessage) {
		string id, message, junk;
		istringstream iss(rawMessage);
		iss >> junk; // throw the tag
		iss >> id;
		iss >> ws;
		getline(iss, message);
		return make_pair(id, message);
	}

	/**
	 * Friend list extraction message: extracting the friend list information of a friend
	 */
	static string wrapFriendListExtractionMessage(const string &id) {
		return getFriendListExtractionMessageTag() + " " + id;
	}

	static bool isFriendListExtractionMessage(const string &rawMessage) {
		return rawMessage.size() > getFriendListExtractionMessageTag().size() &&
			   rawMessage.substr(0, getFriendListExtractionMessageTag().size()) == getFriendListExtractionMessageTag();
	}

	/**
	 * Friend list extraction response message: returning the list information back
	 */
	static string wrapFriendListExtractionResponseMessage(const string &id, const vector<pair<string, pair<string, string> > > &info) {
		ostringstream oss;
		oss << getFriendListExtractionResponseMessageTag() << " " << id << " ";
		for (size_t i = 0; i < info.size(); ++i) {
			oss << info[i].first << " " << info[i].second.first << " " << info[i].second.second << " ";
		}
		return oss.str();
	}

	static bool isFriendListExtractionResponseMessage(const string &rawMessage) {
		return rawMessage.size() > getFriendListExtractionResponseMessageTag().size() &&
			   rawMessage.substr(0, getFriendListExtractionResponseMessageTag().size()) == getFriendListExtractionResponseMessageTag();
	}

	static pair<string, vector<pair<string, pair<string, string> > > > parseFriendListExtractionResponseMessage(const string &rawMessage) {
		string junk;
		istringstream iss(rawMessage);
		iss >> junk; // throw the tag
		string fromID;
		iss >> fromID;
		vector<pair<string, pair<string, string> > > info;
		string id, hostname, port;
		while (iss >> id >> hostname >> port) {
			info.push_back(make_pair(id, make_pair(hostname, port)));
		}
		return make_pair(fromID, info);
	}
private:
	// the rule is no tag should be a prefix of any other
	static string getPlainMessageTag() {
		return "_PLAIN_";
	}
	static string getFriendListExtractionMessageTag() {
		return "_FRIEND_EXTRACTION_";
	}
	static string getFriendListExtractionResponseMessageTag() {
		return "_FRIEND_EX_RESPONSE_";
	}
};

}




