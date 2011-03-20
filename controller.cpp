/*
 * controller.cpp
 *
 *  Created on: Dec 7, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#include "controller.hpp"
#include "chatprotocol.hpp"

#include <sstream>
#include <utility>

namespace openchat {

void Controller::processIncomingMessage(const string &rawMessage, boost::shared_ptr<udp::endpoint> remoteEndpoint) {
	// parse incoming message
	if (ChatProtocol::isPlainChatMessage(rawMessage)) {
		pair<string, string> idMessage = ChatProtocol::parsePlainChatMessage(rawMessage);
		view_->presentLine("[From " + idMessage.first + "]: " + idMessage.second);
		// if the message is from nowhere, add it to the stranger list
		if (!model_->hasFriend(idMessage.first) && !model_->hasStranger(idMessage.first)) {
			string hostname, port;
			hostname = remoteEndpoint->address().to_string();
			ostringstream oss;
			oss << remoteEndpoint->port();
			port = oss.str();
			model_->addStranger(idMessage.first, hostname, port);
			view_->presentLine("ID: " + idMessage.first + " has been added to stranger list.");
		}
		view_->present(">>> "); // begin a new line
	} else if (ChatProtocol::isFriendListExtractionMessage(rawMessage)) {
		vector<pair<string, pair<string, string> > > info = model_->getFriendListInformation();
		// send the information back
		model_->sendTo(ChatProtocol::wrapFriendListExtractionResponseMessage(model_->getID(), info), remoteEndpoint);
	} else if (ChatProtocol::isFriendListExtractionResponseMessage(rawMessage)) {
		pair<string, vector<pair<string, pair<string, string> > > > idInfo = ChatProtocol::parseFriendListExtractionResponseMessage(rawMessage);
		vector<pair<string, pair<string, string> > > &info = idInfo.second;
		// present the information to view
		view_->presentLine("Friend list of [ID=" + idInfo.first + "]:");
		for (size_t i = 0; i != info.size(); ++i) {
			string id, hostname, port;
			id = info[i].first;
			hostname = info[i].second.first;
			port = info[i].second.second;
			view_->presentLine("[ID=" + id + "], [hostname=" + hostname + "], [port=" + port + "]");
		}
		view_->present(">>> "); // begin a new line
	}
}

bool Controller::processUserInput(const string &input) {
	/**
	 * Some inputs (commands) are special, because they query the information of the program rather than
	 * doing anything actually, so they are handled specially.
	 */
	if (input == "help") {
		for (map<string, string>::const_iterator iter = commandDescriptions_.begin(); iter != commandDescriptions_.end(); ++iter) {
			view_->presentLine(iter->first + ": " + iter->second);
		}
		return true;
	}
	if (input == "history") {
		for (vector<boost::shared_ptr<Command> >::reverse_iterator riter = commandHistory_.rbegin();
																   riter != commandHistory_.rend(); ++riter) {
			view_->presentLine((*riter)->getName());
		}
		return true;
	}
	if (input == "undo") {
		if (commandHistory_.empty()) {
			view_->presentLine("No command to undo.");
		} else {
			boost::shared_ptr<Command> command = commandHistory_.back(); commandHistory_.pop_back();
			command->undo();
		}
		return true;
	}
	if (input == "exit") {
		view_->presentLine("Bye bye");
		return false;
	}

	// process commands
	boost::shared_ptr<Command> command = createCommand(input);
	command->showBeforeExecution();
	command->execute();
	command->showAfterExecution();
	// if the command is not NullCommand, add it to the history
	if (typeid(command).name() != typeid(NullCommand).name()) {
		commandHistory_.push_back(command);
	}
	return true;
}

void Controller::initCommandDescriptions() {
	// add command name - command description pairs
	commandDescriptions_["friend"] = "show the list of friends' IDs.";
	commandDescriptions_["stranger"] = "show the list of strangers' IDs.";
	commandDescriptions_["group"] = "show the list of groups.";
	commandDescriptions_["group [ID]"] = "show the list of members of the group with ID [ID].";
	commandDescriptions_["self"] = "show the information of self.";
	commandDescriptions_["to [ID] [message]"] = "send [message] to the contact with ID [ID].";
	commandDescriptions_["tofriends [message]"] = "send [message] to all friends.";
	commandDescriptions_["tostrangers [message]"] = "send [message] to all strangers.";
	commandDescriptions_["togroup [GroupID] [message]"] = "send [message] to the members of the group with ID [GroupID].";
	commandDescriptions_["toall [message]"] = "send [message] to all.";
	commandDescriptions_["+friend [ID] [hostname] [port]"] = "add the friend with ID [ID] and address [hostname]:[port]";
	commandDescriptions_["-friend [ID]"] = "delete the friend with ID [ID].";
	commandDescriptions_["+group [ID]"] = "add the group with ID [ID].";
	commandDescriptions_["-group [ID]"] = "delete the group with ID [ID].";
	commandDescriptions_["+member [GroupID] [MemberID]"] = "add a member with ID [MemberID] to the group with ID [GroupID].";
	commandDescriptions_["-member [GroupID] [MemberID]"] = "delete a member with ID [MemberID] from the group with ID [GroupID].";
	commandDescriptions_["friends [ID]"] = "show the information of the friend list of the friend with ID [ID].";
}

