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
  certifi==2023.5.7 --hash=sha256:c6c2e98f5c7869efca1f8916fed228dd91539f9f1b444c314c06eef02980c716
  # [/certifi]
  # [idna]
  idna==3.4 --hash=sha256:90b77e79eaa3eba6de819a0c442c0b4ceefc341a7a2ab77d7562bf49f425c5c2
  # [/idna]
  # [charset-normalizer]
  # Hashes correspond to the following packages:
  #  - charset_normalizer-3.2.0-cp39-cp39-macosx_10_9_universal2.whl
  #  - charset_normalizer-3.2.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - charset_normalizer-3.2.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - charset_normalizer-3.2.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - charset_normalizer-3.2.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - charset_normalizer-3.2.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - charset_normalizer-3.2.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - charset_normalizer-3.2.0-cp39-cp39-win_amd64.whl
  #  - charset_normalizer-3.2.0-py3-none-any.whl
  charset-normalizer==3.2.0 --hash=sha256:855eafa5d5a2034b4621c74925d89c5efef61418570e5ef9b37717d9c796419c \
                            --hash=sha256:203f0c8871d5a7987be20c72442488a0b8cfd0f43b7973771640fc593f56321f \
                            --hash=sha256:e857a2232ba53ae940d3456f7533ce6ca98b81917d47adc3c7fd55dad8fab858 \
                            --hash=sha256:5e86d77b090dbddbe78867a0275cb4df08ea195e660f1f7f13435a4649e954e5 \
                            --hash=sha256:8700f06d0ce6f128de3ccdbc1acaea1ee264d2caa9ca05daaf492fde7c2a7200 \
                            --hash=sha256:c1c76a1743432b4b60ab3358c937a3fe1341c828ae6194108a94c69028247f22 \
                            --hash=sha256:1249cbbf3d3b04902ff081ffbb33ce3377fa6e4c7356f759f3cd076cc138d020 \
                            --hash=sha256:7095f6fbfaa55defb6b733cfeb14efaae7a29f0b59d8cf213be4e7ca0b857b80 \
                            --hash=sha256:8e098148dd37b4ce3baca71fb394c81dc5d9c7728c95df695d2dca218edf40e6
  # [/charset-normalizer]
  # [urllib3]
  urllib3==2.0.3 --hash=sha256:48e7fafa40319d358848e1bc6809b208340fafe2096f1725d05d67443d0483d1
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

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${_version}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

