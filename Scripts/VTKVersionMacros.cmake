# ---------------------------------------------------------------------------
# VTK_GET_SOURCE_REVISION_AND_DATE
# Get vtkVTKVersion's source revision and date and store them in
# ${revision_varname} and ${date_varname} respectively.
#
# This macro can be used to require a specific revision of the VTK
# library in between version changes.
# For example:
#   include("${VTK_CMAKE_DIR}/VTKVersionMacros.cmake")
#   VTK_GET_SOURCE_REVISION_AND_DATE(source_revision source_date)
#   if(source_revision LESS 1.4)
#    message(FATAL_ERROR "Sorry, your VTK library was last updated on ${source_date}. Its source revision, according to vtkVTKVersion.h, is ${source_revision}. Please update to a newer revision.")
#   endif(source_revision LESS 1.4)

macro(VTK_GET_SOURCE_REVISION_AND_DATE
    revision_varname
    date_varname)

  set(${revision_varname})
  set(${date_varname})
  set(___vtk_version_found)
  foreach(dir ${VTK_INCLUDE_DIRS} ${VTK_INCLUDE_PATH})
    if(NOT ___vtk_version_found)
      set(file "${dir}/vtkVersion.h")
      if(EXISTS ${file})
        file(READ ${file} file_contents)
        string(REGEX REPLACE "(.*Revision: )([0-9]+\\.[0-9]+)( .*)" "\\2"
          ${revision_varname} "${file_contents}")
        string(REGEX REPLACE "(.*Date: )(.+)( \\$.*)" "\\2"
          ${date_varname} "${file_contents}")
        set(___vtk_version_found 1)
      endif(EXISTS ${file})
    endif(NOT ___vtk_version_found)
  endforeach(dir)

  if(NOT ${revision_varname} OR NOT ${date_varname})
    message(FATAL_ERROR "Sorry, vtkVTKVersion's source revision could not be found, either because vtkVTKVersion.h is nowhere in sight, or its contents could not be parsed successfully.")
  endif(NOT ${revision_varname} OR NOT ${date_varname})

endmacro(VTK_GET_SOURCE_REVISION_AND_DATE)
