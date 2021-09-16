################################################################################
# Copyright (c) 2019-2021 Vladislav Trifochkin
#
# This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
#
# Changelog:
#      2021.09.10 Initial version.
################################################################################
add_executable(network_discoverer main.cpp network_discoverer.cpp)
# target_include_directories(network_discoverer PRIVATE
#     ${CMAKE_SOURCE_DIR}/3rdparty/libzmq/include
#     ${CMAKE_SOURCE_DIR}/3rdparty/cppzmq)
target_link_libraries(network_discoverer pfs::io pfs::net)

if (CEREAL_ENABLED)
    target_link_libraries(network_discoverer cereal)
    target_compile_definitions(network_discoverer PRIVATE "-DCEREAL_ENABLED=1")
endif(CEREAL_ENABLED)

if (QT5_CORE_ENABLED)
    target_link_libraries(network_discoverer Qt5::Core)
    target_compile_definitions(network_discoverer PRIVATE "-DQT5_CORE_ENABLED=1")
endif(QT5_CORE_ENABLED)

if (QT5_NETWORK_ENABLED)
    target_sources(network_discoverer PRIVATE network_discoverer_qt5.cpp)
    target_link_libraries(network_discoverer Qt5::Network)
    target_compile_definitions(network_discoverer PRIVATE "-DQT5_NETWORK_ENABLED=1")
endif(QT5_NETWORK_ENABLED)
