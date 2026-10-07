cmake_minimum_required(VERSION 3.20)

project(Stratara
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Stratara Linux-native living-room shell"
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt 6 requirements
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    QuickTemplates2
    Svg
    Network
    DBus
    WaylandClient
)

# KDE Frameworks (optional - make them optional for now)
find_package(KF6 QUIET COMPONENTS
    CoreAddons
    I18n
    Config
    ConfigWidgets
    DBusAddons
    GlobalAccel
    WindowSystem
    Wayland
    Kirigami2
)

# System dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)
pkg_check_modules(GIO_UNIX2 REQUIRED gio-unix-2.0)
pkg_check_modules(LIBINPUT REQUIRED libinput)
pkg_check_modules(UDEV REQUIRED libudev)

# Optional dependencies
pkg_check_modules(PIPEWIRE pipewire-0.3)
pkg_check_modules(BLUEZ bluez)
pkg_check_modules(NETWORKMANAGER libnm)
pkg_check_modules(UPOWER upower-glib)
pkg_check_modules(SYSTEMD libsystemd)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-unused-parameter")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

# Qt6 tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(SRCS
    src/main.cpp
    src/core/Application.cpp
    src/core/Settings.cpp
    src/system/AppModel.cpp
    src/system/IconProvider.cpp
    src/system/FavoritesStore.cpp
    src/system/AppListCache.cpp
    src/system/UpdateStore.cpp
    src/system/UpdateChecker.cpp
    src/system/UpdateDownloader.cpp
    src/system/UpdateInstaller.cpp
    src/system/InputManager.cpp
    src/system/NetworkManager.cpp
    src/system/BluetoothManager.cpp
    src/system/PowerManager.cpp
    src/system/MediaManager.cpp
    src/system/PipeWireManager.cpp
    src/system/NotificationManager.cpp
    src/system/XodusManager.cpp
    src/ui/LauncherWindow.cpp
)

# Xodus service
set(XODUS_SRCS
    src/xodus/main.cpp
    src/xodus/XodusService.cpp
)

# Resources
qt_add_resources(STRATARA_RESOURCES resources.qrc)

# Main executable
add_executable(stratara ${SRCS} ${STRATARA_RESOURCES})

# Link libraries
target_link_libraries(stratara PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickTemplates2
    Qt6::Svg
    Qt6::Network
    Qt6::DBus
    Qt6::WaylandClient
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${GIO_UNIX2_LIBRARIES}
    ${LIBINPUT_LIBRARIES}
    ${UDEV_LIBRARIES}
)

if(KF6_CoreAddons_FOUND)
    target_link_libraries(stratara PRIVATE
        KF6::CoreAddons
        KF6::I18n
        KF6::Config
        KF6::ConfigWidgets
        KF6::DBusAddons
        KF6::GlobalAccel
        KF6::WindowSystem
        KF6::Wayland
        KF6::Kirigami2
    )
endif()

if(PIPEWIRE_FOUND)
    target_link_libraries(stratara PRIVATE ${PIPEWIRE_LIBRARIES})
    target_compile_definitions(stratara PRIVATE HAVE_PIPEWIRE)
endif()

if(BLUEZ_FOUND)
    target_link_libraries(stratara PRIVATE ${BLUEZ_LIBRARIES})
    target_compile_definitions(stratara PRIVATE HAVE_BLUEZ)
endif()

if(NETWORKMANAGER_FOUND)
    target_link_libraries(stratara PRIVATE ${NETWORKMANAGER_LIBRARIES})
    target_compile_definitions(stratara PRIVATE HAVE_NETWORKMANAGER)
endif()

if(UPOWER_FOUND)
    target_link_libraries(stratara PRIVATE ${UPOWER_LIBRARIES})
    target_compile_definitions(stratara PRIVATE HAVE_UPOWER)
endif()

if(SYSTEMD_FOUND)
    target_link_libraries(stratara PRIVATE ${SYSTEMD_LIBRARIES})
    target_compile_definitions(stratara PRIVATE HAVE_SYSTEMD)
endif()

# Xodus service executable
add_executable(xodus ${XODUS_SRCS})
target_link_libraries(xodus PRIVATE
    Qt6::Core
    Qt6::DBus
    ${SYSTEMD_LIBRARIES}
)
target_compile_definitions(xodus PRIVATE HAVE_SYSTEMD)

# Install
install(TARGETS stratara xodus DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES resources.qrc DESTINATION ${CMAKE_INSTALL_DATADIR}/stratara)
install(DIRECTORY qml/ DESTINATION ${CMAKE_INSTALL_DATADIR}/stratara/qml)
install(FILES assets/icons/stratara.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/ RENAME stratara.svg)

# Desktop file generation
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/stratara.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/stratara.desktop
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/stratara.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)