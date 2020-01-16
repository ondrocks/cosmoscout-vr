////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS_CORE_MESSAGE_BUS_HPP
#define CS_CORE_MESSAGE_BUS_HPP

#include "../cs-utils/Signal.hpp"
#include "cs_core_export.hpp"
#include <string>

namespace cs::core {
class CS_CORE_EXPORT MessageBus {
 public:
  /// A request message. eGet will typically result in a Response with type eInfo.
  /// mReceiver must equal to the namespace of the targeted plugin. E.g.: csp::atmospheres
  /// mName is the name of the targeted property
  /// mData only required for eSet requests. The value to set the property to
  /// mSender must equal to the namespace of the sending plugin. E.g.: csp::pie
  struct Request {
    enum class Type { eSet, eGet } mType;

    std::string mReceiver;
    std::string mName;
    std::string mData;
    std::string mSender;
  };

  /// A response message. eInfo for eGet requests or eSet requests that do not change the value.
  /// eChanged if a value changed mSender must equal to the namespace of the sending plugin. E.g.:
  /// csp::atmospheres mName is the name of the targeted property mData must contain the value of
  /// the requested property mRequestSender must equal to the namespace of the requesting plugin.
  /// E.g.: csp::pie
  struct Response {
    enum class Type { eInfo, eChanged } mType;

    std::string mSender;
    std::string mName;
    std::string mData;
    std::string mRequestSender;
  };

  /// Broadcast signal for all requests
  utils::Signal<Request> const& onRequest() const;

  /// Broadcast signal for all responses
  utils::Signal<Response> const& onResponse() const;

  /// Sends a request to all connected receivers
  /// @param request The request to send
  void send(Request const& request);

  /// Sends a response to all connected receivers
  /// @param response The response to send
  void send(Response const& response);

  MessageBus(){};
  ~MessageBus();

 private:
  utils::Signal<Request>  mOnRequest;
  utils::Signal<Response> mOnResponse;
};
} // namespace cs::core

#endif // CS_CORE_MESSAGE_BUS_HPP
