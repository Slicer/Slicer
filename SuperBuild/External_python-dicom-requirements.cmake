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
  pydicom==1.3.0 --hash=sha256:ee3a94de180c6337e1443ce346730a784f9027f56175f61ba4dfda632693b843
  # Hashes correspond to the following packages:
  #  - Pillow-6.2.1-cp36-cp36m-win_amd64.whl
  #  - Pillow-6.2.1-cp36-cp36m-macosx_10_6_intel.whl
  #  - Pillow-6.2.1-cp36-cp36m-manylinux1_x86_64.whl
  pillow==6.2.1 --hash=sha256:83792cb4e0b5af480588601467c0764242b9a483caea71ef12d22a0d0d6bdce2 \
                --hash=sha256:846fa202bd7ee0f6215c897a1d33238ef071b50766339186687bd9b7a6d26ac5 \
                --hash=sha256:e0697b826da6c2472bb6488db4c0a7fa8af0d52fa08833ceb3681358914b14e5
  six==1.12.0 --hash=sha256:3350809f0555b11f552448330d0b52d5f24c91a322ea4a15ef22629740f3761c
  dicomweb_client==0.14.0 --hash=sha256:6391a8a8be33919c2aa869f6538eeeb3d00f2546cb5abd29c8e03bc30cf90e44
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

