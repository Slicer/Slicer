#
# Install the testing data and include the Testing/ sub-directory.
#

ADD_CUSTOM_TARGET(copytesting ALL
  ${CMAKE_COMMAND}
  -DSRC=${CMAKE_CURRENT_SOURCE_DIR}/Testing/TestData
  -DDST=${CMAKE_BINARY_DIR}/${EM_SHARE_DIR}/${PROJECT_NAME}/Testing
  -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/EM_InstallTasksSubScript.cmake
)

# Install the testing data
install(DIRECTORY
    Testing/TestData
    DESTINATION ${EM_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Testing
    FILES_MATCHING PATTERN "*"
    )

# include subdirectory
if(BUILD_TESTING)
    ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
