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
  certifi==2022.12.7 --hash=sha256:4ad3232f5e926d6718ec31cfc1fcadfde020920e278684144551c91769c7bc18
  # [/certifi]
  # [idna]
  idna==3.4 --hash=sha256:90b77e79eaa3eba6de819a0c442c0b4ceefc341a7a2ab77d7562bf49f425c5c2
  # [/idna]
  # [charset-normalizer]
  # Hashes correspond to the following packages:
  #  - charset_normalizer-3.1.0-cp39-cp39-macosx_10_9_universal2.whl
  #  - charset_normalizer-3.1.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - charset_normalizer-3.1.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - charset_normalizer-3.1.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - charset_normalizer-3.1.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - charset_normalizer-3.1.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - charset_normalizer-3.1.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - charset_normalizer-3.1.0-cp39-cp39-win_amd64.whl
  #  - charset_normalizer-3.1.0-py3-none-any.whl
  charset-normalizer==3.1.0 --hash=sha256:38e812a197bf8e71a59fe55b757a84c1f946d0ac114acafaafaf21667a7e169e \
                            --hash=sha256:6baf0baf0d5d265fa7944feb9f7451cc316bfe30e8df1a61b1bb08577c554f31 \
                            --hash=sha256:8f25e17ab3039b05f762b0a55ae0b3632b2e073d9c8fc88e89aca31a6198e88f \
                            --hash=sha256:3747443b6a904001473370d7810aa19c3a180ccd52a7157aacc264a5ac79265e \
                            --hash=sha256:21fa558996782fc226b529fdd2ed7866c2c6ec91cee82735c98a197fae39f706 \
                            --hash=sha256:ac3775e3311661d4adace3697a52ac0bab17edd166087d493b52d4f4f553f9f0 \
                            --hash=sha256:53d0a3fa5f8af98a1e261de6a3943ca631c526635eb5817a87a59d9a57ebf48f \
                            --hash=sha256:830d2948a5ec37c386d3170c483063798d7879037492540f10a475e3fd6f244b \
                            --hash=sha256:3d9098b479e78c85080c98e1e35ff40b4a31d8953102bb0fd7d1b6f8a2111a3d
  # [/charset-normalizer]
  # [urllib3]
  urllib3==1.26.14 --hash=sha256:75edcdc2f7d85b137124a6c3c9fc3933cdeaa12ecb9a6a959f22797a0feca7e1
  # [/urllib3]
  # [requests]
  requests==2.28.2 --hash=sha256:64299f4909223da747622c030b781c0d7811e359c37124b4bd368fb8c6518baa
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

