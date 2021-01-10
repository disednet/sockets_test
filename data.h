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

struct PackageSize {
  uint64_t wholeSize{0};
  uint64_t dataSize{0};
};

struct PackageHeader {
  PackageSize package_size;
  std::string chanel_name{ "" }; //"name close by \n"
  PackageType package_type{ PackageType::PT_BAD };
};



struct EventMessage {
  std::string m_event;
  std::string m_message;
};
//===============================================================================
BinarySerializer& operator<<(BinarySerializer& ser, const MyStruct& data);
void operator>>(BinarySerializer& ser, MyStruct& data);
BinarySerializer& operator<<(BinarySerializer& ser, const PackageSize& size);
void operator>>(BinarySerializer& ser, PackageSize& size);
BinarySerializer& operator<<(BinarySerializer& ser, const PackageHeader& header);
void operator>>(BinarySerializer& ser, PackageHeader& header);
BinarySerializer& operator<<(BinarySerializer& ser, const EventMessage& event);
void operator>>(BinarySerializer& ser, EventMessage& event);