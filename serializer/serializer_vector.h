#pragma once
#include "serializer.h"
#include <vector>

template <typename T>
BinarySerializer &operator<<(BinarySerializer &ser,
                             const std::vector<T> &data) {
  ser.operator<<<uint64_t>(data.size());
  for (const auto &it : data) {
    ser.operator<<<T>(it);
  }
  return ser;
}

template <typename T>
void operator>>(BinarySerializer &ser, std::vector<T> &data) {
  std::size_t size = 0;
  ser.operator>><uint64_t>(size);
  data.resize(size);
  for (std::size_t i = 0; i < size; i++) {
    ser.operator>><T>(data[i]);
  }
}