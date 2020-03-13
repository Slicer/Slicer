set(proj python-dicom-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-numpy
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
  foreach(module_name IN ITEMS
    pydicom
    six
    )
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
  ExternalProject_FindPythonPackage(
    MODULE_NAME PIL #pillow
    REQUIRED
    )
  ExternalProject_FindPythonPackage(
    MODULE_NAME dicomweb_client
    NO_VERSION_PROPERTY
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  pydicom==1.4.2 --hash=sha256:f315ba2296346f4f9913c269618201e170b9326362e2ada6041ca91b7cb2117b
  # Hashes correspond to the following packages:
  #  - Pillow-7.0.0-cp36-cp36m-win_amd64.whl
  #  - Pillow-7.0.0-cp36-cp36m-macosx_10_6_intel.whl
  #  - Pillow-7.0.0-cp36-cp36m-manylinux1_x86_64.whl
  pillow==7.0.0 --hash=sha256:bf598d2e37cf8edb1a2f26ed3fb255191f5232badea4003c16301cb94ac5bdd0 \
                --hash=sha256:ab76e5580b0ed647a8d8d2d2daee170e8e9f8aad225ede314f684e297e3643c2 \
                --hash=sha256:8ac6ce7ff3892e5deaab7abaec763538ffd011f74dc1801d93d3c5fc541feee2
  six==1.14.0 --hash=sha256:8f3cd2e254d8f793e7f3d6d9df77b92252b52637291d0f0da013c76ea2724b6c
  dicomweb-client==0.21.0 --hash=sha256:e8b46e517a5399b85b2f4e724e54bfe3a934bc22c9c2ba3d0e92eb3e87846228
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