boost::shared_ptr<Command> Controller::createCommand(const string &commandName) {
	Command *command;
	// simple commands, just one string, no need for further parsing
	if (commandName == "friend") command = new FriendCommand(model_, view_, is_, commandName);
	else if (commandName == "stranger") command = new StrangerCommand(model_, view_, is_, commandName);
	else if (commandName == "group") command = new GroupCommand(model_, view_, is_, commandName);
	else if (commandName == "self") command = new ShowSelfCommand(model_, view_, is_, commandName);
	// complex commands, need parsing
	else if (commandName.size() > 3 && commandName.substr(0, 2) == "to" && commandName[2] == ' ') {
		istringstream iss(commandName);
		string junk;
		iss >> junk; // skip "to"
		string id;
		iss >> id;
		string message;
		iss >> ws; // consume all leading spaces
		getline(iss, message); // all the rest is message
		// check if id exists in the friend or stranger list
		if (model_->hasFriend(id)) {
			command = new ToFriendCommand(model_, view_, is_, commandName, id, model_->getID(), message);
		} else if (model_->hasStranger(id)) {
			command = new ToStrangerCommand(model_, view_, is_, commandName, id, model_->getID(), message);
		} else {
			view_->presentLine("ID: " + id + " not found.");
			command = new NullCommand(model_, view_, is_, commandName);
		}
	}
	else if (commandName.size() > 6 && commandName.substr(0, 5) == "group" && commandName[5] == ' ') {
		istringstream iss(commandName);
		string junk;
		iss >> junk; // skip "group"
		string id;
		iss >> id;
		// check if group id exists
		if (model_->hasGroup(id)) {
			command = new ShowGroupCommand(model_, view_, is_, commandName, id);
		} else {
			view_->presentLine("Group ID: " + id + " not found.");
			command = new NullCommand(model_, view_, is_, commandName);
		}
	}
	else if (commandName.size() > 10 && commandName.substr(0, 9) == "tofriends" && commandName[9] == ' ') {
		istringstream iss(commandName);
		string junk;
		iss >> junk; // skip "tofriends"
		string message;
		iss >> ws;
		getline(iss, message);
		command = new ToAllFriendsCommand(model_, view_, is_, commandName, model_->getID(), message);
	}
	else if (commandName.size() > 12 && commandName.substr(0, 11) == "tostrangers" && commandName[11] == ' ') {
		istringstream iss(commandName);
		string junk;
		iss >> junk; // skip "tostrangers"
		string message;
		iss >> ws;
		getline(iss, message);
		command = new ToAllStrangersCommand(model_, view_, is_, commandName, model_->getID(), message);
	}
	else if (commandName.size() > 8 && commandName.substr(0, 7) == "togroup" && commandName[7] == ' ') {
		istringstream iss(commandName);
		string junk;
		iss >> junk; // skip "togroup"
		string groupID, message;
		iss >> groupID;
		iss >> ws;
		getline(iss, message);
		// check if groupID exists
		if (model_->hasGroup(groupID)) {
			command = new ToGroupCommand(model_, view_, is_, commandName, groupID, model_->getID(), message);
		} else {
			view_->presentLine("Group ID: " + groupID + " not found.");
			command = new NullCommand(model_, view_, is_, commandName);
		}
	}
	else if (commandName.size() > 6 && commandName.substr(0, 5) == "toall" && commandName[5] == ' ') {
		istringstream iss(commandName);
		string junk;
		iss >> junk; // skip "toall"
		string message;
		iss >> ws;
		getline(iss, message);
		command = new ToAllCommand(model_, view_, is_, commandName, model_->getID(), message);
	}
	else if (commandName.size() > 8 && (commandName.substr(0, 7) == "+friend" || commandName.substr(0, 7) == "-friend") && commandName[7] == ' ') {
		istringstream iss(commandName);
		string name;
		iss >> name;
		string id, hostname, port;
		iss >> id >> hostname >> port;
		if (name == "+friend") {
			// check if id exists in the friend list
			if (model_->hasFriend(id)) {
				view_->presentLine("ID: " + id + " has been in the friend list.");
				command = new NullCommand(model_, view_, is_, commandName);
			} else if (model_->getID() == id) {
				view_->presentLine("ID: " + id + " is yourself.");
				command = new NullCommand(model_, view_, is_, commandName);
			} else {
				command = new AddFriendCommand(model_, view_, is_, commandName, id, hostname, port);
			}
		} else { // name == "-friend"
			// check if id exists in the friend list
			if (model_->hasFriend(id)) {
				command = new DeleteFriendCommand(model_, view_, is_, commandName, id);
			} else {
				view_->presentLine("ID: " + id + " not found.");
				command = new NullCommand(model_, view_, is_, commandName);
			}
		}
	}
	else if (commandName.size() > 7 && (commandName.substr(0, 6) == "+group" || commandName.substr(0, 6) == "-group") && commandName[6] == ' ') {
		istringstream iss(commandName);
		string name;
		iss >> name;
		string id;
		iss >> id;
		if (name == "+group") {
			// check if id exists in the group list
			if (model_->hasGroup(id)) {
				view_->presentLine("Group ID: " + id + " has been in the group list.");
				command = new NullCommand(model_, view_, is_, commandName);
			} else {
				command = new AddGroupCommand(model_, view_, is_, commandName, id);
			}
		} else { // name == "-group"
			if (model_->hasGroup(id)) {
				command = new DeleteGroupCommand(model_, view_, is_, commandName, id);
			} else {
				view_->presentLine("Group ID: " + id + " not found.");
				command = new NullCommand(model_, view_, is_, commandName);
			}
		}
	}
	else if (commandName.size() > 8 && (commandName.substr(0, 7) == "+member" || commandName.substr(0, 7) == "-member") && commandName[7] == ' ') {
		istringstream iss(commandName);
		string name, groupID, memberID;
		iss >> name >> groupID >> memberID;
		if (name == "+member") {
			// check if group exists
			if (model_->hasGroup(groupID)) {
				// check if id exists in the group
				if (model_->hasGroupMember(groupID, memberID)) {
					view_->presentLine("ID: " + memberID + " has been in the group with ID: " + groupID + ".");
					command = new NullCommand(model_, view_, is_, commandName);
				} else {
					// check if id exists in the friend list
					if (model_->hasFriend(memberID)) {
						command = new AddGroupMemberCommand(model_, view_, is_, commandName, groupID, memberID);
					} else {
						view_->presentLine("ID: " + memberID + " not found.");
						command = new NullCommand(model_, view_, is_, commandName);
					}
				}
			} else {
				view_->presentLine("Group ID: " + groupID + " not found.");
				command = new NullCommand(model_, view_, is_, commandName);
			}
		} else { // name == "-member"
			// check if group exists
			if (model_->hasGroup(groupID)) {
				// check if id exists in the group
				if (model_->hasGroupMember(groupID, memberID)) {
					command = new DeleteGroupMemberCommand(model_, view_, is_, commandName, groupID, memberID);
				} else {
					view_->presentLine("ID: " + memberID + " is not found in the group with ID: " + groupID + ".");
					command = new NullCommand(model_, view_, is_, commandName);
				}
			} else {
				view_->presentLine("Group ID: " + groupID + " not found.");
				command = new NullCommand(model_, view_, is_, commandName);
			}
		}
	}
	else if (commandName.size() > 8 && commandName.substr(0, 7) == "friends" && commandName[7] == ' ') {
		istringstream iss(commandName);
		string junk;
		iss >> junk; // skip "friends"
		string id;
		iss >> id;
		// check if id exists in the friend list
		if (model_->hasFriend(id)) {
			command = new ExtractFriendFriendsCommand(model_, view_, is_, commandName, id);
		} else {
			view_->presentLine("ID: " + id + " is not a friend.");
			command = new NullCommand(model_, view_, is_, commandName);
		}
	}
	// invalid commandName
	else {
		view_->presentLine("Command: " + commandName + " not found or not understandable.");
		command = new NullCommand(model_, view_, is_, commandName);
	}

	return boost::shared_ptr<Command>(command);
}

}
