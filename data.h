#pragma once
#include "serializer/serializer.h"
#include "serializer/serializer_string.h"
#include "serializer/serializer_vector.h"
#include <string>
#include <vector>

//===============================================================================
struct MyStruct {
  int x;
  float y;
  std::vector<int> data;
  bool operator==(const MyStruct& other) const {
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

BinarySerializer& operator<<(BinarySerializer& ser, const MyStruct& data) {
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