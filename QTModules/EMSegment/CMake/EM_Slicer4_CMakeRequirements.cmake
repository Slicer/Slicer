#
# Slicer4 specific CMake requirements
#

  cmake_minimum_required(VERSION 2.8.2)

  if(POLICY CMP0016)
    CMAKE_POLICY(SET CMP0016 NEW)
  endif()
  if(POLICY CMP0017)
    CMAKE_POLICY(SET CMP0017 OLD)
  endif()
  
