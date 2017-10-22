set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -o2 -L/usr/local/lib -lcurl -pthread")

set(
	src_list main.cpp
		momi.cpp
)

add_executable(${PROJECT_NAME} ${src_list})

