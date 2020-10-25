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
  pydicom==2.0.0 --hash=sha256:667c5bf9ca52f440e538c9d03ce86b04555c10d069472a5db53103fe40d310c0
  # Hashes correspond to the following packages:
  #  - Pillow-8.0.1-cp36-cp36m-win_amd64.whl
  #  - Pillow-8.0.1-cp36-cp36m-macosx_10_10_x86_64.whl
  #  - Pillow-8.0.1-cp36-cp36m-manylinux1_x86_64.whl
  #  - Pillow-8.0.1-cp36-cp36m-manylinux2014_aarch64.whl
  pillow==8.0.1 --hash=sha256:7ba0ba61252ab23052e642abdb17fd08fdcfdbbf3b74c969a30c58ac1ade7cd3 \
                --hash=sha256:b63d4ff734263ae4ce6593798bcfee6dbfb00523c82753a3a03cbc05555a9cc3 \
                --hash=sha256:6b4a8fd632b4ebee28282a9fef4c341835a1aa8671e2770b6f89adc8e8c2703c \
                --hash=sha256:cc3ea6b23954da84dbee8025c616040d9aa5eaf34ea6895a0a762ee9d3e12e11
  six==1.15.0 --hash=sha256:8b74bedcbbbaca38ff6d7491d76f2b06b3592611af620f8426e82dddb04a5ced
  retrying==1.3.3 --hash=sha256:08c039560a6da2fe4f2c426d0766e284d3b736e355f8dd24b37367b0bb41973b
  dicomweb-client==0.50.2 --hash=sha256:e839b925a89e213c9e1f3b5b9046071c50b291e3d54f975e422ee39edd06c3f8
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

