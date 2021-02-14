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
  # [scipy]
  # Hashes correspond to the following packages:
  #  - scipy-1.5.4-cp36-cp36m-macosx_10_9_x86_64.whl
  #  - scipy-1.5.4-cp36-cp36m-manylinux1_x86_64.whl
  #  - scipy-1.5.4-cp36-cp36m-manylinux2014_aarch64.whl
  #  - scipy-1.5.4-cp36-cp36m-win_amd64.whl
  scipy==1.5.4 --hash=sha256:4f12d13ffbc16e988fa40809cbbd7a8b45bc05ff6ea0ba8e3e41f6f4db3a9e47 \
               --hash=sha256:368c0f69f93186309e1b4beb8e26d51dd6f5010b79264c0f1e9ca00cd92ea8c9 \
               --hash=sha256:4598cf03136067000855d6b44d7a1f4f46994164bcd450fb2c3d481afc25dd06 \
               --hash=sha256:65923bc3809524e46fb7eb4d6346552cbb6a1ffc41be748535aa502a2e3d3389
  # [/scipy]
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

