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
  pydicom==2.4.3 --hash=sha256:797e84f7b22e5f8bce403da505935b0787dca33550891f06495d14b3f6c70504
  # [/pydicom]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [Pillow]
  # Hashes correspond to the following packages:
  #  - Pillow-10.1.0-cp39-cp39-macosx_10_10_x86_64.whl
  #  - Pillow-10.1.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - Pillow-10.1.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - Pillow-10.1.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - Pillow-10.1.0-cp39-cp39-manylinux_2_28_aarch64.whl
  #  - Pillow-10.1.0-cp39-cp39-manylinux_2_28_x86_64.whl
  #  - Pillow-10.1.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - Pillow-10.1.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - Pillow-10.1.0-cp39-cp39-win_amd64.whl
  Pillow==10.1.0 --hash=sha256:0a026c188be3b443916179f5d04548092e253beb0c3e2ee0a4e2cdad72f66099 \
                 --hash=sha256:04f6f6149f266a100374ca3cc368b67fb27c4af9f1cc8cb6306d849dcdf12616 \
                 --hash=sha256:bb40c011447712d2e19cc261c82655f75f32cb724788df315ed992a4d65696bb \
                 --hash=sha256:1a8413794b4ad9719346cd9306118450b7b00d9a15846451549314a58ac42219 \
                 --hash=sha256:c9aeea7b63edb7884b031a35305629a7593272b54f429a9869a4f63a1bf04c34 \
                 --hash=sha256:b4005fee46ed9be0b8fb42be0c20e79411533d1fd58edabebc0dd24626882cfd \
                 --hash=sha256:4d0152565c6aa6ebbfb1e5d8624140a440f2b99bf7afaafbdbf6430426497f28 \
                 --hash=sha256:d921bc90b1defa55c9917ca6b6b71430e4286fc9e44c55ead78ca1a9f9eba5f2 \
                 --hash=sha256:cfe96560c6ce2f4c07d6647af2d0f3c54cc33289894ebd88cfbb3bcd5391e256
  # [/Pillow]
  # [retrying]
  retrying==1.3.4 --hash=sha256:8cc4d43cb8e1125e0ff3344e9de678fefd85db3b750b81b2240dc0183af37b35
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.59.1 --hash=sha256:ad4af95e1bdeb3691841cf8b28894fc6d9ba7ac5b7892bff70a506eedbd20d79
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

