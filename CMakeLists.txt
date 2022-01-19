cmake_minimum_required(VERSION 3.21)
project(asc)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(asc
        asc.cpp
        asc.h
        assembler.cpp
        assembler.h
        cli.cpp
        cli.h
        logger.cpp
        logger.h
        parser.cpp
        parser.h
        symbol.cpp
        symbol.h
        syntax.cpp
        syntax.h
        tokenizer.cpp
        tokenizer.h
        util.cpp
        util.h)
