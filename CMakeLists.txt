cmake_minimum_required(VERSION 3.10.0)
project(logger VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(main main.cc
                    logger.h
                    file.h
                    utils.h)               

