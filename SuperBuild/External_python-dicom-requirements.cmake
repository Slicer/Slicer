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
  pydicom==2.4.4 --hash=sha256:f9f8e19b78525be57aa6384484298833e4d06ac1d6226c79459131ddb0bd7c42
  # [/pydicom]
  # [six]
  six==1.17.0 --hash=sha256:4721f391ed90541fddacab5acf947aa0d3dc7d27b2e1e8eda2be8970586c3274
  # [/six]
  # [pillow]
  # Hashes correspond to the following packages:
  #  - pillow-11.2.1-cp39-cp39-macosx_10_10_x86_64.whl
  #  - pillow-11.2.1-cp39-cp39-macosx_11_0_arm64.whl
  #  - pillow-11.2.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - pillow-11.2.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - pillow-11.2.1-cp39-cp39-manylinux_2_28_aarch64.whl
  #  - pillow-11.2.1-cp39-cp39-manylinux_2_28_x86_64.whl
  #  - pillow-11.2.1-cp39-cp39-musllinux_1_2_aarch64.whl
  #  - pillow-11.2.1-cp39-cp39-musllinux_1_2_x86_64.whl
  #  - pillow-11.2.1-cp39-cp39-win_amd64.whl
  #  - pillow-11.2.1-cp39-cp39-win_arm64.whl
  pillow==11.2.1 --hash=sha256:7491cf8a79b8eb867d419648fff2f83cb0b3891c8b36da92cc7f1931d46108c8 \
                 --hash=sha256:8b02d8f9cb83c52578a0b4beadba92e37d83a4ef11570a8688bbf43f4ca50909 \
                 --hash=sha256:014ca0050c85003620526b0ac1ac53f56fc93af128f7546623cc8e31875ab928 \
                 --hash=sha256:3692b68c87096ac6308296d96354eddd25f98740c9d2ab54e1549d6c8aea9d79 \
                 --hash=sha256:f781dcb0bc9929adc77bad571b8621ecb1e4cdef86e940fe2e5b5ee24fd33b35 \
                 --hash=sha256:2b490402c96f907a166615e9a5afacf2519e28295f157ec3a2bb9bd57de638cb \
                 --hash=sha256:dd6b20b93b3ccc9c1b597999209e4bc5cf2853f9ee66e3fc9a400a78733ffc9a \
                 --hash=sha256:4b835d89c08a6c2ee7781b8dd0a30209a8012b5f09c0a665b65b0eb3560b6f36 \
                 --hash=sha256:6ebce70c3f486acf7591a3d73431fa504a4e18a9b97ff27f5f47b7368e4b9dd1 \
                 --hash=sha256:c27476257b2fdcd7872d54cfd119b3a9ce4610fb85c8e32b70b42e3680a29a1e
  # [/pillow]
  # [retrying]
  retrying==1.3.4 --hash=sha256:8cc4d43cb8e1125e0ff3344e9de678fefd85db3b750b81b2240dc0183af37b35
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.60.0 --hash=sha256:fde6743036f9c9da74580dd643c66121a1650d16737184695bfbe076f3875e71
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

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
