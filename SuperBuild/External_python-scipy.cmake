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
  # - scipy-1.4.1-cp36-cp36m-win_amd64.whl
  # - scipy-1.4.1-cp36-cp36m-macosx_10_6_intel.whl
  # - scipy-1.4.1-cp36-cp36m-manylinux1_x86_64.whl
  scipy==1.4.1 --hash=sha256:dc60bb302f48acf6da8ca4444cfa17d52c63c5415302a9ee77b3b21618090521 \
              --hash=sha256:bb517872058a1f087c4528e7429b4a44533a902644987e7b2fe35ecc223bc408 \
              --hash=sha256:386086e2972ed2db17cebf88610aab7d7f6e2c0ca30042dc9a89cf18dcc363fa
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

