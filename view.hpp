/*
 * view.hpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#pragma once

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

#include "basicchatclient.hpp"
#include "viewobserver.hpp"

using namespace std;

namespace openchat {

/**
 * View is the "view" in MVC architecture and is responsible for showing tbe results to users
 * It is also the Subject in Observer Pattern, maintaining a list of observers which do the
 * actual result-showing task.
 */
class View {
public:
	View(boost::shared_ptr<BasicChatClient> model) : model_(model) { }
	virtual ~View() { }

	void present(const string &message) const {
		for (size_t i = 0; i < observers_.size(); ++i) {
			observers_[i]->present(message);
		}
	}

	void presentLine(const string &message) const {
		for (size_t i = 0; i < observers_.size(); ++i) {
			observers_[i]->presentLine(message);
		}
	}

	void addObserver(boost::shared_ptr<ViewObserver> observer) {
		observers_.push_back(observer);
	}
private:
	boost::shared_ptr<BasicChatClient> model_;
	vector<boost::shared_ptr<ViewObserver> > observers_;
};

}
