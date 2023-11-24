project(wolf-and-goats)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(host client conn utils)
FILE(GLOB HOST host/*.cpp)

add_executable(${PROJECT_NAME}-mq ${HOST} conn/conn_mq.cpp)
target_link_libraries(${PROJECT_NAME}-mq Threads::Threads rt)

add_executable(${PROJECT_NAME}-shm ${HOST} conn/conn_shm.cpp)
target_link_libraries(${PROJECT_NAME}-shm Threads::Threads rt)

add_executable(${PROJECT_NAME}-pipe ${HOST} conn/conn_pipe.cpp)
target_link_libraries(${PROJECT_NAME}-pipe Threads::Threads rt)