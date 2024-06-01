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
  #  - scipy-1.13.1-cp39-cp39-macosx_10_9_x86_64.whl
  #  - scipy-1.13.1-cp39-cp39-macosx_12_0_arm64.whl
  #  - scipy-1.13.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.13.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.13.1-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - scipy-1.13.1-cp39-cp39-win_amd64.whl
  scipy==1.13.1 --hash=sha256:436bbb42a94a8aeef855d755ce5a465479c721e9d684de76bf61a62e7c2b81d5 \
                --hash=sha256:8335549ebbca860c52bf3d02f80784e91a004b71b059e3eea9678ba994796a24 \
                --hash=sha256:d533654b7d221a6a97304ab63c41c96473ff04459e404b83275b60aa8f4b7004 \
                --hash=sha256:637e98dcf185ba7f8e663e122ebf908c4702420477ae52a04f9908707456ba4d \
                --hash=sha256:a014c2b3697bde71724244f63de2476925596c24285c7a637364761f8710891c \
                --hash=sha256:392e4ec766654852c25ebad4f64e4e584cf19820b980bc04960bca0b0cd6eaa2
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

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
