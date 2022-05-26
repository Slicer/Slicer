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
  #  - Pillow-9.2.0-cp39-cp39-macosx_10_10_x86_64.whl
  #  - Pillow-9.2.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - Pillow-9.2.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - Pillow-9.2.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - Pillow-9.2.0-cp39-cp39-manylinux_2_28_aarch64.whl
  #  - Pillow-9.2.0-cp39-cp39-manylinux_2_28_x86_64.whl
  #  - Pillow-9.2.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - Pillow-9.2.0-cp39-cp39-win_amd64.whl
  Pillow==9.2.0 --hash=sha256:0ed2c4ef2451de908c90436d6e8092e13a43992f1860275b4d8082667fbb2ffc \
                --hash=sha256:4ad2f835e0ad81d1689f1b7e3fbac7b01bb8777d5a985c8962bedee0cc6d43da \
                --hash=sha256:ea98f633d45f7e815db648fd7ff0f19e328302ac36427343e4432c84432e7ff4 \
                --hash=sha256:9a54614049a18a2d6fe156e68e188da02a046a4a93cf24f373bffd977e943421 \
                --hash=sha256:5aed7dde98403cd91d86a1115c78d8145c83078e864c1de1064f52e6feb61b20 \
                --hash=sha256:13b725463f32df1bfeacbf3dd197fb358ae8ebcd8c5548faa75126ea425ccb60 \
                --hash=sha256:808add66ea764ed97d44dda1ac4f2cfec4c1867d9efb16a33d158be79f32b8a4 \
                --hash=sha256:fac2d65901fb0fdf20363fbd345c01958a742f2dc62a8dd4495af66e3ff502a4
  # [/Pillow]
  # [retrying]
  retrying==1.3.3 --hash=sha256:08c039560a6da2fe4f2c426d0766e284d3b736e355f8dd24b37367b0bb41973b
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.56.2 --hash=sha256:7cb314f5dc899613ddacbc8481256416793ccf3ff503d0105c508e2116a1639c
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

