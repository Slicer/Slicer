if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  file(GLOB HEADERFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
  foreach(headerfile ${HEADERFILES})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${headerfile}
      "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_SHARE_DIR}/Resources/${headerfile}"
      COPYONLY
      )
  endforeach()

  install(
    FILES ${HEADERFILES}
    DESTINATION ${Slicer_INSTALL_SHARE_DIR}/Resources COMPONENT Development
    )

  file(GLOB PNGFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png")
  foreach(pngfile ${PNGFILES})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${pngfile}
      "${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/Resources/${pngfile}"
      COPYONLY
      )
  endforeach()

  install(
    FILES ${PNGFILES}
    DESTINATION ${Slicer_INSTALL_SHARE_DIR}/Resources
    COMPONENT Development
    )
endif()

# Should this location change, update Base/Logic/vtkSlicerModuleLogic.cxx

file(GLOB PARAMETERSETS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "ParameterSets/*.mrml")

foreach(parameterset ${PARAMETERSETS})
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${parameterset}
    "${CMAKE_BINARY_DIR}/${Slicer_SHARE_DIR}/${parameterset}"
    COPYONLY
    )
endforeach()
install(
  FILES ${PARAMETERSETS}
  DESTINATION ${Slicer_INSTALL_SHARE_DIR}/ParameterSets
  COMPONENT Runtime
  )

