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
  # [pydicom]
  pydicom==2.1.2 --hash=sha256:d97f53a7b269dbd7414d18342f1b70f80d7d35dc4e479316bab146daac0e0c15
  # [/pydicom]
  # [Pillow]
  # Hashes correspond to the following packages:
  #  - Pillow-8.2.0-cp36-cp36m-macosx_10_10_x86_64.whl
  #  - Pillow-8.2.0-cp36-cp36m-manylinux1_x86_64.whl
  #  - Pillow-8.2.0-cp36-cp36m-manylinux2014_aarch64.whl
  #  - Pillow-8.2.0-cp36-cp36m-win_amd64.whl
  Pillow==8.2.0 --hash=sha256:dc38f57d8f20f06dd7c3161c59ca2c86893632623f33a42d592f097b00f720a9 \
                --hash=sha256:8bb1e155a74e1bfbacd84555ea62fa21c58e0b4e7e6b20e4447b8d07990ac78b \
                --hash=sha256:c5236606e8570542ed424849f7852a0ff0bce2c4c8d0ba05cc202a5a9c97dee9 \
                --hash=sha256:5afe6b237a0b81bd54b53f835a153770802f164c5570bab5e005aad693dab87f
  # [/Pillow]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [retrying]
  retrying==1.3.3 --hash=sha256:08c039560a6da2fe4f2c426d0766e284d3b736e355f8dd24b37367b0bb41973b
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.52.0 --hash=sha256:2fd1e6f599198246ca082f25561dce406d9ec32fda0bcec757910c79481e54c9
  # [/dicomweb-client]
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

