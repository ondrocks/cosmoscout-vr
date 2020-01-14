////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "MessageBus.hpp"

namespace cs::core {
MessageBus::MessageBus() {
}

void MessageBus::send(const MessageBus::Request& request) {
  mOnRequest.emit(request);
}

void MessageBus::send(const MessageBus::Response& response) {
  mOnResponse.emit(response);
}

void MessageBus::update() {
}

MessageBus::~MessageBus() {
}

utils::Signal<MessageBus::Request> const& MessageBus::onRequest() const {
  return mOnRequest;
}

utils::Signal<MessageBus::Response> const& MessageBus::onResponse() const {
  return mOnResponse;
}
} // namespace cs::core