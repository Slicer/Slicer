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
  certifi==2024.2.2 --hash=sha256:dc383c07b76109f368f6106eee2b593b04a011ea4d55f652c6ca24a754d1cdd1
  # [/certifi]
  # [idna]
  idna==3.6 --hash=sha256:c05567e9c24a6b9faaa835c4821bad0590fbb9d5779e7caa6e1cc4978e7eb24f
  # [/idna]
  # [charset-normalizer]
  # Hashes correspond to the following packages:
  #  - charset_normalizer-3.3.2-cp39-cp39-macosx_10_9_universal2.whl
  #  - charset_normalizer-3.3.2-cp39-cp39-macosx_10_9_x86_64.whl
  #  - charset_normalizer-3.3.2-cp39-cp39-macosx_11_0_arm64.whl
  #  - charset_normalizer-3.3.2-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - charset_normalizer-3.3.2-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - charset_normalizer-3.3.2-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - charset_normalizer-3.3.2-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - charset_normalizer-3.3.2-cp39-cp39-win_amd64.whl
  #  - charset_normalizer-3.3.2-py3-none-any.whl
  charset-normalizer==3.3.2 --hash=sha256:c235ebd9baae02f1b77bcea61bce332cb4331dc3617d254df3323aa01ab47bd4 \
                            --hash=sha256:5b4c145409bef602a690e7cfad0a15a55c13320ff7a3ad7ca59c13bb8ba4d45d \
                            --hash=sha256:68d1f8a9e9e37c1223b656399be5d6b448dea850bed7d0f87a8311f1ff3dabb0 \
                            --hash=sha256:22afcb9f253dac0696b5a4be4a1c0f8762f8239e21b99680099abd9b2b1b2269 \
                            --hash=sha256:b261ccdec7821281dade748d088bb6e9b69e6d15b30652b74cbbac25e280b796 \
                            --hash=sha256:d0eccceffcb53201b5bfebb52600a5fb483a20b61da9dbc885f8b103cbe7598c \
                            --hash=sha256:ff8fa367d09b717b2a17a052544193ad76cd49979c805768879cb63d9ca50561 \
                            --hash=sha256:b01b88d45a6fcb69667cd6d2f7a9aeb4bf53760d7fc536bf679ec94fe9f3ff3d \
                            --hash=sha256:3e4d1f6587322d2788836a99c69062fbb091331ec940e02d12d179c1d53e25fc
  # [/charset-normalizer]
  # [urllib3]
  urllib3==2.2.1 --hash=sha256:450b20ec296a467077128bff42b73080516e71b56ff59a60a02bef2232c4fa9d
  # [/urllib3]
  # [requests]
  requests==2.31.0 --hash=sha256:58cd2187c01e70e6e26505bca751777aa9f2ee0b7f4300988b709f44e013003f
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
