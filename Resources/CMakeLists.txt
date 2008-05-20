file(GLOB resources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
foreach(resource ${resources})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${resource}
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/Resources/${resource}"
    COPYONLY
    )
endforeach(resource ${resources})
install(
  FILES ${resources}
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}/Resources COMPONENT Development
  )
