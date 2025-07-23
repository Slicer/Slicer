project(krb5-gssapi-stub C)

if(APPLE OR WIN32)
  return()
endif()

find_package(Qt5 COMPONENTS Network QUIET)
set(qt_target "Qt5::Network")

set(_msg "Checking if ${PROJECT_NAME} is needed")
message(STATUS "${_msg}")

if(NOT TARGET ${qt_target})
  message(STATUS "${_msg} - no (${qt_target} not available)")
  return()
endif()

get_target_property(_features ${qt_target} QT_ENABLED_PUBLIC_FEATURES)
if(NOT "gssapi" IN_LIST _features)
  message(STATUS "${_msg} - no (${qt_target} built without 'gssapi' feature)")
  return()
endif()

message(STATUS "${_msg} - yes (${qt_target} built with 'gssapi' feature)")

set(target "${PROJECT_NAME}")
set(soversion "2")
add_library(${target} SHARED stub_gssapi_krb5.c)
set_target_properties(${target} PROPERTIES
  OUTPUT_NAME gssapi_krb5
  SOVERSION ${soversion}
  LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/gssapi_krb5.ver"
)

install(TARGETS ${target}
  DESTINATION ${Slicer_INSTALL_LIB_DIR}
  COMPONENT Runtime
  )
