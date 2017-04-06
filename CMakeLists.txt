cmake_minimum_required(VERSION 3.7.2)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 14) # C++14...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

#
# Config options
#
option(BOOST_ASIO_SSL_FILE_DL_TEST_USE_LIBRE_SSL "Use LibreSSL instead of OpenSSL" TRUE)

#
# Set our project name
#
project(boost_asio_ssl_file_dl_test)

#
# Source files
#
set(boost_asio_ssl_file_dl_test_src
    boost_asio_ssl_file_dl_test/Source.cpp
    boost_asio_ssl_file_dl_test/downloader.cpp
    boost_asio_ssl_file_dl_test/downloader_impl.cpp
)

#
# find libraries and include
#

# find thread library
find_package(Threads REQUIRED)

# find Boost
# https://cmake.org/cmake/help/v3.7/module/FindBoost.html
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_DEBUG_RUNTIME OFF)
set(Boost_DEBUG ON)
add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
set(boost_required_components
    system date_time regex zlib bzip2 iostreams
    # thread
)
find_package(Boost 1.59 REQUIRED COMPONENTS ${boost_required_components})
if(NOT Boost_FOUND)
#   Config options
    set(BOOST_ROOT ${BOOST_ROOT} CACHE PATH "Set boost root directory" FORCE)
    set(BOOST_LIBRARYDIR ${BOOST_LIBRARYDIR} CACHE PATH "Set boost library directory" FORCE)
    message(FATAL_ERROR "Fail to find Boost")
endif()
include_directories(${Boost_INCLUDE_DIRS})

# find SSL Library
if(BOOST_ASIO_SSL_FILE_DL_TEST_USE_LIBRE_SSL)
#   find LibreSSL
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
    include(FindLibreSSL)
    find_package(LibreSSL)
    if(NOT LIBRESSL_FOUND)
        message(FATAL_ERROR "Fail to find LibreSSL") # exit
    endif()
    message(STATUS "LibreSSL_INCLUDE_DIR: ${LibreSSL_INCLUDE_DIR}")
    include_directories(${LibreSSL_INCLUDE_DIR})
else()
#   find OpenSSL
#   https://cmake.org/cmake/help/v3.7/module/FindOpenSSL.html
    find_package(OpenSSL REQUIRED)
    if(NOT OPENSSL_FOUND)
        message(FATAL_ERROR "Fail to find OpenSSL") # exit
    endif()
    message(STATUS "OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

#
# Update compiler waring flags
#
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic")
endif()

#
# Compile
#
add_executable(boost_asio_ssl_file_dl_test ${boost_asio_ssl_file_dl_test_src})

#
# Link
#
target_link_libraries(boost_asio_ssl_file_dl_test Threads::Threads)
target_link_libraries(boost_asio_ssl_file_dl_test Boost::disable_autolinking ${Boost_LIBRARIES})
if(BOOST_ASIO_SSL_FILE_DL_TEST_USE_LIBRE_SSL)
    target_link_libraries(boost_asio_ssl_file_dl_test ${LibreSSL_LIBRARIES})
else()
    target_link_libraries(boost_asio_ssl_file_dl_test OpenSSL::SSL)
    target_link_libraries(boost_asio_ssl_file_dl_test OpenSSL::Crypto)
endif()
