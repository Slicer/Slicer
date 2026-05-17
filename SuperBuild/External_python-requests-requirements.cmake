set(proj python-requests-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-pip
  python-setuptools
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS certifi idna chardet urllib3 requests)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [certifi]
  certifi==2026.5.20 --hash=sha256:3c52e209ba0a4ad7aebe60436a4ab349c39e1e602e8c134221e546902ad25897
  # [/certifi]
  # [idna]
  idna==3.16 --hash=sha256:cc246e3a3f89580c3a951b5ad298ca4638078b2cdd4f115654332b5c26daded5
  # [/idna]
  # [charset-normalizer]
  # Hashes correspond to the following packages:
  #  - charset_normalizer-3.4.7-cp312-cp312-macosx_10_13_universal2.whl
  #  - charset_normalizer-3.4.7-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.manylinux_2_28_aarch64.whl
  #  - charset_normalizer-3.4.7-cp312-cp312-manylinux2014_x86_64.manylinux_2_17_x86_64.manylinux_2_28_x86_64.whl
  #  - charset_normalizer-3.4.7-cp312-cp312-manylinux_2_31_riscv64.manylinux_2_39_riscv64.whl
  #  - charset_normalizer-3.4.7-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - charset_normalizer-3.4.7-cp312-cp312-musllinux_1_2_riscv64.whl
  #  - charset_normalizer-3.4.7-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - charset_normalizer-3.4.7-cp312-cp312-win_amd64.whl
  #  - charset_normalizer-3.4.7-cp312-cp312-win_arm64.whl
  #  - charset_normalizer-3.4.7-py3-none-any.whl
  charset-normalizer==3.4.7 --hash=sha256:eca9705049ad3c7345d574e3510665cb2cf844c2f2dcfe675332677f081cbd46 \
                            --hash=sha256:6178f72c5508bfc5fd446a5905e698c6212932f25bcdd4b47a757a50605a90e2 \
                            --hash=sha256:5649fd1c7bade02f320a462fdefd0b4bd3ce036065836d4f42e0de958038e116 \
                            --hash=sha256:298930cec56029e05497a76988377cbd7457ba864beeea92ad7e844fe74cd1f1 \
                            --hash=sha256:708838739abf24b2ceb208d0e22403dd018faeef86ddac04319a62ae884c4f15 \
                            --hash=sha256:aed52fea0513bac0ccde438c188c8a471c4e0f457c2dd20cdbf6ea7a450046c7 \
                            --hash=sha256:bb6d88045545b26da47aa879dd4a89a71d1dce0f0e549b1abcb31dfe4a8eac49 \
                            --hash=sha256:5ed6ab538499c8644b8a3e18debabcd7ce684f3fa91cf867521a7a0279cab2d6 \
                            --hash=sha256:56be790f86bfb2c98fb742ce566dfb4816e5a83384616ab59c49e0604d49c51d \
                            --hash=sha256:3dce51d0f5e7951f8bb4900c257dad282f49190fdbebecd4ba99bcc41fef404d
  # [/charset-normalizer]
  # [urllib3]
  urllib3==2.7.0 --hash=sha256:9fb4c81ebbb1ce9531cce37674bbc6f1360472bc18ca9a553ede278ef7276897
  # [/urllib3]
  # [requests]
  requests==2.34.2 --hash=sha256:2a0d60c172f83ac6ab31e4554906c0f3b3588d37b5cb939b1c061f4907e278e0
  # [/requests]
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
