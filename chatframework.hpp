/*
 * chatframework.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <string>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string/trim.hpp>


#include "basicchatclient.hpp"
#include "controller.hpp"
#include "view.hpp"

namespace openchat {

class ChatFramework {
public:
	ChatFramework(const string &fileName, istream &is) : fileName_(fileName), is_(is) {
		ifstream ifs(fileName_.c_str());
		ifs >> id_ >> port_;
		model_ = boost::shared_ptr<BasicChatClient>(new BasicChatClient(id_, port_));
		ifs >> *model_;
		ifs.close();

		view_ = boost::shared_ptr<View>(new View(model_));
		controller_ = boost::shared_ptr<Controller>(new Controller(model_, view_, is_));
		model_->setMessageProcesser(controller_);
	}

	~ChatFramework() {
		ofstream ofs(fileName_.c_str());
		ofs << id_ << " " << port_ << endl << endl;
		ofs << *model_;
		ofs.close();
	}

	void addViewObserver(boost::shared_ptr<ViewObserver> observer) const {
		view_->addObserver(observer);
	}

	void run() const {
		// run the chat client
		boost::thread clientThread(boost::bind(&BasicChatClient::run, model_));
		// begin processing user input
		view_->presentLine("CPSC427 OOP Final Project\nFei Huang\nWelcome to OpenChat");
		view_->presentLine("");
		controller_->presentOpeningLine();
		bool processMore;
		do {
			view_->present(">>> ");
			string input;
			getline(is_, input);
			boost::trim(input);
			processMore = controller_->processUserInput(input);
		} while (processMore);
		// clean up: stop the client and wait the thread to join
		model_->stop();
		clientThread.join();
	}
private:
	string fileName_;
	istream &is_;
	string id_;
	int port_;
	boost::shared_ptr<BasicChatClient> model_;
	boost::shared_ptr<Controller> controller_;
	boost::shared_ptr<View> view_;
};

}


