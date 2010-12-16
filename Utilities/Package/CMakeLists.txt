# -------------------------------------------------------------------------
# Find and install Tcl
# -------------------------------------------------------------------------
IF (Slicer_USE_PYTHONQT_WITH_TCL)
  if(NOT Slicer_TCL_DIR)
    get_filename_component(Slicer_TCL_DIR "${TCL_INCLUDE_PATH}" PATH)
  endif()

  if(NOT EXISTS "${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT}" AND
      NOT EXISTS "${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe")
    message(STATUS "Not Packaging TCL (cannot found: ${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT} or ${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe)")
    set(Slicer_TCL_DIR "")
  endif()

  # Note: this is probably dangerous if Tcl is somewhere in /usr/local, as it
  # ends up installing the whole /usr/local to the Slicer3 install tree :(
  # TODO: use VTK/KWWidgets macros to copy only the files that are known to
  # belong to Tcl/Tk; in the meantime only a few people are using external
  # VTK/KWWidgets/Teem/TclTk packages, so we will assume they know what they
  #  are doing (i.e. they have Tcl/Tk installed in a standalone directory 
  # like /opt/tcltk8.5.0)
  
  SET(TclTk_INSTALL_LIB_DIR lib/TclTk)
  
  if(Slicer_TCL_DIR)
    install(DIRECTORY
      ${Slicer_TCL_DIR}/
      DESTINATION ${TclTk_INSTALL_LIB_DIR}
      USE_SOURCE_PERMISSIONS
      REGEX "man/" EXCLUDE
      REGEX "include/" EXCLUDE
      REGEX "bin/" EXCLUDE
      REGEX "demos/" EXCLUDE
      PATTERN "*.sh" EXCLUDE
      PATTERN "*.c" EXCLUDE
      )
  endif()
ENDIF()

# -------------------------------------------------------------------------
# Find and install python
# -------------------------------------------------------------------------
if(Slicer_USE_PYTHON OR Slicer_USE_PYTHONQT)
  #assum get build test build tree
  get_filename_component(PARENT_BUILD_DIR "${Slicer_BINARY_DIR}" PATH)
  set(PYTHON_DIR "${PARENT_BUILD_DIR}/Slicer3-lib/python-build")

  #try superbuild tree
  if(NOT EXISTS ${PYTHON_DIR})
    get_filename_component(SUPER_BUILD_DIR "${Slicer_BINARY_DIR}" PATH)
    set(PYTHON_DIR "${SUPER_BUILD_DIR}/python-build")
  endif()
  if(WIN32 AND NOT EXISTS "${PYTHON_DIR}" AND EXISTS "${PYTHON_INCLUDE_PATH}")
    get_filename_component(PYTHON_DIR "${PYTHON_INCLUDE_PATH}" PATH)
  endif()

  if(EXISTS "${PYTHON_DIR}")
    install(DIRECTORY
      "${PYTHON_DIR}/"
      DESTINATION lib/Python
      USE_SOURCE_PERMISSIONS
      REGEX "include/" EXCLUDE
      REGEX "share/man" EXCLUDE
      )
  endif()

  #install python dll.
  if(WIN32)
    get_filename_component(PYTHON_LIB_BASE ${PYTHON_LIBRARY} NAME_WE)
    get_filename_component(PYTHON_LIB_PATH ${PYTHON_LIBRARY} PATH)

    install(FILES "${PYTHON_LIB_PATH}/${PYTHON_LIB_BASE}.dll"
      DESTINATION bin
      COMPONENT Runtime)
  endif()
endif()

# -------------------------------------------------------------------------
# Find and install QT
# -------------------------------------------------------------------------
if(Slicer_USE_QT)

  SET(QT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR})

  SET (QTLIBLIST QTCORE QTGUI QTNETWORK QTXML QTTEST QTSQL QTOPENGL QTWEBKIT PHONON QTXMLPATTERNS)
  IF(UNIX OR APPLE)
    LIST(APPEND QTLIBLIST QTDBUS)
  ENDIF()
  FOREACH(qtlib ${QTLIBLIST})
    IF (QT_${qtlib}_LIBRARY_RELEASE)
      IF(APPLE)
        INSTALL(DIRECTORY "${QT_${qtlib}_LIBRARY_RELEASE}" 
          DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime)
      ELSEIF(UNIX)
        # Install .so and versioned .so.x.y
        GET_FILENAME_COMPONENT(QT_LIB_DIR_tmp ${QT_${qtlib}_LIBRARY_RELEASE} PATH)
        GET_FILENAME_COMPONENT(QT_LIB_NAME_tmp ${QT_${qtlib}_LIBRARY_RELEASE} NAME)
        INSTALL(DIRECTORY ${QT_LIB_DIR_tmp}/ 
          DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime
          FILES_MATCHING PATTERN "${QT_LIB_NAME_tmp}*"
          PATTERN "${QT_LIB_NAME_tmp}*.debug" EXCLUDE)
      ELSEIF(WIN32)
        GET_FILENAME_COMPONENT(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
        INSTALL(FILES ${QT_DLL_PATH_tmp}/${qtlib}4.dll 
          DESTINATION bin COMPONENT Runtime)
      ENDIF()
    ENDIF()
  ENDFOREACH()
endif()

# -------------------------------------------------------------------------
# Find and install PythonQt
# -------------------------------------------------------------------------
IF(Slicer_USE_PYTHONQT)

  SET(PYTHONQT_INSTALL_LIB_DIR "${Slicer_INSTALL_LIB_DIR}")
  
  IF(WIN32)
    INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/bin/PythonQt.dll 
      DESTINATION bin COMPONENT Runtime)
  ELSEIF(APPLE)
    # needs to install symlink version named libraries as well.
    INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/lib/libPythonQt.dylib 
      DESTINATION ${PYTHONQT_INSTALL_LIB_DIR} COMPONENT Runtime)
  ELSEIF(UNIX)
    INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/lib/libPythonQt.so 
      DESTINATION ${PYTHONQT_INSTALL_LIB_DIR} COMPONENT Runtime)
  ENDIF()
