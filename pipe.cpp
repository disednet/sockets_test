#include "pipe.h"


//===============================================================================
template<typename package, PackageType type>
SendResult generatePackage(const std::string& chanel,
  const package& data, std::vector<uint8_t>& outData) {
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
CeiveResult getHeader(const char* data, unsigned int dataSize, PackageHeader& header) {
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

SendResult Pipe::PutEventMessage(const std::string& chanel, const std::string& event, const std::string& message) {
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

SendResult Pipe::PutMyStruct(const std::string& chanel, const MyStruct& data) {
  std::vector<uint8_t> serializedData;
  generatePackage<MyStruct, PackageType::PT_MYSTRUCT>(chanel, data, serializedData);
  auto result = m_socket.Send(reinterpret_cast<char*>(serializedData.data()), serializedData.size());
  if (result == -1) {
    std::cerr << "Send mystruct was failed.\n";
    return SendResult::SR_FAIL;
  }
  return SendResult::SR_FAIL;
}

CeiveResult Pipe::GetEventMessage(const std::string& chanel, std::string& outEvent, std::string& outMessage) {
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

CeiveResult Pipe::GetMyStruct(const std::string& chanel, MyStruct& data) {
  return CeiveResult::CR_FAIL;
}
