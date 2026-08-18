cmake_minimum_required(VERSION 3.14)
project(ATMSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(atm_system
    src/main.cpp
    src/transaction.cpp
    src/card.cpp
    src/account.cpp
    src/customer.cpp
    src/bank.cpp
    src/atm.cpp
)
