#pragma once
#include <string>
#include <vector>
#include <WinSock2.h>
enum class SocketType {
  client,
  server
};
struct DataType {
  std::string chanel;
  std::vector<uint8_t> data;
};

class Socket {
public:
  Socket(const std::string &addr, SocketType type);
  ~Socket();
  int Start();
  int ShutDown();
  int Send(const char* data, unsigned int dataSize);
  int Receive(char* data, unsigned int dataSize, int flag = 0);
  int ReceiveWithoutPop(char* data, unsigned int dataSize);
  int RecieveAll(char* data, unsigned int dataSize);
private:
  int InitServer();
  int InitClient();
  int WholeStart();
  void CloseSocketAndCleanUp(SOCKET socket);
private:
  std::string m_addr;
  std::string m_port;
  SocketType m_type;
  SOCKET m_socket{INVALID_SOCKET};
  SOCKET s_socket{ INVALID_SOCKET };
};