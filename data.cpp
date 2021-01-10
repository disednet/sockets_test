#include "data.h"
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

BinarySerializer& operator<<(BinarySerializer& ser, const PackageSize& sizeInfo) {
  ser << sizeInfo.wholeSize;
  ser << sizeInfo.dataSize;
  return ser;
}

void operator>>(BinarySerializer& ser, PackageSize& sizeInfo) {
  ser >> sizeInfo.wholeSize;
  ser >> sizeInfo.dataSize;
}

BinarySerializer& operator<<(BinarySerializer& ser, const PackageHeader& header) {
  ser << header.package_size;
  ser << header.chanel_name;
  ser << header.package_type;
  return ser;
}

void operator>>(BinarySerializer& ser, PackageHeader& header) {
  ser >> header.package_size;
  ser >> header.chanel_name;
  ser >> header.package_type;
}

BinarySerializer& operator<<(BinarySerializer& ser, const EventMessage& event) {
  ser << event.m_event;
  ser << event.m_message;
  return ser;
}

void operator>>(BinarySerializer& ser, EventMessage& event) {
  ser >> event.m_event;
  ser >> event.m_message;
}