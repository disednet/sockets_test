#include "serializer_string.h"

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
