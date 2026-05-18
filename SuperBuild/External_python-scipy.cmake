set(proj python-scipy)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-ensurepip
  python-numpy
  python-pip
  python-setuptools
  python-wheel
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS scipy)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [scipy]
  # Hashes correspond to the following packages:
  #  - scipy-1.17.1-cp312-cp312-macosx_10_14_x86_64.whl
  #  - scipy-1.17.1-cp312-cp312-macosx_12_0_arm64.whl
  #  - scipy-1.17.1-cp312-cp312-macosx_14_0_arm64.whl
  #  - scipy-1.17.1-cp312-cp312-macosx_14_0_x86_64.whl
  #  - scipy-1.17.1-cp312-cp312-manylinux_2_27_aarch64.manylinux_2_28_aarch64.whl
  #  - scipy-1.17.1-cp312-cp312-manylinux_2_27_x86_64.manylinux_2_28_x86_64.whl
  #  - scipy-1.17.1-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - scipy-1.17.1-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - scipy-1.17.1-cp312-cp312-win_amd64.whl
  #  - scipy-1.17.1-cp312-cp312-win_arm64.whl
  scipy==1.17.1 --hash=sha256:35c3a56d2ef83efc372eaec584314bd0ef2e2f0d2adb21c55e6ad5b344c0dcb8 \
                --hash=sha256:fcb310ddb270a06114bb64bbe53c94926b943f5b7f0842194d585c65eb4edd76 \
                --hash=sha256:cc90d2e9c7e5c7f1a482c9875007c095c3194b1cfedca3c2f3291cdc2bc7c086 \
                --hash=sha256:c80be5ede8f3f8eded4eff73cc99a25c388ce98e555b17d31da05287015ffa5b \
                --hash=sha256:e19ebea31758fac5893a2ac360fedd00116cbb7628e650842a6691ba7ca28a21 \
                --hash=sha256:02ae3b274fde71c5e92ac4d54bc06c42d80e399fec704383dcd99b301df37458 \
                --hash=sha256:8a604bae87c6195d8b1045eddece0514d041604b14f2727bbc2b3020172045eb \
                --hash=sha256:f590cd684941912d10becc07325a3eeb77886fe981415660d9265c4c418d0bea \
                --hash=sha256:41b71f4a3a4cab9d366cd9065b288efc4d4f3c0b37a91a8e0947fb5bd7f31d87 \
                --hash=sha256:f4115102802df98b2b0db3cce5cb9b92572633a1197c77b7553e5203f284a5b3
  # [/scipy]
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
