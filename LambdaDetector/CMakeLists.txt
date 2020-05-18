# CMakeList.txt : CMake project for LambdaScourerCmake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost COMPONENTS regex)

include_directories(${Boost_INCLUDE_DIR}) 

message("Found boost: " ${Boost_FOUND})
message("Library Dirs: " ${Boost_LIBRARY_DIRS})
message("Include Dirs: " ${Boost_INCLUDE_DIRS})

link_directories(${Boost_LIBRARY_DIRS})

# Add source to this project's executable.
add_executable (LambdaDetector "LambdaDetector.cpp" "LambdaDetector.h" )
add_executable(LineCopy "LineCopy.cpp")
add_executable(Categorizer "Categorizer.cpp")
add_executable(CommentCounter "CommentCounter.cpp")

# TODO: Add tests and install targets if needed.