ENDIF()

# -------------------------------------------------------------------------
# Find and install LibArchive
# -------------------------------------------------------------------------

IF(WIN32)
  INSTALL(FILES ${LibArchive_DIR}/bin/archive.dll
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Runtime
    )
ELSE()
  slicerInstallLibrary(FILE ${LibArchive_LIBRARY}
    DESTINATION ${Slicer_INSTALL_LIB_DIR}
    COMPONENT Runtime
    )
ENDIF()


# -------------------------------------------------------------------------
# CPack
# -------------------------------------------------------------------------

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in"
  "${Slicer_BINARY_DIR}/PackageLibraries/CMakeLists.txt"
  @ONLY)

include(InstallRequiredSystemLibraries)
include(${CMAKE_CURRENT_SOURCE_DIR}/InstallDebugSystemLibraries.cmake)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Medical Visualization and Processing Environment for Research")

SET(CPACK_MONOLITHIC_INSTALL ON)
#append RuntimeExecutables
#SET(CPACK_COMPONENTS_ALL "Development;Runtime;RuntimeLibraries;Unspecified;RuntimeExecutables;")

set(CPACK_PACKAGE_VENDOR "NA-MIC")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${Slicer_SOURCE_DIR}/README.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${Slicer_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${Slicer_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Slicer_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${Slicer_VERSION_PATCH}")

# 
# Slicer
#
set(CPACK_INSTALL_CMAKE_PROJECTS
  "${Slicer_BINARY_DIR};Slicer;ALL;/")

# 
# VTK
#
if(EXISTS "${VTK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTK_DIR};VTK;RuntimeLibraries;/")
endif()

# 
# ITK
#
if(EXISTS "${ITK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;RuntimeLibraries;/")
endif()

# 
# Teem
#
if(EXISTS "${Teem_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Teem_DIR};teem;ALL;/")
endif()

# 
# BatchMake
#
if(EXISTS "${BatchMake_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${BatchMake_DIR};BatchMake;Runtime;/")
endif()

# 
# OpenIGTLink
#
if(EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${OpenIGTLink_DIR};igtl;RuntimeLibraries;/")
endif()

# 
# CTK
#
if(EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;Runtime;/")
endif()

if(APPLE)
  set(CPACK_PACKAGE_ICON "${Slicer_SOURCE_DIR}/Utilities/Package/Slicer3.icns")
endif()

# slicer3 does _not_ require setting the windows path
set(CPACK_NSIS_MODIFY_PATH OFF)

set(APPLICATION_NAME "Slicer")
set(EXECUTABLE_NAME "Slicer3")
set(CPACK_PACKAGE_EXECUTABLES "..\\\\${EXECUTABLE_NAME}" "${APPLICATION_NAME}")

# -------------------------------------------------------------------------
# File extensions

set(FILE_EXTENSIONS .mrml .xcat)

if(FILE_EXTENSIONS)

  # For NSIS (Win32) now, we will add MacOSX support later (get back to Wes)

  if(WIN32 AND NOT UNIX)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS)
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS)
    foreach(ext ${FILE_EXTENSIONS})
      string(LENGTH "${ext}" len)
      math(EXPR len_m1 "${len} - 1")
      string(SUBSTRING "${ext}" 1 ${len_m1} ext_wo_dot)
      set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS 
        "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
            WriteRegStr HKCR \\\"${APPLICATION_NAME}\\\" \\\"\\\" \\\"${APPLICATION_NAME} supported file\\\"
            WriteRegStr HKCR \\\"${APPLICATION_NAME}\\\\shell\\\\open\\\\command\\\" \\\"\\\" \\\"$\\\\\\\"$INSTDIR\\\\${EXECUTABLE_NAME}.exe$\\\\\\\" $\\\\\\\"%1$\\\\\\\"\\\"
            WriteRegStr HKCR \\\"${ext}\\\" \\\"\\\" \\\"${APPLICATION_NAME}\\\"
            WriteRegStr HKCR \\\"${ext}\\\" \\\"Content Type\\\" \\\"application/x-${ext_wo_dot}\\\"
          ")
      set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
            DeleteRegKey HKCR \\\" ${APPLICATION_NAME}\\\"
            DeleteRegKey HKCR \\\"${ext}\\\"
          ")
    endforeach()
  endif()

endif()

include(CPack)

