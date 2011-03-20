/*
 * basicchatclient.cpp
 *
 *  Created on: Dec 6, 2010
 *      Author: Fei Huang
 *       Email: felix.fei.huang@yale.edu
 */

#include "basicchatclient.hpp"
#include "controller.hpp"

namespace openchat {

void BasicChatClient::handleRequest(const string &rawMessage, boost::shared_ptr<udp::endpoint> remoteEndpoint) {
	/**
	 * Delegate back to controller
	 * It could look strange at the first sight that BasicChatClient, as the model, calls back the controller, which
	 * is not usual in MVC architecture.  However, the reason for this here is that BasicChatClient is NOT only a
	 * model, but also the object that receives incoming messages which can be legitimately processed by the controller
	 * that is responsible for issuing commands.
	 */
	controller_->processIncomingMessage(rawMessage, remoteEndpoint);
}

}
