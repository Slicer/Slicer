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
  #  - Pillow-7.2.0-cp36-cp36m-win_amd64.whl
  #  - Pillow-7.2.0-cp36-cp36m-macosx_10_10_x86_64.whl
  #  - Pillow-7.2.0-cp36-cp36m-manylinux1_x86_64.whl
  #  - Pillow-7.2.0-cp36-cp36m-manylinux2014_aarch64.whl
  pillow==7.2.0 --hash=sha256:ffe538682dc19cc542ae7c3e504fdf54ca7f86fb8a135e59dd6bc8627eae6cce \
                --hash=sha256:ec29604081f10f16a7aea809ad42e27764188fc258b02259a03a8ff7ded3808d \
                --hash=sha256:0a80dd307a5d8440b0a08bd7b81617e04d870e40a3e46a32d9c246e54705e86f \
                --hash=sha256:06aba4169e78c439d528fdeb34762c3b61a70813527a2c57f0540541e9f433a8
  six==1.15.0 --hash=sha256:8b74bedcbbbaca38ff6d7491d76f2b06b3592611af620f8426e82dddb04a5ced
  retrying==1.3.3 --hash=sha256:08c039560a6da2fe4f2c426d0766e284d3b736e355f8dd24b37367b0bb41973b
  dicomweb-client==0.41.0 --hash=sha256:e79163362d2af9399d4661216d6bcc3de9af755a1eec36995d35e3e135f22fa0
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

