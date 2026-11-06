# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

vtk_module_find_package(
  PACKAGE ZLIB
  PRIVATE_IF_SHARED
)

vtk_module_find_package(
  PACKAGE CURL
  PRIVATE_IF_SHARED
)

if(Slicer_USE_PYTHONQT_WITH_OPENSSL)
  vtk_module_find_package(
    PACKAGE OpenSSL
    PRIVATE_IF_SHARED
  )
endif()

vtk_module_find_package(PACKAGE vtkAddon)

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------

slicer_generate_module_wrapper(
  HEADER_NAME "vtkRemoteIOExport.h"
  OUTPUT_VAR wrapper_header_file
)

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------

set(classes vtkHTTPHandler)

vtk_module_add_module(Slicer::RemoteIO
  EXPORT_MACRO_PREFIX VTK_RemoteIO
  CLASSES ${classes}
  HEADERS ${wrapper_header_file}
  EXCLUDE_HEADER_TEST
)

vtk_module_link(Slicer::RemoteIO PUBLIC vtkAddon PRIVATE CURL::libcurl_static ZLIB::ZLIB)

if(WIN32)
  # Define CURL_STATICLIB when linking against static curl on Windows
  vtk_module_definitions(Slicer::RemoteIO PRIVATE CURL_STATICLIB)
  vtk_module_link(Slicer::RemoteIO PRIVATE iphlpapi ws2_32)
endif()
