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
  pydicom==2.3.0 --hash=sha256:8ff31e077cc51d19ac3b8ca988ac486099cdebfaf885989079fdc7c75068cdd8
  # [/pydicom]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [Pillow]
  # Hashes correspond to the following packages:
  #  - Pillow-9.0.1-1-cp39-cp39-macosx_11_0_arm64.whl
  #  - Pillow-9.0.1-cp39-cp39-macosx_10_10_x86_64.whl
  #  - Pillow-9.0.1-cp39-cp39-macosx_11_0_arm64.whl
  #  - Pillow-9.0.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - Pillow-9.0.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - Pillow-9.0.1-cp39-cp39-win_amd64.whl
  Pillow==9.0.1 --hash=sha256:b9618823bd237c0d2575283f2939655f54d51b4527ec3972907a927acbcc5bfc \
                --hash=sha256:80ca33961ced9c63358056bd08403ff866512038883e74f3a4bf88ad3eb66838 \
                --hash=sha256:1c3c33ac69cf059bbb9d1a71eeaba76781b450bc307e2291f8a4764d779a6b28 \
                --hash=sha256:12875d118f21cf35604176872447cdb57b07126750a33748bac15e77f90f1f9c \
                --hash=sha256:d3c5c79ab7dfce6d88f1ba639b77e77a17ea33a01b07b99840d6ed08031cb2a7 \
                --hash=sha256:f25ed6e28ddf50de7e7ea99d7a976d6a9c415f03adcaac9c41ff6ff41b6d86ac
  # [/Pillow]
  # [retrying]
  retrying==1.3.3 --hash=sha256:08c039560a6da2fe4f2c426d0766e284d3b736e355f8dd24b37367b0bb41973b
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.54.4 --hash=sha256:397cb05982258fde33bd059efecd74893350826f9fe71f36a9500e98baf6f96d
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

