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
  #  - charset_normalizer-3.0.1-cp39-cp39-macosx_10_9_universal2.whl
  #  - charset_normalizer-3.0.1-cp39-cp39-macosx_10_9_x86_64.whl
  #  - charset_normalizer-3.0.1-cp39-cp39-macosx_11_0_arm64.whl
  #  - charset_normalizer-3.0.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - charset_normalizer-3.0.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - charset_normalizer-3.0.1-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - charset_normalizer-3.0.1-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - charset_normalizer-3.0.1-cp39-cp39-win_amd64.whl
  #  - charset_normalizer-3.0.1-py3-none-any.whl
  charset-normalizer==3.0.1 --hash=sha256:8eade758719add78ec36dc13201483f8e9b5d940329285edcd5f70c0a9edbd7f \
                            --hash=sha256:8499ca8f4502af841f68135133d8258f7b32a53a1d594aa98cc52013fff55678 \
                            --hash=sha256:3fc1c4a2ffd64890aebdb3f97e1278b0cc72579a08ca4de8cd2c04799a3a22be \
                            --hash=sha256:00d3ffdaafe92a5dc603cb9bd5111aaa36dfa187c8285c543be562e61b755f6b \
                            --hash=sha256:3ae1de54a77dc0d6d5fcf623290af4266412a7c4be0b1ff7444394f03f5c54e3 \
                            --hash=sha256:ab5de034a886f616a5668aa5d098af2b5385ed70142090e2a31bcbd0af0fdb3d \
                            --hash=sha256:356541bf4381fa35856dafa6a965916e54bed415ad8a24ee6de6e37deccf2786 \
                            --hash=sha256:0a11e971ed097d24c534c037d298ad32c6ce81a45736d31e0ff0ad37ab437d59 \
                            --hash=sha256:7e189e2e1d3ed2f4aebabd2d5b0f931e883676e51c7624826e0a4e5fe8a0bf24
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

