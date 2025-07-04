cmake_minimum_required(VERSION 3.24)
project(stretch)
set(CMAKE_CXX_STANDARD 20)

add_library(signalsmith-stretch INTERFACE)
set_target_properties(signalsmith-stretch PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(
	signalsmith-linear
	GIT_REPOSITORY https://github.com/Signalsmith-Audio/linear.git
	GIT_TAG 0.1.2
	GIT_SHALLOW ON
)
FetchContent_MakeAvailable(signalsmith-linear)
FetchContent_Declare(
		Catch2
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG v3.8.1
		GIT_SHALLOW ON
)
FetchContent_MakeAvailable(Catch2)

target_link_libraries(signalsmith-stretch INTERFACE signalsmith-linear)

add_executable(stretch-tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp)
target_link_libraries(stretch-tests PRIVATE signalsmith-stretch Catch2::Catch2WithMain)
