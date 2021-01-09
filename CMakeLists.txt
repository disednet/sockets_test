# CMakeList.txt : CMake project for udp_client_server, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("udp_client_server")
set(SRC 
udp_client_server.cpp
serializer/serializer.cpp
serializer/serializer_string.cpp
data.cpp
main.cpp 
pipe.cpp)

set(HDRS 
data.h
pipe.h
udp_client_server.h
serializer/serializer.h
serializer/serializer_string.h
serializer/serializer_vector.h
)
# Add source to this project's executable.
add_executable (udp_client_server ${SRC} ${HDRS})

# TODO: Add tests and install targets if needed.
