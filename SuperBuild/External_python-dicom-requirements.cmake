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
  pydicom==2.4.1 --hash=sha256:301cbf8bf2cca95643ccf678f5b0cfecfd15974cd27e199f0856c44694852c0e
  # [/pydicom]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [Pillow]
  # Hashes correspond to the following packages:
  #  - Pillow-10.0.0-cp39-cp39-macosx_10_10_x86_64.whl
  #  - Pillow-10.0.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - Pillow-10.0.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - Pillow-10.0.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - Pillow-10.0.0-cp39-cp39-manylinux_2_28_aarch64.whl
  #  - Pillow-10.0.0-cp39-cp39-manylinux_2_28_x86_64.whl
  #  - Pillow-10.0.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - Pillow-10.0.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - Pillow-10.0.0-cp39-cp39-win_amd64.whl
  Pillow==10.0.0 --hash=sha256:9211e7ad69d7c9401cfc0e23d49b69ca65ddd898976d660a2fa5904e3d7a9baa \
                 --hash=sha256:faaf07ea35355b01a35cb442dd950d8f1bb5b040a7787791a535de13db15ed90 \
                 --hash=sha256:c9f72a021fbb792ce98306ffb0c348b3c9cb967dce0f12a49aa4c3d3fdefa967 \
                 --hash=sha256:9f7c16705f44e0504a3a2a14197c1f0b32a95731d251777dcb060aa83022cb2d \
                 --hash=sha256:76edb0a1fa2b4745fb0c99fb9fb98f8b180a1bbceb8be49b087e0b21867e77d3 \
                 --hash=sha256:368ab3dfb5f49e312231b6f27b8820c823652b7cd29cfbd34090565a015e99ba \
                 --hash=sha256:608bfdee0d57cf297d32bcbb3c728dc1da0907519d1784962c5f0c68bb93e5a3 \
                 --hash=sha256:5c6e3df6bdd396749bafd45314871b3d0af81ff935b2d188385e970052091017 \
                 --hash=sha256:7be600823e4c8631b74e4a0d38384c73f680e6105a7d3c6824fcf226c178c7e6
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

