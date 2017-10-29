set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -o2 -L/usr/local/lib -lcurl -lpthread")


set(
	src_list main.cpp
		momi.cpp
		loader.cpp
		saver.cpp
)

add_executable(${PROJECT_NAME} ${src_list})

