# -------------------------------------------------------------------------
# Find and install Qt
# -------------------------------------------------------------------------
SET(QT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR})

SET (QTLIBLIST QTCORE QTGUI QTNETWORK QTXML QTTEST QTSCRIPT QTSQL QTOPENGL QTWEBKIT PHONON QTXMLPATTERNS)
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

