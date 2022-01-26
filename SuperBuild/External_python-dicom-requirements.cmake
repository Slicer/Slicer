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
  pydicom==2.2.2 --hash=sha256:6ecb9c6d56a20b2104099b8ef8fe0f3664d797b08a0e0548fe0311b515b32308
  # [/pydicom]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [Pillow]
  # Hashes correspond to the following packages:
  #  - Pillow-9.0.0-cp39-cp39-macosx_10_10_x86_64.whl
  #  - Pillow-9.0.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - Pillow-9.0.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - Pillow-9.0.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - Pillow-9.0.0-cp39-cp39-win_amd64.whl
  Pillow==9.0.0 --hash=sha256:c2067b3bb0781f14059b112c9da5a91c80a600a97915b4f48b37f197895dd925 \
                --hash=sha256:2d16b6196fb7a54aff6b5e3ecd00f7c0bab1b56eee39214b2b223a9d938c50af \
                --hash=sha256:98cb63ca63cb61f594511c06218ab4394bf80388b3d66cd61d0b1f63ee0ea69f \
                --hash=sha256:3586e12d874ce2f1bc875a3ffba98732ebb12e18fb6d97be482bd62b56803281 \
                --hash=sha256:6579f9ba84a3d4f1807c4aab4be06f373017fc65fff43498885ac50a9b47a553
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

