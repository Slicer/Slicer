set(proj python-extension-manager-ssl-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-pip
  python-requests-requirements
  python-setuptools
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS jwt)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
  ExternalProject_FindPythonPackage(
    MODULE_NAME github
    NO_VERSION_PROPERTY
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [PyJWT]
  PyJWT==1.7.1 --hash=sha256:5c6eca3c2940464d106b99ba83b00c6add741c9becaec087fb7ccdefea71350e
  # [/PyJWT]
  # [wrapt]
  # Hashes correspond to the following packages:
  #  - wrapt-1.13.3-cp39-cp39-macosx_10_9_x86_64.whl
  #  - wrapt-1.13.3-cp39-cp39-manylinux_2_5_x86_64.manylinux1_x86_64.manylinux_2_12_x86_64.manylinux2010_x86_64.whl
  #  - wrapt-1.13.3-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - wrapt-1.13.3-cp39-cp39-win_amd64.whl
  wrapt==1.13.3 --hash=sha256:981da26722bebb9247a0601e2922cedf8bb7a600e89c852d063313102de6f2cb \
                --hash=sha256:25b1b1d5df495d82be1c9d2fad408f7ce5ca8a38085e2da41bb63c914baadff7 \
                --hash=sha256:865c0b50003616f05858b22174c40ffc27a38e67359fa1495605f96125f76640 \
                --hash=sha256:81bd7c90d28a4b2e1df135bfbd7c23aee3050078ca6441bead44c42483f9ebfb
  # [/wrapt]
  # [Deprecated]
  Deprecated==1.2.13 --hash=sha256:64756e3e14c8c5eea9795d93c524551432a0be75629f8f29e67ab8caf076c76d
  # [/Deprecated]
  # [PyGithub]
  PyGithub==1.54.1 --hash=sha256:87afd6a67ea582aa7533afdbf41635725f13d12581faed7e3e04b1579c0c0627
  # [/PyGithub]
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

