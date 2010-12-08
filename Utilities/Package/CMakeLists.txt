# -------------------------------------------------------------------------
# Find and install Tcl
IF (VTK_WRAP_TCL OR Slicer_USE_PYTHONQT_WITH_TCL)
  if(NOT Slicer_TCL_DIR)
    if(TCL_INCLUDE_PATH)
      get_filename_component(Slicer_TCL_DIR "${TCL_INCLUDE_PATH}" PATH)
    else(TCL_INCLUDE_PATH)
      set(Slicer_TCL_DIR "${Slicer_BINARY_DIR}/../Slicer3-lib/tcl-build")
    endif(TCL_INCLUDE_PATH)
  endif(NOT Slicer_TCL_DIR)

  if(NOT EXISTS "${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT}" AND
      NOT EXISTS "${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe")
    message(STATUS "Not Packaging TCL (cannot found: ${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT} or ${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe)")
    set(Slicer_TCL_DIR "")
  endif(NOT EXISTS "${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION_DOT}" AND
    NOT EXISTS "${Slicer_TCL_DIR}/bin/tclsh${TCL_TK_VERSION}.exe")
ENDIF (VTK_WRAP_TCL OR Slicer_USE_PYTHONQT_WITH_TCL)

# Note: this is probably dangerous if Tcl is somewhere in /usr/local, as it
# ends up installing the whole /usr/local to the Slicer3 install tree :(
# TODO: use VTK/KWWidgets macros to copy only the files that are known to
# belong to Tcl/Tk; in the meantime only a few people are using external
# VTK/KWWidgets/Teem/TclTk packages, so we will assume they know what they
#  are doing (i.e. they have Tcl/Tk installed in a standalone directory 
# like /opt/tcltk8.5.0)

IF (VTK_WRAP_TCL OR Slicer_USE_PYTHONQT_WITH_TCL)
  if(Slicer_TCL_DIR)
    install(DIRECTORY
      ${Slicer_TCL_DIR}/
      DESTINATION lib/TclTk
      USE_SOURCE_PERMISSIONS
      )
  endif(Slicer_TCL_DIR)
ENDIF (VTK_WRAP_TCL OR Slicer_USE_PYTHONQT_WITH_TCL)

# In the following section we tell the packager to include libslicerlibcurl.so
# in the CPACK package, because we build it as a .so on Solaris,
# and need it run-time. And we find and package a few more libs too.

if(CMAKE_SYSTEM MATCHES "SunOS.*")
  FIND_PATH(CURL_DIR libslicerlibcurl.so ${Slicer_BINARY_DIR}/../Slicer3-lib/cmcurl-build/bin)
  FIND_PATH(SFW_LIB_DIR libgcc_s.so.1 /usr/sfw/lib)
  FIND_PATH(SFW_LIB_DIR_64 libgcc_s.so.1 /usr/sfw/lib/amd64)
  FIND_PATH(SUN_CC_LIB_DIR libCrun.so.1 /usr/lib)
  FIND_PATH(SUN_CC_LIB_DIR_64 libCrun.so.1 /usr/lib/amd64)

# This might look ugly here: since CPACK puts the softlink itself into the install dir, and Sun puts stlport.so and stlport.so.1
# as softlinks into the /opt/SUNWspro/lib/stlport(/amd64) directories, we have to feed more directories into CPACK to find libstlport.so, and libstlport.so.1
# Like: /opt/SUNWspro/lib/stlport4/amd64/libstlport.so -> ./libstlport.so.1*
# /opt/SUNWspro/lib/stlport4/amd64/libstlport.so.1 -> ../../../prod/lib/stlport4/amd64/libstlport.so.1*
# /opt/SUNWspro/prod/lib/stlport4/amd64/libstlport.so.1*
# /opt/SUNWspro/prod/lib/stlport4/libstlport.so.1*
# There may be a workaround (so that we can find and install from the /opt/SUNWspro/lib locactions), but I'm not aware of any at the moment.

  FIND_PATH(SUN_STLPORT4_SO_LIB_DIR libstlport.so /opt/SUNWspro/lib/stlport4)
  FIND_PATH(SUN_STLPORT4_SO_LIB_DIR_64 libstlport.so /opt/SUNWspro/lib/stlport4/amd64)
  FIND_PATH(SUN_STLPORT4_SO_1_LIB_DIR libstlport.so.1 /opt/SUNWspro/prod/lib/stlport4)
  FIND_PATH(SUN_STLPORT4_SO_1_LIB_DIR_64 libstlport.so.1 /opt/SUNWspro/prod/lib/stlport4/amd64)

# Let's check if we're okay

  message(STATUS "CURL_DIR is ${CURL_DIR}")
  message(STATUS "SFW_LIB_DIR is ${SFW_LIB_DIR}")

  FIND_FILE(CURL_LIB libslicerlibcurl.so PATHS ${CURL_DIR})

