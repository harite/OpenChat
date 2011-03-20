/*
 * controller.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

#include "basicchatclient.hpp"
#include "view.hpp"
#include "command.hpp"

using namespace std;

namespace openchat {

/**
 * Controller is the "controller" in MVC architecture and is responsible for manipulating Client's state
 */
class Controller {
public:
	Controller(boost::shared_ptr<BasicChatClient> model, boost::shared_ptr<View> view, istream &is)
		: model_(model), view_(view), is_(is) {
		initCommandDescriptions();
	}
	virtual ~Controller() { }

	void processIncomingMessage(const string &rawMessage, boost::shared_ptr<udp::endpoint> remoteEndpoint);

	bool processUserInput(const string &input);

	// print the basic instruction to users
	void presentOpeningLine() const {
		view_->presentLine("Type \"help\" for usage, \"history\" for checking command history, \"undo\" for undoing the last command, and \"exit\" for leaving.");
	}
private:
	boost::shared_ptr<BasicChatClient> model_;
	boost::shared_ptr<View> view_;
	istream &is_;

	// store the history of used commands, supporting undo operation
	vector<boost::shared_ptr<Command> > commandHistory_;

	// store the description of commands
	map<string, string> commandDescriptions_;
	void initCommandDescriptions();

	// create a command for certain command name, using Factory Method Pattern
	boost::shared_ptr<Command> createCommand(const string &commandName);
};

}



