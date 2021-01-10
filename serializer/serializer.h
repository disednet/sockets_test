#pragma once
#include <stdexcept>
#include <type_traits>
#include <vector>
class BinarySerializer {
public:
  BinarySerializer(std::vector<uint8_t> &&in) : m_buffer(std::move(in)) {
    m_iterator = m_buffer.begin();
  }
  BinarySerializer() { 
  }

  template <typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>>>
  void operator<<(const T &data) {
    if (m_buffer.empty()) {
      m_buffer.resize(sizeof(T));
      m_iterator = m_buffer.begin();
    }
    auto dist = std::distance(m_iterator, m_buffer.end());
    if (dist <= sizeof(T)) {
      auto pos = std::distance(m_buffer.begin(), m_iterator);
      m_buffer.resize(m_buffer.size() * 2 + (sizeof(T) - dist) * 5);
      m_iterator = m_buffer.begin() + pos;
    }
    *(reinterpret_cast<T *>(&(*m_iterator))) = data;
    m_iterator += sizeof(T);
    m_citerator = m_iterator;
  }

  template <typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>>>
  void operator>>(T &data) {
    if (std::distance(m_iterator, m_buffer.end()) >= sizeof(T)) {
      data = *(reinterpret_cast<T *>(&(*m_iterator)));
      m_iterator += sizeof(T);
      m_citerator = m_iterator;
    } else {
      throw std::runtime_error("out of data range");
    }
  }
  
  void operator<<(const BinarySerializer &data) {
    auto dist = std::distance(m_buffer.begin(), m_iterator);
    auto currentCapacity = std::distance(m_iterator, m_buffer.end());
    auto otherSize = data.getSize();
    if (currentCapacity < otherSize) {
      m_buffer.resize(m_buffer.size() +
                      (otherSize - currentCapacity)*2);
    }
    memcpy(&m_buffer[dist], &data.m_buffer[0], otherSize);
    m_iterator = m_buffer.begin() + dist + otherSize;
    m_citerator = m_iterator;
  }
  const char* getData() const { return reinterpret_cast<const char*>(m_buffer.data()); }
  std::size_t getSize() const { return std::distance(m_buffer.cbegin(), m_citerator); }

private:
  std::vector<uint8_t> m_buffer;
  std::vector<uint8_t>::iterator m_iterator;
  std::vector<uint8_t>::const_iterator m_citerator;
};

