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
  pydicom==2.2.1 --hash=sha256:444b5b7289135ff5ea76dfc69d3597dcfde1cd050ca387f709d777f35701242d
  # [/pydicom]
  # [Pillow]
  # Hashes correspond to the following packages:
  #  - Pillow-8.3.2-cp36-cp36m-macosx_10_10_x86_64.whl
  #  - Pillow-8.3.2-cp36-cp36m-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - Pillow-8.3.2-cp36-cp36m-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - Pillow-8.3.2-cp36-cp36m-manylinux_2_5_x86_64.manylinux1_x86_64.whl
  #  - Pillow-8.3.2-cp36-cp36m-win_amd64.whl
  Pillow==8.3.2 --hash=sha256:11eb7f98165d56042545c9e6db3ce394ed8b45089a67124298f0473b29cb60b2 \
                --hash=sha256:2f23b2d3079522fdf3c09de6517f625f7a964f916c956527bed805ac043799b8 \
                --hash=sha256:e5a31c07cea5edbaeb4bdba6f2b87db7d3dc0f446f379d907e51cc70ea375629 \
                --hash=sha256:8f284dc1695caf71a74f24993b7c7473d77bc760be45f776a2c2f4e04c170550 \
                --hash=sha256:a048dad5ed6ad1fad338c02c609b862dfaa921fcd065d747194a6805f91f2196
  # [/Pillow]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [retrying]
  retrying==1.3.3 --hash=sha256:08c039560a6da2fe4f2c426d0766e284d3b736e355f8dd24b37367b0bb41973b
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.53.0 --hash=sha256:f68434122d1ec02fdb35e37ce355d8c511183d79f1752b50cf53517c6d9be1d0
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

