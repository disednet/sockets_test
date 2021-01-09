#pragma once
#include "serializer.h"
#include <string>

BinarySerializer& operator<<(BinarySerializer& ser, const std::string& str);
void operator>>(BinarySerializer& ser, std::string& str);
