#include "udp_client_server.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

class BinarySerializer {
public:
  BinarySerializer(std::vector<uint8_t> &&in) : m_buffer(std::move(in)) {
    m_iterator = m_buffer.begin();
  }
  BinarySerializer() {
    m_iterator = m_buffer.begin();
  }
  template <typename T> void operator<<(const T &data) {
    if (std::distance(m_iterator, m_buffer.end()) <= sizeof(T))
      m_buffer.resize(m_buffer.size() * 2);
    *(reinterpret_cast<T *>(&(*m_iterator))) = data;
    m_iterator += sizeof(T);
  }

  template <typename T> void operator>>(T &data) {
    if (std::distance(m_iterator, m_buffer.end()) > sizeof(T)) {
      data = *(reinterpret_cast<T*>((*m_iterator)));
      m_iterator += sizeof(T);
    }
    else {
      throw std::runtime_error("out of data range");
    }
  }

  void operator<<(const BinarySerializer& data) {
    auto dist = std::distance(m_buffer.begin(), m_iterator);
    auto currentCapacity = std::distance(m_iterator, m_buffer.end());
    if (currentCapacity < data.m_buffer.size()) {
      m_buffer.resize(m_buffer.size() + (data.m_buffer.size() - currentCapacity));
    }
    memcpy(&m_buffer[dist], &data.m_buffer[0], data.m_buffer.size());
    m_iterator = m_buffer.begin() + dist + data.m_buffer.size();
  }


  const std::vector<uint8_t> &getData() const { return m_buffer; }
  const std::size_t getSize() const { return m_buffer.size(); }
private:
  std::vector<uint8_t> m_buffer;
  std::vector<uint8_t>::iterator m_iterator;
};

template <typename T>
BinarySerializer& operator<<(BinarySerializer &ser, const std::vector<T> &data) {
  ser.operator<<<uint64_t>(data.size());
  for (const auto &it : data) {
    ser.operator<<<T>(it);
  }
  return ser;
}

template <typename T> void operator>>(BinarySerializer &ser, std::vector<T> &data) {
  std::size_t size = 0;
  ser.operator>><uint64_t>(size);
  data.resize(size);
  for (std::size_t i = 0; i < size; i++) {
    ser.operator>><T>(data[i]);
  }
}

BinarySerializer& operator<<(BinarySerializer& ser, const std::string& str) {
  ser.operator<<<uint64_t>(str.size());
  for (std::size_t i = 0; i < str.size(); i++) {
    ser << str[i];
  }
  return ser;
}

void operator>>(BinarySerializer& ser, std::string& str) {
  std::size_t size = 0;
  ser.operator>><uint64_t>(size);
  str.resize(size);
  for (std::size_t i = 0; i < str.size(); i++) {
    ser >> str[i];
  }
}

//===============================================================================

struct MyStruct {
  int x;
  float y;
  std::vector<int> data;
  bool operator==(const MyStruct &other) const {
    if (x != other.x)
      return false;
    if (abs(y - other.y) >= 1.0e-4)
      return false;
    if (data.size() != other.data.size())
      return false;
    for (unsigned int i = 0; i < data.size(); i++) {
      if (data[i] != other.data[i])
        return false;
    }
    return true;
  }
};

enum class PackageType : uint8_t {
  PT_MYSTRUCT = 1,
  PT_FILMFRAME = 2,
  PT_EVENTLOG = 4,
  PT_BAD = 8,
};


struct PackageHeader {
  uint64_t package_size{ 0 }; // data_size + len(chanel_name)
  uint64_t data_size{ 0 };
  std::string chanel_name{ "" }; //"name close by \n"
  PackageType package_type{ PackageType::PT_BAD };
};

struct EventMessage {
  std::string m_event;
  std::string m_message;
};
//===============================================================================

BinarySerializer& operator<<(BinarySerializer &ser, const MyStruct &data) {
  ser << data.x;
  ser << data.y;
  ser << data.data;
  return ser;
}
void operator>>(BinarySerializer& ser, MyStruct& data) {
  ser >> data.x;
  ser >> data.y;
  ser >> data.data;
}


BinarySerializer& operator<<(BinarySerializer& ser, const PackageHeader& header) {
  ser << header.package_size;
  ser << header.data_size;
  ser << header.chanel_name;
  ser << header.package_type;
  return ser;
}

void operator>>(BinarySerializer& ser, PackageHeader& header) {
  ser >> header.package_size;
  ser >> header.data_size;
  ser >> header.chanel_name;
  ser >> header.package_type;
}

BinarySerializer& operator<<(BinarySerializer& ser, const EventMessage& event) {
  ser << event.m_event << event.m_message;
  return ser;
}

void operator>>(BinarySerializer& ser, EventMessage& event) {
  ser >> event.m_event;
  ser >> event.m_message;
}

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

