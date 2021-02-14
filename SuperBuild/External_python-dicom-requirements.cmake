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
  #  - Pillow-8.1.2-cp36-cp36m-macosx_10_10_x86_64.whl
  #  - Pillow-8.1.2-cp36-cp36m-manylinux1_x86_64.whl
  #  - Pillow-8.1.2-cp36-cp36m-manylinux2014_aarch64.whl
  #  - Pillow-8.1.2-cp36-cp36m-win_amd64.whl
  Pillow==8.1.2 --hash=sha256:5cf03b9534aca63b192856aa601c68d0764810857786ea5da652581f3a44c2b0 \
                --hash=sha256:5762ebb4436f46b566fc6351d67a9b5386b5e5de4e58fdaa18a1c83e0e20f1a8 \
                --hash=sha256:e2cd8ac157c1e5ae88b6dd790648ee5d2777e76f1e5c7d184eaddb2938594f34 \
                --hash=sha256:d1d6bca39bb6dd94fba23cdb3eeaea5e30c7717c5343004d900e2a63b132c341
  # [/Pillow]
  # [six]
  six==1.15.0 --hash=sha256:8b74bedcbbbaca38ff6d7491d76f2b06b3592611af620f8426e82dddb04a5ced
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

