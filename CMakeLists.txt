cmake_minimum_required(VERSION 2.8)
project(OssFileUploader)

add_definitions(-fpermissive -fPIC -g)
#set(CMAKE_EXE_LINKER_FLAGS "-static")

include_directories(include thirdLib/include)
aux_source_directory(src DEMO_SRCS)

link_libraries(ssl crypto pthread)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdLib/lib/linux)

add_executable(OssFileUploader ${DEMO_SRCS})
