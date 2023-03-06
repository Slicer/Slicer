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
  pydicom==2.3.1 --hash=sha256:b00bacdabc1f8a18b61a08bb6cd0f41c419446531638c895a708c22a35d69cfe
  # [/pydicom]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [Pillow]
  # Hashes correspond to the following packages:
  #  - Pillow-9.4.0-1-cp39-cp39-macosx_10_10_x86_64.whl
  #  - Pillow-9.4.0-2-cp39-cp39-macosx_10_10_x86_64.whl
  #  - Pillow-9.4.0-cp39-cp39-macosx_10_10_x86_64.whl
  #  - Pillow-9.4.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - Pillow-9.4.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - Pillow-9.4.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - Pillow-9.4.0-cp39-cp39-manylinux_2_28_aarch64.whl
  #  - Pillow-9.4.0-cp39-cp39-manylinux_2_28_x86_64.whl
  #  - Pillow-9.4.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - Pillow-9.4.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - Pillow-9.4.0-cp39-cp39-win_amd64.whl
  Pillow==9.4.0 --hash=sha256:b8c2f6eb0df979ee99433d8b3f6d193d9590f735cf12274c108bd954e30ca858 \
                --hash=sha256:9e5f94742033898bfe84c93c831a6f552bb629448d4072dd312306bab3bd96f1 \
                --hash=sha256:0f3269304c1a7ce82f1759c12ce731ef9b6e95b6df829dccd9fe42912cc48569 \
                --hash=sha256:cb362e3b0976dc994857391b776ddaa8c13c28a16f80ac6522c23d5257156bed \
                --hash=sha256:a2e0f87144fcbbe54297cae708c5e7f9da21a4646523456b00cc956bd4c65815 \
                --hash=sha256:0884ba7b515163a1a05440a138adeb722b8a6ae2c2b33aea93ea3118dd3a899e \
                --hash=sha256:53dcb50fbdc3fb2c55431a9b30caeb2f7027fcd2aeb501459464f0214200a503 \
                --hash=sha256:e8c5cf126889a4de385c02a2c3d3aba4b00f70234bfddae82a5eaa3ee6d5e3e6 \
                --hash=sha256:6c6b1389ed66cdd174d040105123a5a1bc91d0aa7059c7261d20e583b6d8cbd2 \
                --hash=sha256:0dd4c681b82214b36273c18ca7ee87065a50e013112eea7d78c7a1b89a739153 \
                --hash=sha256:54614444887e0d3043557d9dbc697dbb16cfb5a35d672b7a0fcc1ed0cf1c600b
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

