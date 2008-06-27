file(GLOB HEADERFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
foreach(headerfile ${HEADERFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${headerfile}
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/Resources/${headerfile}"
    COPYONLY
    )
endforeach(headerfile)

install(
  FILES ${HEADERFILES}
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}/Resources COMPONENT Development
  )

file(GLOB PNGFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png")
foreach(pngfile ${PNGFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${pngfile}
    "${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/Resources/${pngfile}"
    COPYONLY
    )
endforeach(pngfile)

install(
  FILES ${PNGFILES}
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}/Resources
  )