# Bellow we'll find the libraries for various compilers, and their 64 bit versions too.
# Remark: since the whole toolchain should be compiled with the stlport4 library (with Sun's compilers), Crun and Cstd are here only for convernience.

  FIND_FILE(GNUSTDCXX1 libstdc++.so PATHS ${SFW_LIB_DIR})
  FIND_FILE(GNUSTDCXX2 libstdc++.so.6 PATHS ${SFW_LIB_DIR})
  FIND_FILE(GNUSTDCXX3 libstdc++.so.6.0.3 PATHS ${SFW_LIB_DIR})
  FIND_FILE(LIBGCC1 libgcc_s.so libgcc_s.so.1 PATHS ${SFW_LIB_DIR})
  FIND_FILE(LIBGCC2 libgcc_s.so.1 PATHS ${SFW_LIB_DIR})
  FIND_FILE(LIBCRUN libCrun.so.1 PATHS ${SUN_CC_LIB_DIR})
  FIND_FILE(LIBCSTD libCstd.so.1 PATHS ${SUN_CC_LIB_DIR})
  FIND_FILE(STLPORT4_SO_32 libstlport.so  PATHS ${SUN_STLPORT4_SO_LIB_DIR})
  FIND_FILE(STLPORT4_SO_1_32 libstlport.so.1  PATHS ${SUN_STLPORT4_SO_1_LIB_DIR})

# These are the 64 bit counterparts of the above libraries.

  FIND_FILE(GNUSTDCXX_64_1 libstdc++.so PATHS ${SFW_LIB_DIR_64})
  FIND_FILE(GNUSTDCXX_64_2 libstdc++.so.6 PATHS ${SFW_LIB_DIR_64})
  FIND_FILE(GNUSTDCXX_64_3 libstdc++.so.6.0.3 PATHS ${SFW_LIB_DIR_64})
  FIND_FILE(LIBGCC_64_1 libgcc_s.so libgcc_s.so.1 PATHS ${SFW_LIB_DIR_64})
  FIND_FILE(LIBGCC_64_2 libgcc_s.so.1 PATHS ${SFW_LIB_DIR_64})
  FIND_FILE(LIBCRUN_64 libCrun.so.1 PATHS ${SUN_CC_LIB_DIR_64})
  FIND_FILE(LIBCSTD_64 libCstd.so.1 PATHS ${SUN_CC_LIB_DIR_64})
  FIND_FILE(STLPORT4_SO_64 libstlport.so  PATHS ${SUN_STLPORT4_SO_LIB_DIR_64})
  FIND_FILE(STLPORT4_SO_1_64 libstlport.so.1  PATHS ${SUN_STLPORT4_SO_1_LIB_DIR_64})

# Let's check these...

 message(STATUS "CURL_LIB is ${CURL_LIB} GNUSTDCXX1 is ${GNUSTDCXX1} GNUSTDCXX2 is ${GNUSTDCXX2} GNUSTDCXX3 is ${GNUSTDCXX3} LIBGCC1 is ${LIBGCC1}  LIBGCC2 is ${LIBGCC2} LIBCRUN is ${LIBCRUN} LIBCSTD is ${LIBCSTD} STLPORT4_SO_32 is ${STLPORT4_SO_32} STLPORT4_SO_1_32 is ${STLPORT4_SO_1_32}")
 message (STATUS " These are the 64 bit libs: GNUSTDCXX_64_1 is ${GNUSTDCXX_64_1} GNUSTDCXX_64_2 is ${GNUSTDCXX_64_2} GNUSTDCXX_64_3 is ${GNUSTDCXX_64_3} LIBGCC_64_1 is ${LIBGCC_64_1}  LIBGCC_64_2 is ${LIBGCC_64_2}  LIBCRUN_64 is ${LIBCRUN_64} LIBCSTD_64 is ${LIBCSTD_64} STLPORT4_SO_64 is ${STLPORT4_SO_64} STLPORT4_SO_1_64 is ${STLPORT4_SO_1_64}")
# Let's install them.

  INSTALL(FILES
      ${CURL_LIB} 
      DESTINATION lib/Slicer3
      )      
  INSTALL(FILES
      ${GNUSTDCXX1} ${GNUSTDCXX2} ${GNUSTDCXX3} ${LIBGCC1} ${LIBGCC2} ${LIBCRUN} ${LIBCSTD} ${STLPORT4_SO_32} ${STLPORT4_SO_1_32}
      DESTINATION lib/Slicer3/CompilerLibs/32
      ) 
  INSTALL(FILES
      ${GNUSTDCXX_64_1} ${GNUSTDCXX_64_2} ${GNUSTDCXX_64_3} ${LIBGCC_64_1} ${LIBGCC_64_2} ${LIBCRUN} ${LIBCSTD} ${STLPORT4_SO_64} ${STLPORT4_SO_1_64}
      DESTINATION lib/Slicer3/CompilerLibs/64
      ) 
endif(CMAKE_SYSTEM MATCHES "SunOS.*")

