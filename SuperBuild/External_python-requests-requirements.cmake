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
  certifi==2023.7.22 --hash=sha256:92d6037539857d8206b8f6ae472e8b77db8058fec5937a1ef3f54304089edbb9
  # [/certifi]
  # [idna]
  idna==3.4 --hash=sha256:90b77e79eaa3eba6de819a0c442c0b4ceefc341a7a2ab77d7562bf49f425c5c2
  # [/idna]
  # [charset-normalizer]
  # Hashes correspond to the following packages:
  #  - charset_normalizer-3.3.0-cp39-cp39-macosx_10_9_universal2.whl
  #  - charset_normalizer-3.3.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - charset_normalizer-3.3.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - charset_normalizer-3.3.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - charset_normalizer-3.3.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - charset_normalizer-3.3.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - charset_normalizer-3.3.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - charset_normalizer-3.3.0-cp39-cp39-win_amd64.whl
  #  - charset_normalizer-3.3.0-py3-none-any.whl
  charset-normalizer==3.3.0 --hash=sha256:e0fc42822278451bc13a2e8626cf2218ba570f27856b536e00cfa53099724828 \
                            --hash=sha256:09c77f964f351a7369cc343911e0df63e762e42bac24cd7d18525961c81754f4 \
                            --hash=sha256:12ebea541c44fdc88ccb794a13fe861cc5e35d64ed689513a5c03d05b53b7c82 \
                            --hash=sha256:805dfea4ca10411a5296bcc75638017215a93ffb584c9e344731eef0dcfb026a \
                            --hash=sha256:619d1c96099be5823db34fe89e2582b336b5b074a7f47f819d6b3a57ff7bdb86 \
                            --hash=sha256:93aa7eef6ee71c629b51ef873991d6911b906d7312c6e8e99790c0f33c576f89 \
                            --hash=sha256:153e7b6e724761741e0974fc4dcd406d35ba70b92bfe3fedcb497226c93b9da7 \
                            --hash=sha256:d97d85fa63f315a8bdaba2af9a6a686e0eceab77b3089af45133252618e70884 \
                            --hash=sha256:e46cd37076971c1040fc8c41273a8b3e2c624ce4f2be3f5dfcb7a430c1d3acc2
  # [/charset-normalizer]
  # [urllib3]
  urllib3==2.0.6 --hash=sha256:7a7c7003b000adf9e7ca2a377c9688bbc54ed41b985789ed576570342a375cd2
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

