cmake_minimum_required(VERSION 3.10)
project(SignalLang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include dirs (headers live in these folders)
include_directories(
    ${CMAKE_SOURCE_DIR}/errorHandler
    ${CMAKE_SOURCE_DIR}/symbolTable
    ${CMAKE_SOURCE_DIR}/Tests
    ${CMAKE_SOURCE_DIR}/lexer
    ${CMAKE_SOURCE_DIR}/parser
    ${CMAKE_SOURCE_DIR}/tac
)

add_executable(SensorLang
    main.cpp
    Tests/errorHandlerTest.cpp
    Tests/symbolTableTest.cpp
    errorHandler/errorHandler.cpp
    symbolTable/symbolTable.cpp     
    lexer/lexer.cpp     
    parser/parser.cpp   
    tac/tacGen.cpp
    tac/dce.cpp
)
