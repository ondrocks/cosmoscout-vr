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
  struct Request {
    enum class Type { eSet, eGet } mType;

    std::string mReceiver;
    std::string mName;
    std::string mData;
    std::string mSender;
  };

  struct Response {
    enum class Type { eInfo, eChanged } mType;

    std::string mSender;
    std::string mName;
    std::string mData;
    std::string mRequestSender;
  };

  utils::Signal<Request> const&  onRequest() const;
  utils::Signal<Response> const& onResponse() const;

  void send(Request const& request);
  void send(Response const& response);
  void update();

  MessageBus();
  ~MessageBus();

 private:
  utils::Signal<Request>  mOnRequest;
  utils::Signal<Response> mOnResponse;
};
} // namespace cs::core

#endif // CS_CORE_MESSAGE_BUS_HPP
