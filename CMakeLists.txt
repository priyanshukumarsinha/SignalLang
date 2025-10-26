cmake_minimum_required(VERSION 3.10)
project(SignalLang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include dirs
include_directories(Tests ErrorHandler)

add_executable(SensorLang
    main.cpp
    Tests/errorHandlerTest.cpp
    errorHandler/errorHandler.cpp
)