//===============================================================================
template<typename package, PackageType type>
 SendResult generatePackage(const std::string &chanel,
                                     const package &data, std::vector<uint8_t>& outData) {
  constexpr std::size_t header_const_size = 8 + 8 + chanel.size() + sizeof(PackageType);
  BinarySerializer ser1;
  try {
    ser1 << data;
    PackageHeader header{ ser1.getSize() + header_const_size, ser1.getSize(), chanel, type };
    BinarySerializer finalSer;
    finalSer << header << ser1;
    outData = finalSer.getData();
  }
  catch (std::exception& err) {
    std::cerr << err.what() << std::endl;
    return SendResult::SR_SERIALIZATION_FAIL;
  }
  return SendResult::SR_SERIALIZATION_OK;
}
//===============================================================================
 CeiveResult getHeader(const char *data, unsigned int dataSize, PackageHeader& header) {
  std::vector<uint8_t> buffer(dataSize);
  memcpy(&buffer[0], data, dataSize);
  BinarySerializer ser(std::move(buffer));
  try {
    ser >> header;
  }
  catch (std::runtime_error& error) {
    std::cerr << error.what() << std::endl;
    return CeiveResult::CR_SERIALIZATION_FAIL;
  }
  return CeiveResult::CR_SERIALIZATION_OK;
}
//===============================================================================
template<typename T, PackageType type>
int getDataFromPackage(const char* data, unsigned int dataSize, T& outData) {
  PackageHeader header;
  auto result = getHeader(data, dataSize, header);
  if (result == CeiveResult::CR_SERIALIZATION_OK) {
    if (header.data_size != 0 && header.data_size <= dataSize) {
      std::vector<uint8_t> buffer(dataSize);
      memcpy(&buffer[0], data, dataSize);
      BinarySerializer ser(std::move(buffer));
      try {
        ser >> outData;
      }
      catch (std::runtime_error& err) {
        std::cerr << err.what() << std::end;
        return CeiveResult::CR_SERIALIZATION_FAIL;
      }
      return CeiveResult::CR_SERIALIZATION_OK;
    }
  }
  return CeiveResult::CR_SERIALIZATION_FAIL;
}



class Pipe {
public:
  Pipe(Socket &socket) : m_socket(socket) {}

  SendResult PutEventMessage(const std::string &chanel, const std::string &event,
                      const std::string &message) {
    EventMessage data{ event, message };
    std::vector<uint8_t> serializedData;
    generatePackage<EventMessage, PackageType::PT_EVENTLOG>(chanel, data, serializedData);
    auto result = m_socket.Send(reinterpret_cast<char*>(serializedData.data()), serializedData.size());
    if (result == -1) {
      std::cerr << "Send event was failed.\n";
      return SendResult::SR_FAIL;
    }
    return SendResult::SR_FAIL;
  }

  SendResult PutMyStruct(const std::string &chanel, const MyStruct &data) { 
    std::vector<uint8_t> serializedData;
    generatePackage<MyStruct, PackageType::PT_MYSTRUCT>(chanel, data, serializedData);
    auto result = m_socket.Send(reinterpret_cast<char*>(serializedData.data()), serializedData.size());
    if (result == -1) {
      std::cerr << "Send mystruct was failed.\n";
      return SendResult::SR_FAIL;
    }
    return SendResult::SR_FAIL;
  }

  CeiveResult GetEventMessage(const std::string& chanel, std::string& outEvent,
                      std::string &outMessage) {
    const unsigned int maxHeaderSize = 120;
    std::vector<uint8_t> inData(maxHeaderSize);
    m_socket.ReceiveWithoutPop(reinterpret_cast<char*>(&inData.at(0)), maxHeaderSize);
    PackageHeader header;
    auto result = getHeader(reinterpret_cast<char*>(inData.data()), maxHeaderSize, header);
    if (result == CeiveResult::CR_SERIALIZATION_FAIL) {
      std::cerr << "Package was currupted.\n";
      return CeiveResult::CR_FAIL;
    }
    assert(header.)
    return CeiveResult::CR_OK;
  }

  CeiveResult GetMyStruct(const std::string &chanel, MyStruct &data) { 
    return CeiveResult::CR_FAIL; 
  }

private:
  Socket &m_socket;
};

int main(int arg, char **argv) {
  auto server = std::make_unique<Socket>("localhost", SocketType::server);
  auto client = std::make_unique<Socket>("localhost", SocketType::client);
  std::thread startServer([Server = server.get()]() { Server->Start(); });
  client->Start();
  startServer.join();
  Pipe pipeWriter(*server.get());
  Pipe pipeReader(*client.get());
  MyStruct data1, data2;
  data1.x = 12;
  data1.y = 3.144f;
  data1.data = {1, 2, 3, 4};
  data2.x = 100;
  data2.y = 9.8f;
  data2.data = {11, 22, 33, 44};

  pipeWriter.PutEventMessage("A", "event1", "message1");
  pipeWriter.PutEventMessage("A", "event2", "message2");
  pipeWriter.PutEventMessage("B", "event3", "message3");
  pipeWriter.PutEventMessage("B", "event4", "message4");
  pipeWriter.PutMyStruct("chanel1", data1);
  pipeWriter.PutMyStruct("chanel2", data2);

  MyStruct data3, data4;
  std::string evnt, message;
  pipeReader.GetEventMessage("A", evnt, message);
  assert(evnt == "event1" && message == "message1");
  pipeReader.GetEventMessage("B", evnt, message);
  assert(evnt == "event3" && message == "message3");
  pipeReader.GetMyStruct("chanel2", data3);
  assert(data3 == data2);
  pipeReader.GetEventMessage("A", evnt, message);
  assert(evnt == "event2" && message == "message2");
  pipeReader.GetEventMessage("B", evnt, message);
  assert(evnt == "event4" && message == "message3");
  pipeReader.GetMyStruct("chanel1", data4);
  assert(data4 == data1);
  return 0;
}
