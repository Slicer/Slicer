
#
# DiscoverSystemNameAndBits - Set the variable Slicer_BUILD and Slicer_BUILD_BITS
#

# Slicer_BUILD can take on of the following value:
#   solaris8, linux-x86, linux-x86_64, darwin-ppc, darwin-x86, darwin-x86_64, win32, win64


SET(Slicer_BUILD "")

SET(Slicer_BUILD_BITS "32")
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(Slicer_BUILD_BITS "64")
ENDIF()

IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")

  SET(Slicer_BUILD "win${Slicer_BUILD_BITS}")
  
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")

  SET(Slicer_BUILD "linux-x86")
  IF (Slicer_BUILD_BITS STREQUAL "64")
    SET(Slicer_BUILD "linux-x86_64")
  ENDIF()
  
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

  # we do not differentiate 32 vs 64 for mac - all are 64 bit.
  IF(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
    SET(Slicer_BUILD "darwin-ppc")
  ELSE(CMAKE_SYSTEM_PROCESSOR MATCHES "powerpc")
    SET(Slicer_BUILD "darwin-x86")
  ENDIF()
  
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Solaris")

  SET(Slicer_BUILD "solaris8") # What about solaris9 and solaris10 ?
  
ENDIF()
