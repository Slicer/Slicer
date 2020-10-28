set(proj python-scipy)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-ensurepip
  python-numpy
  python-pip
  python-setuptools
  python-wheel
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS scipy)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # Hashes correspond to the following packages:
  # - scipy-1.5.3-cp36-cp36m-win_amd64.whl
  # - scipy-1.5.3-cp36-cp36m-macosx_10_9_x86_64.whl
  # - scipy-1.5.3-cp36-cp36m-manylinux1_x86_64.whl
  # - scipy-1.5.3-cp36-cp36m-manylinux2014_aarch64.whl
  scipy==1.5.3 --hash=sha256:ffcbd331f1ffa82e22f1d408e93c37463c9a83088243158635baec61983aaacf \
               --hash=sha256:f574558f1b774864516f3c3fe072ebc90a29186f49b720f60ed339294b7f32ac \
               --hash=sha256:b9751b39c52a3fa59312bd2e1f40144ee26b51404db5d2f0d5259c511ff6f614 \
               --hash=sha256:d5e3cc60868f396b78fc881d2c76460febccfe90f6d2f082b9952265c79a8788
  ]===])

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${PYTHON_EXECUTABLE} -m pip install --require-hashes -r ${requirements_file}
    LOG_INSTALL 1
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${_version}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

