cmake_minimum_required(VERSION 3.0)
project(SignalLang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include dirs
include_directories(SymbolTable ErrorHandler)

add_executable(SensorLang
    main.cpp
    symbolTable/symbolTable.cpp
)