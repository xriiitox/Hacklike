cmake_minimum_required(VERSION 3.29)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

add_executable(untitled
        main.cpp
        Actor.hpp
        Actor.cpp
        Ai.cpp
        Ai.hpp
        Map.cpp
        Map.hpp
        Engine.cpp
        Engine.hpp
        main.hpp
        Destructible.cpp
        Destructible.hpp
        Attacker.cpp
        Attacker.hpp
        Ai.cpp
        Ai.hpp
        Gui.cpp
        Gui.hpp
        Container.cpp
        Container.hpp
        Pickable.cpp
        Pickable.hpp
        Persistent.hpp
)

file(COPY Alloy_curses_12x12-export.png DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY matrix.png DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")


find_package(libtcod CONFIG REQUIRED)
target_link_libraries(untitled PRIVATE libtcod::libtcod )