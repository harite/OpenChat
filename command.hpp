/*
 * command.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>

#include "view.hpp"
#include "basicchatclient.hpp"
#include "chatprotocol.hpp"

using namespace std;

namespace openchat {

/**
 * Command encapsulates the operation on the chat client, using Command Pattern
 */
class Command {
public:
	Command(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name)
		: model_(model), view_(view), is_(is), name_(name) { }
	virtual ~Command() { }
	string getName() const { return name_; }

	// present information on View before executing the command
	virtual void showBeforeExecution() const = 0;

	// actually execute the command
	virtual void execute() = 0;

	// reverse the effect of the command
	virtual void undo() {
		view_->presentLine("Command: " + name_ + " is removed from command history.");
	}

	// present information on View after executing the command
	virtual void showAfterExecution() const = 0;
protected:
	boost::shared_ptr<BasicChatClient> model_;
	boost::shared_ptr<View> view_;
	istream &is_;
	string name_;
};

/**
 * Null command is a special command which does nothing, as a placeholder, using Null Pattern
 */
class NullCommand : public Command {
public:
	NullCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name)
		: Command(model, view, is, name) { }
	virtual void showBeforeExecution() const { }
	virtual void execute() { }
	virtual void undo() { }
	virtual void showAfterExecution() const { }
};

// Concrete commands

/**
 * FriendCommand shows the list of friends
 */
class FriendCommand : public Command {
public:
	FriendCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name)
		: Command(model, view, is, name) { }

	virtual void showBeforeExecution() const {
		vector<string> friendIDs = model_->getFriendsIDs();
		for (size_t i = 0; i < friendIDs.size(); ++i) {
			view_->presentLine(friendIDs[i]);
		}
	}
	virtual void execute() { }
	virtual void showAfterExecution() const { }
};

/**
 * StrangerCommand shows the list of strangers
 */
class StrangerCommand : public Command {
public:
	StrangerCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name)
		: Command(model, view, is, name) { }

	virtual void showBeforeExecution() const {
		vector<string> strangerIDs = model_->getStrangersIDs();
		for (size_t i = 0; i < strangerIDs.size(); ++i) {
			view_->presentLine(strangerIDs[i]);
		}
	}
	virtual void execute() { }
	virtual void showAfterExecution() const { }
};

/**
 * GroupCommand shows the list of groups
 */
class GroupCommand : public Command {
public:
	GroupCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name)
		: Command(model, view, is, name) { }

	virtual void showBeforeExecution() const {
		vector<string> groupsIDs = model_->getGroupsIDs();
		for (size_t i = 0; i < groupsIDs.size(); ++i) {
			view_->presentLine(groupsIDs[i]);
		}
	}
	virtual void execute() { }
	virtual void showAfterExecution() const { }
};

/**
 * ShowGroupCommand shows the list of members of a group
 */
class ShowGroupCommand : public Command {
public:
	ShowGroupCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
					 const string &id)
		: Command(model, view, is, name), id_(id) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() { }
	virtual void showAfterExecution() const {
		vector<string> memberIDs = model_->getGroupMemberIDs(id_);
		for (size_t i = 0; i < memberIDs.size(); ++i) {
			view_->presentLine(memberIDs[i]);
		}
	}
private:
	string id_;
};


/**
 * ShowSelfCommand shows the information about self
 */
class ShowSelfCommand : public Command {
public:
	ShowSelfCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name)
			: Command(model, view, is, name) { }
	virtual void showBeforeExecution() const { }
	virtual void execute() { }
	virtual void showAfterExecution() const {
		ostringstream port;
		port << model_->getPort();
		view_->presentLine("[ID=" + model_->getID() + "], [ip=" + model_->getHostname() + "], [port=" + port.str() + "]");
	}
};


/**
 * ToCommand sends a message to a contact
 */
class ToCommand : public Command {
public:
	ToCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
			  const string &toID, const string &fromID, const string &message)
		: Command(model, view, is, name), toID_(toID), fromID_(fromID), message_(message) { }
protected:
	string toID_;
	string fromID_;
	string message_;
};

/**
 * ToFriendCommand sends a message to a friend
 */
class ToFriendCommand : public ToCommand {
public:
	ToFriendCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
			        const string &toID, const string &fromID, const string &message)
		: ToCommand(model, view, is, name, toID, fromID, message) { }
	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->sendToFriend(toID_, ChatProtocol::wrapPlainChatMessage(fromID_, message_));
	}
	virtual void showAfterExecution() const { }
};

/**
 * ToStrangerCommand sends a message to a stranger
 */
class ToStrangerCommand : public ToCommand {
public:
	ToStrangerCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
			          const string &toID, const string &fromID, const string &message)
		: ToCommand(model, view, is, name, toID, fromID, message) { }
	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->sendToStranger(toID_, ChatProtocol::wrapPlainChatMessage(fromID_, message_));
	}
	virtual void showAfterExecution() const { }
};

/**
 * ToAllFriendsCommand sends a message to all friends
 */
class ToAllFriendsCommand : public ToCommand {
public:
	ToAllFriendsCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
				        const string &fromID, const string &message)
		: ToCommand(model, view, is, name, "", fromID, message) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->sendToAllFriends(ChatProtocol::wrapPlainChatMessage(fromID_, message_));
	}
	virtual void showAfterExecution() const { }
};

/**
 * ToAllStrangersCommand sends a message to all strangers
 */
class ToAllStrangersCommand : public ToCommand {
public:
	ToAllStrangersCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
				        const string &fromID, const string &message)
		: ToCommand(model, view, is, name, "", fromID, message) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->sendToAllStrangers(ChatProtocol::wrapPlainChatMessage(fromID_, message_));
	}
	virtual void showAfterExecution() const { }
};

