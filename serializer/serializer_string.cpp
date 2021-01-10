#include "serializer_string.h"

BinarySerializer& operator<<(BinarySerializer& ser, const std::string& str) {
  ser.operator<<<uint64_t>(static_cast<uint64_t>(str.size()));
  for (std::size_t i = 0; i < str.size(); i++) {
    ser << str[i];
  }
  return ser;
}

void operator>>(BinarySerializer& ser, std::string& str) {
  std::uint64_t size = 0;
  ser >> size;
  str.resize(size);
  for (std::size_t i = 0; i < str.size(); i++) {
    ser >> str[i];
  }
}
