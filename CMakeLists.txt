cmake_minimum_required(VERSION 3.10.2)
project(casino)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)
include_directories(/home/nikita/tgbot-cpp/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif ()

add_executable(casino main.cpp
        Person.cpp
        Person.h
        Time.h
        Time.cpp
        Color.h
        concurrent_queue.h
        GameEventHandler.h
        GameEventHandler.cpp
        PersonEventHandler.cpp
        PersonEventHandler.h
        Game.cpp
        Game.h
        Globals.h
        Color.cpp
        PersonManager.cpp
        PersonManager.h
        GameManager.cpp
        GameManager.h
        Globals.cpp
        rand.cpp
        rand.h
        Names.cpp
        Names.h
        Chat.cpp
        Chat.h
)

target_link_libraries(casino /home/nikita/tgbot-cpp/libTgBot.a ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${CURL_LIBRARIES})