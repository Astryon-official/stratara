cmake_minimum_required(VERSION 3.20)

project(calamares-stratara-shell
    VERSION 0.1.0
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Calamares
find_package(Calamares REQUIRED)

# Qt6 requirements
find_package(Qt6 REQUIRED COMPONENTS
    Core
    DBus
)

# Module definition
calamares_add_module( stratara-shell
    SOURCES
        main.cpp
    INSTALL
        stratara-shell.conf
)

# Link libraries
target_link_libraries( stratara-shell PRIVATE
    ${CALAMARES_LIBRARIES}
    ${Qt6_DBUS_LIBRARIES}
)

# Install module config
install( FILES stratara-shell.conf
    DESTINATION ${CALAMARES_MODULE_INSTALL_DIR}
)