#include "pipe.h"
#include "udp_client_server.h"
#include <exception>
#include <iostream>
#include <cassert>


namespace {
  //===============================================================================
  template<typename package, PackageType type>
  SendResult generatePackage(const std::string& chanel,
    const package& data, std::vector<uint8_t>& outData) {
    BinarySerializer ser1;
    BinarySerializer headerSer;
    try {
      PackageHeader h2;
      headerSer << h2;
      ser1 << data;
      PackageHeader header{ PackageSize{ser1.getSize() + headerSer.getSize(), ser1.getSize()}, chanel, type };
      BinarySerializer finalSer;
      finalSer << header;
      finalSer << ser1;
      auto sz = finalSer.getSize();
      outData.resize(sz);
      for (std::size_t i = 0; i < sz; i++)
        outData[i] = static_cast<uint8_t>(finalSer.getData()[i]);
    }
    catch (std::exception& err) {
      std::cerr << err.what() << std::endl;
      return SendResult::SR_SERIALIZATION_FAIL;
    }
    return SendResult::SR_SERIALIZATION_OK;
  }

  //===============================================================================
  template<typename ElementType>
  CeiveResult getElement(const char* data, unsigned int dataSize, ElementType& element, unsigned int offset = 0) {
    std::vector<uint8_t> buffer(dataSize - offset);
    memcpy(&buffer[0], data + offset, dataSize - offset);
    BinarySerializer ser(std::move(buffer));
    try {
      ser >> element;
    }
    catch (std::runtime_error& error) {
      std::cerr << error.what() << std::endl;
      return CeiveResult::CR_SERIALIZATION_FAIL;
    }
    return CeiveResult::CR_SERIALIZATION_OK;
  }

  //===============================================================================
  template<typename T, PackageType type>
  CeiveResult getDataFromPackage(const char* data, unsigned int dataSize, T& outData) {
    PackageHeader header;
    auto result = getElement(data, dataSize, header);
    if (result == CeiveResult::CR_SERIALIZATION_OK) {
      if (header.package_size.dataSize != 0 && header.package_size.dataSize <= dataSize) {
        std::vector<uint8_t> buffer(header.package_size.dataSize);
        const auto dataOffset = header.package_size.wholeSize - header.package_size.dataSize;
        memcpy(&buffer[0], data+dataOffset, header.package_size.dataSize);
        BinarySerializer ser(std::move(buffer));
        try {
          ser >> outData;
        }
        catch (std::runtime_error& err) {
          std::cerr << err.what() << std::endl;
          return CeiveResult::CR_SERIALIZATION_FAIL;
        }
        return CeiveResult::CR_SERIALIZATION_OK;
      }
    }
    return CeiveResult::CR_SERIALIZATION_FAIL;
  }
}

//==========================================================================
//
//
//
//==========================================================================
SendResult Pipe::PutEventMessage(const std::string& chanel, const std::string& event, const std::string& message) {
  EventMessage data{ event, message };
  std::vector<uint8_t> serializedData;
  generatePackage<EventMessage, PackageType::PT_EVENTLOG>(chanel, data, serializedData);
  auto result = m_socket.Send(reinterpret_cast<char*>(serializedData.data()), serializedData.size());
  if (result == -1) {
    std::cerr << "Send event was failed.\n";
    return SendResult::SR_FAIL;
  }
  return SendResult::SR_OK;
}

//==========================================================================
SendResult Pipe::PutMyStruct(const std::string& chanel, const MyStruct& data) {
  std::vector<uint8_t> serializedData;
  generatePackage<MyStruct, PackageType::PT_MYSTRUCT>(chanel, data, serializedData);
  auto result = m_socket.Send(reinterpret_cast<char*>(serializedData.data()), serializedData.size());
  if (result == -1) {
    std::cerr << "Send mystruct was failed.\n";
    return SendResult::SR_FAIL;
  }
  return SendResult::SR_OK;
}

//==========================================================================
CeiveResult Pipe::GetEventMessage(const std::string& chanel, std::string& outEvent, std::string& outMessage) {
  UpdateData();
  if (m_messages.count(chanel) > 0) {
    if (m_messages[chanel].empty())
      return CeiveResult::CR_FAIL;
    auto item = m_messages[chanel].front();
    m_messages[chanel].pop_front();
    outEvent = item.m_event;
    outMessage = item.m_message;
    return CeiveResult::CR_OK;
  }
  return CeiveResult::CR_FAIL;
}

//==========================================================================
CeiveResult Pipe::GetMyStruct(const std::string& chanel, MyStruct& data) {
  UpdateData();
  if (m_mystructures.count(chanel) > 0) {
    if (m_mystructures[chanel].empty())
      return CeiveResult::CR_FAIL;
    data = m_mystructures[chanel].front();
    return CeiveResult::CR_OK;
  }
  return CeiveResult::CR_FAIL;
}


//==========================================================================
void Pipe::UpdateData() {
  const unsigned int maxHeaderSize = sizeof(PackageSize);
  std::vector<uint8_t> inData(maxHeaderSize);
  m_socket.ReceiveWithoutPop(reinterpret_cast<char*>(&inData.at(0)), maxHeaderSize);
  PackageSize sizeInfo;
  auto result = getElement(reinterpret_cast<char*>(inData.data()), maxHeaderSize, sizeInfo);
  if (result == CeiveResult::CR_SERIALIZATION_FAIL) {
    std::cerr << "Package was currupted.\n";
    return;
  }
  inData.resize(sizeInfo.wholeSize);
  auto ceivResult = m_socket.RecieveAll(reinterpret_cast<char*>(&inData.at(0)), sizeInfo.wholeSize);
  if (ceivResult != 0) {
    std::cerr << "Error was happened while reading package body.\n";
    return;
  }
  PackageHeader header;
  result = getElement(reinterpret_cast<char*>(inData.data()), sizeInfo.wholeSize - sizeInfo.dataSize, header);
  if (result == CeiveResult::CR_SERIALIZATION_FAIL || header.package_type == PackageType::PT_BAD) {
    std::cerr << "Package was currupted.\n";
    return;
  }
  if (header.package_type == PackageType::PT_MYSTRUCT) {
    MyStruct data;
    if (getDataFromPackage<MyStruct, PackageType::PT_MYSTRUCT>(reinterpret_cast<const char*>(inData.data()), inData.size(), data) == CeiveResult::CR_SERIALIZATION_OK) {
      m_mystructures[header.chanel_name].push_back(data);
    }
    else {
      std::cerr << "Can't converte data to MyStruct.\n";
      return;
    }
  }
  else if (header.package_type == PackageType::PT_EVENTLOG) {
    EventMessage data;
    if (getDataFromPackage<EventMessage, PackageType::PT_EVENTLOG>(reinterpret_cast<const char*>(inData.data()), inData.size(), data) == CeiveResult::CR_SERIALIZATION_OK) {
      m_messages[header.chanel_name].push_back(data);
    }
    else {
      std::cerr << "Can't converte data to EventMessage.\n";
      return;
    }
  }
}
