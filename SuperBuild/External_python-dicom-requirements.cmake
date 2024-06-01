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
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [pillow]
  # Hashes correspond to the following packages:
  #  - pillow-10.3.0-cp39-cp39-macosx_10_10_x86_64.whl
  #  - pillow-10.3.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - pillow-10.3.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - pillow-10.3.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - pillow-10.3.0-cp39-cp39-manylinux_2_28_aarch64.whl
  #  - pillow-10.3.0-cp39-cp39-manylinux_2_28_x86_64.whl
  #  - pillow-10.3.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - pillow-10.3.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - pillow-10.3.0-cp39-cp39-win_amd64.whl
  #  - pillow-10.3.0-cp39-cp39-win_arm64.whl
  pillow==10.3.0 --hash=sha256:2ed854e716a89b1afcedea551cd85f2eb2a807613752ab997b9974aaa0d56936 \
                 --hash=sha256:dc1a390a82755a8c26c9964d457d4c9cbec5405896cba94cf51f36ea0d855002 \
                 --hash=sha256:4203efca580f0dd6f882ca211f923168548f7ba334c189e9eab1178ab840bf60 \
                 --hash=sha256:3102045a10945173d38336f6e71a8dc71bcaeed55c3123ad4af82c52807b9375 \
                 --hash=sha256:6fb1b30043271ec92dc65f6d9f0b7a830c210b8a96423074b15c7bc999975f57 \
                 --hash=sha256:1dfc94946bc60ea375cc39cff0b8da6c7e5f8fcdc1d946beb8da5c216156ddd8 \
                 --hash=sha256:b09b86b27a064c9624d0a6c54da01c1beaf5b6cadfa609cf63789b1d08a797b9 \
                 --hash=sha256:d3b2348a78bc939b4fed6552abfd2e7988e0f81443ef3911a4b8498ca084f6eb \
                 --hash=sha256:0ba26351b137ca4e0db0342d5d00d2e355eb29372c05afd544ebf47c0956ffeb \
                 --hash=sha256:50fd3f6b26e3441ae07b7c979309638b72abc1a25da31a81a7fbd9495713ef4f
  # [/pillow]
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

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