# -------------------------------------------------------------------------
# Find and install python

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
  endif(WIN32 AND NOT EXISTS "${PYTHON_DIR}" AND EXISTS "${PYTHON_INCLUDE_PATH}")

  if(EXISTS "${PYTHON_DIR}")
    install(DIRECTORY
      "${PYTHON_DIR}/"
      DESTINATION lib/Python
      USE_SOURCE_PERMISSIONS
      )
  endif(EXISTS "${PYTHON_DIR}")

  #install python dll.
  if(WIN32)
    get_filename_component(PYTHON_LIB_BASE ${PYTHON_LIBRARY} NAME_WE)
    get_filename_component(PYTHON_LIB_PATH ${PYTHON_LIBRARY} PATH)

    install(FILES "${PYTHON_LIB_PATH}/${PYTHON_LIB_BASE}.dll"
      DESTINATION bin
      COMPONENT Runtime)
  endif()
endif(Slicer_USE_PYTHON OR Slicer_USE_PYTHONQT)

# -------------------------------------------------------------------------
# Find and install QT

if(Slicer_USE_QT)

  SET (QTLIBLIST QTCORE QTGUI QTNETWORK QTXML QTTEST QTSQL QTOPENGL QTWEBKIT PHONON QTXMLPATTERNS)
  IF(UNIX OR APPLE)
    LIST(APPEND QTLIBLIST QTDBUS)
  ENDIF()
  FOREACH(qtlib ${QTLIBLIST})
    IF (QT_${qtlib}_LIBRARY_RELEASE)
      IF(APPLE)
        INSTALL(DIRECTORY "${QT_${qtlib}_LIBRARY_RELEASE}" DESTINATION lib/Qt COMPONENT Runtime)
      ELSEIF(UNIX)
        # Install .so and versioned .so.x.y
        GET_FILENAME_COMPONENT(QT_LIB_DIR_tmp ${QT_${qtlib}_LIBRARY_RELEASE} PATH)
        GET_FILENAME_COMPONENT(QT_LIB_NAME_tmp ${QT_${qtlib}_LIBRARY_RELEASE} NAME)
        INSTALL(DIRECTORY ${QT_LIB_DIR_tmp}/ DESTINATION lib/Qt COMPONENT Runtime
                FILES_MATCHING PATTERN "${QT_LIB_NAME_tmp}*"
                PATTERN "${QT_LIB_NAME_tmp}*.debug" EXCLUDE)
      ELSEIF(WIN32)
        GET_FILENAME_COMPONENT(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
        INSTALL(FILES ${QT_DLL_PATH_tmp}/${qtlib}4.dll DESTINATION bin COMPONENT Runtime)
      ENDIF(APPLE)
    ENDIF (QT_${qtlib}_LIBRARY_RELEASE)
  ENDFOREACH(qtlib)
  
  IF(Slicer_USE_PYTHONQT)
    IF(WIN32)
      INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/bin/PythonQt.dll DESTINATION bin COMPONENT Runtime)
    ELSEIF(APPLE)
      # needs to install symlink version named libraries as well.
      INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/lib/libPythonQt.dylib DESTINATION lib/PythonQt COMPONENT Runtime)
    ELSEIF(UNIX)
      INSTALL(FILES ${PYTHONQT_INSTALL_DIR}/lib/libPythonQt.so DESTINATION lib/PythonQt COMPONENT Runtime)
    ENDIF(WIN32)
  ENDIF(Slicer_USE_PYTHONQT)
endif(Slicer_USE_QT)

# -------------------------------------------------------------------------
# CPack

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
set(CPACK_INSTALL_CMAKE_PROJECTS
  "${Slicer_BINARY_DIR};Slicer;ALL;/")
if(EXISTS "${VTK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTK_DIR};VTK;ALL;/")
endif(EXISTS "${VTK_DIR}/CMakeCache.txt")
if(EXISTS "${ITK_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;ALL;/")
endif(EXISTS "${ITK_DIR}/CMakeCache.txt")
IF (Slicer_USE_KWWIDGETS)
  if(EXISTS "${KWWidgets_DIR}/CMakeCache.txt")
    set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${KWWidgets_DIR};KWWidgets;ALL;/")
  endif(EXISTS "${KWWidgets_DIR}/CMakeCache.txt")
ENDIF (Slicer_USE_KWWIDGETS)
if(EXISTS "${Teem_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${Teem_DIR};teem;ALL;/")
endif(EXISTS "${Teem_DIR}/CMakeCache.txt")
if(EXISTS "${BatchMake_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${BatchMake_DIR};BatchMake;ALL;/")
endif(EXISTS "${BatchMake_DIR}/CMakeCache.txt")
if(EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${OpenIGTLink_DIR};igtl;ALL;/")
endif(EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
if(EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
  set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CTK_DIR}/CTK-build;CTK;ALL;/")
endif(EXISTS "${CTK_DIR}/CTK-build/CMakeCache.txt")
if(APPLE)
  set(CPACK_PACKAGE_ICON "${Slicer_SOURCE_DIR}/Utilities/Package/Slicer3.icns")
endif(APPLE)

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
    endforeach(ext)
  endif(WIN32 AND NOT UNIX)

endif(FILE_EXTENSIONS)

include(CPack)

