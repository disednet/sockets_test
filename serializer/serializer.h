#pragma once
#include <stdexcept>
#include <type_traits>
#include <vector>
class BinarySerializer {
public:
  BinarySerializer(std::vector<uint8_t> &&in) : m_buffer(std::move(in)) {
    m_iterator = m_buffer.begin();
  }
  BinarySerializer() { m_iterator = m_buffer.begin(); }

  template <typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>>>
  void operator<<(const T &data) {
    if (std::distance(m_iterator, m_buffer.end()) <= sizeof(T))
      m_buffer.resize(m_buffer.size() * 2);
    *(reinterpret_cast<T *>(&(*m_iterator))) = data;
    m_iterator += sizeof(T);
  }

  template <typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>>>
  void operator>>(T &data) {
    if (std::distance(m_iterator, m_buffer.end()) > sizeof(T)) {
      data = *(reinterpret_cast<T *>((*m_iterator)));
      m_iterator += sizeof(T);
    } else {
      throw std::runtime_error("out of data range");
    }
  }
  
  void operator<<(const BinarySerializer &data) {
    auto dist = std::distance(m_buffer.begin(), m_iterator);
    auto currentCapacity = std::distance(m_iterator, m_buffer.end());
    if (currentCapacity < data.m_buffer.size()) {
      m_buffer.resize(m_buffer.size() +
                      (data.m_buffer.size() - currentCapacity));
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
/*
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void operator<<(BinarySerializer& sr, const T& data) {
  if (std::distance(m_iterator, m_buffer.end()) <= sizeof(T))
    m_buffer.resize(m_buffer.size() * 2);
  *(reinterpret_cast<T*>(&(*m_iterator))) = data;
  m_iterator += sizeof(T);
}

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void operator<<(T& data) {
  if (std::distance(m_iterator, m_buffer.end()) > sizeof(T)) {
    data = *(reinterpret_cast<T*>((*m_iterator)));
    m_iterator += sizeof(T);
  }
  else {
    throw std::runtime_error("out of data range");
  }
}*/
