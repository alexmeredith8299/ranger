<<<<<<< HEAD:cpp_version/bin/cmake_install.cmake
# Install script for directory: /home/alex/ranger/cpp_version
=======
# Install script for directory: /home/pi/ranger/cpp_version
>>>>>>> d18733621b7bc4e1a57c01aab06ba7efa9c5c1bf:cpp_version/bin-arm-static/cmake_install.cmake

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

<<<<<<< HEAD:cpp_version/bin/cmake_install.cmake
# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

=======
>>>>>>> d18733621b7bc4e1a57c01aab06ba7efa9c5c1bf:cpp_version/bin-arm-static/cmake_install.cmake
if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
<<<<<<< HEAD:cpp_version/bin/cmake_install.cmake
file(WRITE "/home/alex/ranger/cpp_version/bin/${CMAKE_INSTALL_MANIFEST}"
=======
file(WRITE "/home/pi/ranger/cpp_version/bin-arm-static/${CMAKE_INSTALL_MANIFEST}"
>>>>>>> d18733621b7bc4e1a57c01aab06ba7efa9c5c1bf:cpp_version/bin-arm-static/cmake_install.cmake
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
