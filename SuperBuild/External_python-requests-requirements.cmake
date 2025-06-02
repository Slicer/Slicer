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
  certifi==2025.4.26 --hash=sha256:30350364dfe371162649852c63336a15c70c6510c2ad5015b21c2345311805f3
  # [/certifi]
  # [idna]
  idna==3.10 --hash=sha256:946d195a0d259cbba61165e88e65941f16e9b36ea6ddb97f00452bae8b1287d3
  # [/idna]
  # [charset-normalizer]
  # Hashes correspond to the following packages:
  #  - charset_normalizer-3.4.2-cp39-cp39-macosx_10_9_universal2.whl
  #  - charset_normalizer-3.4.2-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - charset_normalizer-3.4.2-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - charset_normalizer-3.4.2-cp39-cp39-musllinux_1_2_aarch64.whl
  #  - charset_normalizer-3.4.2-cp39-cp39-musllinux_1_2_x86_64.whl
  #  - charset_normalizer-3.4.2-cp39-cp39-win_amd64.whl
  #  - charset_normalizer-3.4.2-py3-none-any.whl
  charset-normalizer==3.4.2 --hash=sha256:005fa3432484527f9732ebd315da8da8001593e2cf46a3d817669f062c3d9ed4 \
                            --hash=sha256:e92fca20c46e9f5e1bb485887d074918b13543b1c2a1185e69bb8d17ab6236a7 \
                            --hash=sha256:82d8fd25b7f4675d0c47cf95b594d4e7b158aca33b76aa63d07186e13c0e0ab7 \
                            --hash=sha256:dccab8d5fa1ef9bfba0590ecf4d46df048d18ffe3eec01eeb73a42e0d9e7a8ba \
                            --hash=sha256:982bb1e8b4ffda883b3d0a521e23abcd6fd17418f6d2c4118d257a10199c0ce3 \
                            --hash=sha256:d11b54acf878eef558599658b0ffca78138c8c3655cf4f3a4a673c437e67732e \
                            --hash=sha256:7f56930ab0abd1c45cd15be65cc741c28b1c9a34876ce8c17a2fa107810c0af0
  # [/charset-normalizer]
  # [urllib3]
  urllib3==2.4.0 --hash=sha256:4e16665048960a0900c702d4a66415956a584919c03361cac9f1df5c5dd7e813
  # [/urllib3]
  # [requests]
  requests==2.32.3 --hash=sha256:70761cfe03c773ceb22aa2f671b4757976145175cdfca038c02654d061d6dcc6
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
