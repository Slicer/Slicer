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
  certifi==2025.11.12 --hash=sha256:97de8790030bbd5c2d96b7ec782fc2f7820ef8dba6db909ccf95449f2d062d4b
  # [/certifi]
  # [idna]
  idna==3.11 --hash=sha256:771a87f49d9defaf64091e6e6fe9c18d4833f140bd19464795bc32d966ca37ea
  # [/idna]
  # [charset-normalizer]
  # Hashes correspond to the following packages:
  #  - charset_normalizer-3.4.4-cp312-cp312-macosx_10_13_universal2.whl
  #  - charset_normalizer-3.4.4-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.manylinux_2_28_aarch64.whl
  #  - charset_normalizer-3.4.4-cp312-cp312-manylinux2014_x86_64.manylinux_2_17_x86_64.manylinux_2_28_x86_64.whl
  #  - charset_normalizer-3.4.4-cp312-cp312-manylinux_2_31_riscv64.manylinux_2_39_riscv64.whl
  #  - charset_normalizer-3.4.4-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - charset_normalizer-3.4.4-cp312-cp312-musllinux_1_2_riscv64.whl
  #  - charset_normalizer-3.4.4-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - charset_normalizer-3.4.4-cp312-cp312-win_amd64.whl
  #  - charset_normalizer-3.4.4-cp312-cp312-win_arm64.whl
  #  - charset_normalizer-3.4.4-py3-none-any.whl
  charset-normalizer==3.4.4 --hash=sha256:0a98e6759f854bd25a58a73fa88833fba3b7c491169f86ce1180c948ab3fd394 \
                            --hash=sha256:b5b290ccc2a263e8d185130284f8501e3e36c5e02750fc6b6bdeb2e9e96f1e25 \
                            --hash=sha256:11d694519d7f29d6cd09f6ac70028dba10f92f6cdd059096db198c283794ac86 \
                            --hash=sha256:ac1c4a689edcc530fc9d9aa11f5774b9e2f33f9a0c6a57864e90908f5208d30a \
                            --hash=sha256:21d142cc6c0ec30d2efee5068ca36c128a30b0f2c53c1c07bd78cb6bc1d3be5f \
                            --hash=sha256:d055ec1e26e441f6187acf818b73564e6e6282709e9bcb5b63f5b23068356a15 \
                            --hash=sha256:780236ac706e66881f3b7f2f32dfe90507a09e67d1d454c762cf642e6e1586e0 \
                            --hash=sha256:a79cfe37875f822425b89a82333404539ae63dbdddf97f84dcbc3d339aae9525 \
                            --hash=sha256:376bec83a63b8021bb5c8ea75e21c4ccb86e7e45ca4eb81146091b56599b80c3 \
                            --hash=sha256:7a32c560861a02ff789ad905a2fe94e3f840803362c84fecf1851cb4cf3dc37f
  # [/charset-normalizer]
  # [urllib3]
  urllib3==2.6.3 --hash=sha256:bf272323e553dfb2e87d9bfd225ca7b0f467b919d7bbd355436d3fd37cb0acd4
  # [/urllib3]
  # [requests]
  requests==2.32.5 --hash=sha256:2462f94637a34fd532264295e186976db0f5d453d1cdd31473c85a6a161affb6
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
