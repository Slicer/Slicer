#-----------------------------------------------------------------------------
# Temporary hack to set/get persistent values accross subdirs until 2.6 comes
# Neither set_source_file_properties nor set(... INTERNAL FORCE) would work

#-----------------------------------------------------------------------------
macro(slicer3_set_persistent_property property_name)
  set(_file "${CMAKE_BINARY_DIR}/persistent/${property_name}")
  file(WRITE "${_file}" "${ARGN}")
  #set(__${property_name} ${value} CACHE INTERNAL "" FORCE)
endmacro(slicer3_set_persistent_property property_name value)

#-----------------------------------------------------------------------------
macro(slicer3_get_persistent_property property_name var_name)
  set(_file "${CMAKE_BINARY_DIR}/persistent/${property_name}")
  if(EXISTS "${_file}")
    file(READ "${_file}" ${var_name})
  else(EXISTS "${_file}")
    set(${var_name})
  endif(EXISTS "${_file}")
  #set(${var_name} ${__${property_name}})
endmacro(slicer3_get_persistent_property property_name var_name)

