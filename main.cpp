#include "pipe.h"
#include "udp_client_server.h"

#include <iostream>
#include <string>
#include <thread>
#include <vector>





int main(int arg, char **argv) {
  auto server = std::make_unique<Socket>("localhost", SocketType::server);
  auto client = std::make_unique<Socket>("localhost", SocketType::client);
  std::thread startServer([Server = server.get()]() { Server->Start(); });
  client->Start();
  startServer.join();
  Pipe pipeWriter(*server.get());
  Pipe pipeReader(*client.get());
  MyStruct data1, data2;
  data1.x = 12;
  data1.y = 3.144f;
  data1.data = {1, 2, 3, 4};
  data2.x = 100;
  data2.y = 9.8f;
  data2.data = {11, 22, 33, 44};

  pipeWriter.PutEventMessage("A", "event1", "message1");
  pipeWriter.PutEventMessage("A", "event2", "message2");
  pipeWriter.PutEventMessage("B", "event3", "message3");
  pipeWriter.PutEventMessage("B", "event4", "message4");
  pipeWriter.PutMyStruct("chanel1", data1);
  pipeWriter.PutMyStruct("chanel2", data2);

  MyStruct data3, data4;
  std::string evnt, message;
  pipeReader.GetEventMessage("A", evnt, message);
  assert(evnt == "event1" && message == "message1");
  pipeReader.GetEventMessage("B", evnt, message);
  assert(evnt == "event3" && message == "message3");
  pipeReader.GetMyStruct("chanel2", data3);
  assert(data3 == data2);
  pipeReader.GetEventMessage("A", evnt, message);
  assert(evnt == "event2" && message == "message2");
  pipeReader.GetEventMessage("B", evnt, message);
  assert(evnt == "event4" && message == "message3");
  pipeReader.GetMyStruct("chanel1", data4);
  assert(data4 == data1);
  return 0;
}
