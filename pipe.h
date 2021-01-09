#pragma once
#include "serializer/serializer.h"
#include "data.h"
#include "udp_client_server.h"
#include <map>
#include <string>
#include <deque>
enum class SendResult {
  SR_SERIALIZATION_FAIL = 1,
  SR_SERIALIZATION_OK = 2,
  SR_FAIL = 3,
  SR_OK = 4
};

enum class CeiveResult {
  CR_SERIALIZATION_FAIL = 1,
  CR_SERIALIZATION_OK = 2,
  CR_FAIL = 3,
  CR_OK = 4
};


class Pipe {
public:
  Pipe(Socket& socket) : m_socket(socket) {}

  SendResult PutEventMessage(const std::string& chanel, const std::string& event,
    const std::string& message);

  SendResult PutMyStruct(const std::string& chanel, const MyStruct& data);

  CeiveResult GetEventMessage(const std::string& chanel, std::string& outEvent,
    std::string& outMessage);

  CeiveResult GetMyStruct(const std::string& chanel, MyStruct& data);
private:
  void UpdateData();
private:
  Socket& m_socket;
  std::map<std::string, std::deque<EventMessage>> m_messages;
  std::map<std::string, std::deque<MyStruct>> m_mystructures;
};