/**
 * ToGroupCommand sends a message to a group
 */
class ToGroupCommand : public ToCommand {
public:
	ToGroupCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
				   const string &groupID, const string &fromID, const string &message)
		: ToCommand(model, view, is, name, groupID, fromID, message) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->sendToGroup(toID_, ChatProtocol::wrapPlainChatMessage(fromID_, message_));
	}
	virtual void showAfterExecution() const { }
};

/**
 * ToAllCommand sends a message to all
 */
class ToAllCommand : public ToCommand {
public:
	ToAllCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
			     const string &fromID, const string &message)
		: ToCommand(model, view, is, name, "", fromID, message) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		string message = ChatProtocol::wrapPlainChatMessage(fromID_, message_);
		model_->sendToAllFriends(message);
		model_->sendToAllStrangers(message);
	}
	virtual void showAfterExecution() const { }
};


/**
 * AddFriendCommand adds and connects a friend
 */
class AddFriendCommand : public Command {
public:
	AddFriendCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
					const string &id, const string &hostname, const string &port)
		: Command(model, view, is, name), id_(id), hostname_(hostname), port_(port) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->addFriend(id_, hostname_, port_);
	}
	virtual void undo() {
		model_->deleteFriend(id_);
		view_->presentLine("Friend: " + id_ + " has been deleted.");
	}
	virtual void showAfterExecution() const {
		view_->presentLine("Friend: " + id_ + " has been added.");
	}
private:
	string id_;
	string hostname_;
	string port_;
};

/**
 * DeleteFriendCommand deletes a friend
 */
class DeleteFriendCommand : public Command {
public:
	DeleteFriendCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
					    const string &id)
		: Command(model, view, is, name), id_(id) {
		pair<string, string> hostnameAndPort = model_->getFriendHostnameAndPort(id_);
		hostname_ = hostnameAndPort.first;
		port_ = hostnameAndPort.second;
	}

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->deleteFriend(id_);
	}
	virtual void undo() {
		model_->addFriend(id_, hostname_, port_);
		view_->presentLine("Friend: " + id_ + " has been added.");
	}
	virtual void showAfterExecution() const {
		view_->presentLine("Friend: " + id_ + " has been deleted.");
	}
private:
	string id_;
	string hostname_;
	string port_;
};

/**
 * AddGroupCommand adds a group
 */
class AddGroupCommand : public Command {
public:
	AddGroupCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
					const string &id)
		: Command(model, view, is, name), id_(id) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->addGroup(id_);
	}
	virtual void undo() {
		model_->deleteGroup(id_);
		view_->presentLine("Group: " + id_ + " has been deleted.");
	}
	virtual void showAfterExecution() const {
		view_->presentLine("Group: " + id_ + " has been added.");
	}
private:
	string id_;
};

/**
 * DeleteGroupCommand deletes a group
 */
class DeleteGroupCommand : public Command {
public:
	DeleteGroupCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
					   const string &id)
		: Command(model, view, is, name), id_(id) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		members_ = model_->getGroupMemberIDs(id_);
		model_->deleteGroup(id_);
	}
	virtual void undo() {
		model_->addGroup(id_);
		for (size_t i = 0; i < members_.size(); ++i) {
			model_->addGroupMember(id_, members_[i]);
		}
		view_->presentLine("Group: " + id_ + " together with its members has been added back.");
	}
	virtual void showAfterExecution() const {
		view_->presentLine("Group: " + id_ + " has been deleted.");
	}
private:
	string id_;
	vector<string> members_; // store member IDs for undoing
};

/**
 * AddGroupMemberCommand adds a member to a group
 */
class AddGroupMemberCommand : public Command {
public:
	AddGroupMemberCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
					      const string &groupID, const string &memberID)
		: Command(model, view, is, name), groupID_(groupID), memberID_(memberID) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->addGroupMember(groupID_, memberID_);
	}
	virtual void undo() {
		model_->deleteGroupMember(groupID_, memberID_);
		view_->presentLine("ID: " + memberID_ + " has been deleted from the group with ID: " + groupID_ + ".");
	}
	virtual void showAfterExecution() const {
		view_->presentLine("ID: " + memberID_ + " has been added to the group with ID: " + groupID_ + ".");
	}
private:
	string groupID_;
	string memberID_;
};

/**
 * DeleteGroupMemberCommand deletes a member from a group
 */
class DeleteGroupMemberCommand : public Command {
public:
	DeleteGroupMemberCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
					         const string &groupID, const string &memberID)
		: Command(model, view, is, name), groupID_(groupID), memberID_(memberID) { }

	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->deleteGroupMember(groupID_, memberID_);
	}
	virtual void undo() {
		model_->addGroupMember(groupID_, memberID_);
		view_->presentLine("ID: " + memberID_ + " has been added back to the group with ID: " + groupID_ + ".");
	}
	virtual void showAfterExecution() const {
		view_->presentLine("ID: " + memberID_ + " has been deleted from the group with ID: " + groupID_ + ".");
	}
private:
	string groupID_;
	string memberID_;
};


/**
 * ExtractFriendFriendsCommand extracts the friend list information of a friend
 */
class ExtractFriendFriendsCommand : public Command {
public:
	ExtractFriendFriendsCommand(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is, const string &name,
					            const string &id)
		: Command(model, view, is, name), id_(id) { }
	virtual void showBeforeExecution() const { }
	virtual void execute() {
		model_->sendToFriend(id_, ChatProtocol::wrapFriendListExtractionMessage(id_));
	}
	virtual void showAfterExecution() const { }
private:
	string id_;
};


}




