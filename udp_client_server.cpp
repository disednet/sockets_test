#include "udp_client_server.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
//#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")

Socket::Socket(const std::string &addr, SocketType type) : m_type(type) {
  auto pos = addr.find(":");
  if (pos == std::string::npos) {
    m_addr = addr;
    m_port = "8086";
  } else {
    m_addr = addr.substr(0, pos);
    m_port = addr.substr(pos + 1, addr.length() - pos - 1);
  }
}

Socket::~Socket() {
  ShutDown();
  CloseSocketAndCleanUp(m_socket);
}

int Socket::Start() {
  return m_type == SocketType::client ? InitClient() : InitServer();
}

int Socket::ShutDown() {
  auto iResult = shutdown(m_socket, SD_SEND);
  if (iResult == SOCKET_ERROR) {
    std::cerr << "shutdown failed with code: " << WSAGetLastError()
              << std::endl;
    CloseSocketAndCleanUp(m_socket);
    return 1;
  }
  std::cout << "Shutdown client socket." << std::endl;
  return 0;
}

int Socket::InitServer() {
  std::cout << "Server start.....\n";
  WholeStart();
  // start structs
  struct addrinfo *result = nullptr;
  struct addrinfo *ptr = nullptr;
  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;
  // translate ansi host name to an address
  auto iResult = getaddrinfo(m_addr.c_str(), m_port.c_str(), &hints, &result);
  if (iResult != 0) {
    std::cerr << "getaddrinfo failed with code: " << iResult << std::endl;
    WSACleanup();
    return 1;
  }

  // create socket
  s_socket = INVALID_SOCKET;
  s_socket =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (s_socket == INVALID_SOCKET) {
    std::cerr << "Error at socket()" << WSAGetLastError() << std::endl;
    freeaddrinfo(result);
    WSACleanup();
    return 1;
  }
  std::cout << "Listen socket was created." << std::endl;
  // binding to socket
  iResult = bind(s_socket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
    freeaddrinfo(result);
    CloseSocketAndCleanUp(s_socket);
    return 1;
  }
  std::cout << "Listen socket was binded to " << result->ai_addr->sa_data
            << std::endl;
  freeaddrinfo(result);
  // listeng on a socket
  if (listen(s_socket, SOMAXCONN) == SOCKET_ERROR) {
    std::cerr << "Listen failed with error code: " << WSAGetLastError()
              << std::endl;
    CloseSocketAndCleanUp(s_socket);
    return 1;
  }
  std::cout << "Listen start." << std::endl;
  // accapting a connection
  // first of all create client socket
  m_socket = INVALID_SOCKET;
  m_socket = accept(s_socket, nullptr, nullptr);
  if (m_socket == INVALID_SOCKET) {
    std::cerr << "accept failed with code: " << WSAGetLastError() << std::endl;
    CloseSocketAndCleanUp(s_socket);
    return 1;
  }
  std::cout << "Client socket was accepted" << std::endl;
  // now we can close server socket
  // closesocket(s_socket);
  return 0;
}

int Socket::InitClient() {
  WholeStart();
  // start structs
  struct addrinfo *result = nullptr;
  struct addrinfo *ptr = nullptr;
  struct addrinfo hints;
  int iResult;
  m_socket = INVALID_SOCKET;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  // translate ansi host name to an address

  iResult = getaddrinfo(m_addr.c_str(), m_port.c_str(), &hints, &result);
  if (iResult != 0) {
    std::cerr << "getaddrinfo failed with code: " << iResult << std::endl;
    WSACleanup();
    return 1;
  }
  // create socket
  ptr = result;
  for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
    m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (m_socket == INVALID_SOCKET) {
      std::cerr << "Error at socket() : " << WSAGetLastError() << std::endl;
      freeaddrinfo(result);
      WSACleanup();
      return 1;
    }
    // connect to server
    iResult = connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
      closesocket(m_socket);
      m_socket = INVALID_SOCKET;
      continue;
    }
    break;
  }
  freeaddrinfo(result);
  if (m_socket == INVALID_SOCKET) {
    std::cerr << "Unable to connect to server!" << std::endl;
    WSACleanup();
    return 1;
  }
  return 0;
}

int Socket::WholeStart() {
  std::cout << "Start init net dll's...\n";
  WSADATA wsaData;
  int iResult;
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    std::cerr << "WSAStartup failed: " << iResult << std::endl;
    return 1;
  }
  std::cout << "Init net dll's finished.\n";
  return 0;
}

void Socket::CloseSocketAndCleanUp(SOCKET socket) {
  closesocket(socket);
  WSACleanup();
}

int Socket::Send(const char *data, unsigned int dataSize) {
  assert(m_socket != INVALID_SOCKET);
  auto iResult = send(m_socket, data, dataSize, 0);
  if (iResult == SOCKET_ERROR) {
    std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
    CloseSocketAndCleanUp(m_socket);
    return 1;
  }
  return 0;
}

int Socket::Receive(char *data, unsigned int dataSize, int flag) {
  assert(m_socket != INVALID_SOCKET);
  auto iResult = recv(m_socket, data, dataSize, flag);
  if (iResult > 0) {
    return iResult;
  } else if (iResult == 0)
    std::cout << "connection closed \n";
  else {
    std::cerr << "receiv failed with code: " << WSAGetLastError() << std::endl;
    CloseSocketAndCleanUp(m_socket);
    return 1;
  }
  return 0;
}

int Socket::ReceiveWithoutPop(char *data, unsigned int dataSize) {
  return Receive(data, dataSize, MSG_PEEK);
}

int Socket::RecieveAll(char* data, unsigned int dataSize)
{
  assert(m_socket != INVALID_SOCKET);
  unsigned int total = 0;
  while (total < dataSize) {
    auto result = recv(m_socket, data + total, dataSize - total, 0);
    if (result > 0) {
      total += result;
    }
    else if (result == 0)
      std::cout << "connection closed \n";
    else {
      std::cerr << "receiv failed with code: " << WSAGetLastError() << std::endl;
      CloseSocketAndCleanUp(m_socket);
      return 1;
    }
  }
  return 0;
}
