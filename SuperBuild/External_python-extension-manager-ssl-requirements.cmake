set(proj python-extension-manager-ssl-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
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
  foreach(module_name IN ITEMS jwt wrapt deprecated pycparser cffi nacl dateutil)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
  ExternalProject_FindPythonPackage(
    MODULE_NAME github
    NO_VERSION_PROPERTY
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [cryptography]
  # Hashes correspond to the following packages:
  #  - cryptography-42.0.7-cp37-abi3-macosx_10_12_universal2.whl
  #  - cryptography-42.0.7-cp37-abi3-macosx_10_12_x86_64.whl
  #  - cryptography-42.0.7-cp37-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cryptography-42.0.7-cp37-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cryptography-42.0.7-cp37-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-42.0.7-cp37-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-42.0.7-cp37-abi3-musllinux_1_1_aarch64.whl
  #  - cryptography-42.0.7-cp37-abi3-musllinux_1_1_x86_64.whl
  #  - cryptography-42.0.7-cp37-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-42.0.7-cp37-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-42.0.7-cp37-abi3-win_amd64.whl
  #  - cryptography-42.0.7-cp39-abi3-macosx_10_12_universal2.whl
  #  - cryptography-42.0.7-cp39-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cryptography-42.0.7-cp39-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cryptography-42.0.7-cp39-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-42.0.7-cp39-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-42.0.7-cp39-abi3-musllinux_1_1_aarch64.whl
  #  - cryptography-42.0.7-cp39-abi3-musllinux_1_1_x86_64.whl
  #  - cryptography-42.0.7-cp39-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-42.0.7-cp39-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-42.0.7-cp39-abi3-win_amd64.whl
  cryptography==42.0.7 --hash=sha256:a987f840718078212fdf4504d0fd4c6effe34a7e4740378e59d47696e8dfb477 \
                       --hash=sha256:bd13b5e9b543532453de08bcdc3cc7cebec6f9883e886fd20a92f26940fd3e7a \
                       --hash=sha256:a79165431551042cc9d1d90e6145d5d0d3ab0f2d66326c201d9b0e7f5bf43604 \
                       --hash=sha256:a47787a5e3649008a1102d3df55424e86606c9bae6fb77ac59afe06d234605f8 \
                       --hash=sha256:02c0eee2d7133bdbbc5e24441258d5d2244beb31da5ed19fbb80315f4bbbff55 \
                       --hash=sha256:5e44507bf8d14b36b8389b226665d597bc0f18ea035d75b4e53c7b1ea84583cc \
                       --hash=sha256:7f8b25fa616d8b846aef64b15c606bb0828dbc35faf90566eb139aa9cff67af2 \
                       --hash=sha256:93a3209f6bb2b33e725ed08ee0991b92976dfdcf4e8b38646540674fc7508e13 \
                       --hash=sha256:e6b8f1881dac458c34778d0a424ae5769de30544fc678eac51c1c8bb2183e9da \
                       --hash=sha256:3de9a45d3b2b7d8088c3fbf1ed4395dfeff79d07842217b38df14ef09ce1d8d7 \
                       --hash=sha256:8cb8ce7c3347fcf9446f201dc30e2d5a3c898d009126010cbd1f443f28b52678 \
                       --hash=sha256:a3a5ac8b56fe37f3125e5b72b61dcde43283e5370827f5233893d461b7360cd4 \
                       --hash=sha256:779245e13b9a6638df14641d029add5dc17edbef6ec915688f3acb9e720a5858 \
                       --hash=sha256:0d563795db98b4cd57742a78a288cdbdc9daedac29f2239793071fe114f13785 \
                       --hash=sha256:31adb7d06fe4383226c3e963471f6837742889b3c4caa55aac20ad951bc8ffda \
                       --hash=sha256:efd0bf5205240182e0f13bcaea41be4fdf5c22c5129fc7ced4a0282ac86998c9 \
                       --hash=sha256:a9bc127cdc4ecf87a5ea22a2556cab6c7eda2923f84e4f3cc588e8470ce4e42e \
                       --hash=sha256:3577d029bc3f4827dd5bf8bf7710cac13527b470bbf1820a3f394adb38ed7d5f \
                       --hash=sha256:2e47577f9b18723fa294b0ea9a17d5e53a227867a0a4904a1a076d1646d45ca1 \
                       --hash=sha256:1a58839984d9cb34c855197043eaae2c187d930ca6d644612843b4fe8513c886 \
                       --hash=sha256:16268d46086bb8ad5bf0a2b5544d8a9ed87a0e33f5e77dd3c3301e63d941a83b
  # [/cryptography]
  # Specifying "[crypto]" is required since PyGithub 1.58.1
  # See https://github.com/Slicer/Slicer/issues/7112
  # [PyJWT]
  PyJWT[crypto]==2.8.0 --hash=sha256:59127c392cc44c2da5bb3192169a91f429924e17aff6534d70fdc02ab3e04320
  # [/PyJWT]
  # [wrapt]
  # Hashes correspond to the following packages:
  #  - wrapt-1.16.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - wrapt-1.16.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - wrapt-1.16.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - wrapt-1.16.0-cp39-cp39-manylinux_2_5_x86_64.manylinux1_x86_64.manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - wrapt-1.16.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - wrapt-1.16.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - wrapt-1.16.0-cp39-cp39-win_amd64.whl
  #  - wrapt-1.16.0-py3-none-any.whl
  wrapt==1.16.0 --hash=sha256:9b201ae332c3637a42f02d1045e1d0cccfdc41f1f2f801dafbaa7e9b4797bfc2 \
                --hash=sha256:2076fad65c6736184e77d7d4729b63a6d1ae0b70da4868adeec40989858eb3fb \
                --hash=sha256:c5cd603b575ebceca7da5a3a251e69561bec509e0b46e4993e1cac402b7247b8 \
                --hash=sha256:f8212564d49c50eb4565e502814f694e240c55551a5f1bc841d4fcaabb0a9b8a \
                --hash=sha256:5f15814a33e42b04e3de432e573aa557f9f0f56458745c2074952f564c50e664 \
                --hash=sha256:edfad1d29c73f9b863ebe7082ae9321374ccb10879eeabc84ba3b69f2579d537 \
                --hash=sha256:eb1b046be06b0fce7249f1d025cd359b4b80fc1c3e24ad9eca33e0dcdb2e4a35 \
                --hash=sha256:6906c4100a8fcbf2fa735f6059214bb13b97f75b1a61777fcf6432121ef12ef1
  # [/wrapt]
  # [Deprecated]
  Deprecated==1.2.14 --hash=sha256:6fac8b097794a90302bdbb17b9b815e732d3c4720583ff1b198499d78470466c
  # [/Deprecated]
  # [pycparser]
  pycparser==2.22 --hash=sha256:c3702b6d3dd8c7abc1afa565d7e63d53a1d0bd86cdc24edd75470f4de499cfcc
  # [/pycparser]
  # [cffi]
  # Hashes correspond to the following packages:
  #  - cffi-1.16.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - cffi-1.16.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - cffi-1.16.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cffi-1.16.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cffi-1.16.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - cffi-1.16.0-cp39-cp39-win_amd64.whl
  cffi==1.16.0 --hash=sha256:582215a0e9adbe0e379761260553ba11c58943e4bbe9c36430c4ca6ac74b15ed \
               --hash=sha256:b29ebffcf550f9da55bec9e02ad430c992a87e5f512cd63388abb76f1036d8d2 \
               --hash=sha256:9cb4a35b3642fc5c005a6755a5d17c6c8b6bcb6981baf81cea8bfbc8903e8ba8 \
               --hash=sha256:8f8e709127c6c77446a8c0a8c8bf3c8ee706a06cd44b1e827c3e6a2ee6b8c098 \
               --hash=sha256:8895613bcc094d4a1b2dbe179d88d7fb4a15cee43c052e8885783fac397d91fe \
               --hash=sha256:3686dffb02459559c74dd3d81748269ffb0eb027c39a6fc99502de37d501faa8
  # [/cffi]
  # [PyNaCl]
  # Hashes correspond to the following packages:
  #  - PyNaCl-1.5.0-cp36-abi3-macosx_10_10_universal2.whl
  #  - PyNaCl-1.5.0-cp36-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.manylinux_2_24_aarch64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.manylinux_2_24_x86_64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-musllinux_1_1_aarch64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-musllinux_1_1_x86_64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-win_amd64.whl
  PyNaCl==1.5.0 --hash=sha256:401002a4aaa07c9414132aaed7f6836ff98f59277a234704ff66878c2ee4a0d1 \
                --hash=sha256:52cb72a79269189d4e0dc537556f4740f7f0a9ec41c1322598799b0bdad4ef92 \
                --hash=sha256:a36d4a9dda1f19ce6e03c9a784a2921a4b726b02e1c736600ca9c22029474394 \
                --hash=sha256:0c84947a22519e013607c9be43706dd42513f9e6ae5d39d3613ca1e142fba44d \
                --hash=sha256:06b8f6fa7f5de8d5d2f7573fe8c863c051225a27b61e6860fd047b1775807858 \
                --hash=sha256:a422368fc821589c228f4c49438a368831cb5bbc0eab5ebe1d7fac9dded6567b \
                --hash=sha256:61f642bf2378713e2c2e1de73444a3778e5f0a38be6fee0fe532fe30060282ff \
                --hash=sha256:20f42270d27e1b6a29f54032090b972d97f0a1b0948cc52392041ef7831fee93
  # [/PyNaCl]
  # [python-dateutil]
  python-dateutil==2.9.0.post0 --hash=sha256:a8b2bc7bffae282281c8140a97d3aa9c14da0b136dfe83f850eea9a5f7470427
  # [/python-dateutil]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [typing_extensions]
  typing_extensions==4.12.1 --hash=sha256:6024b58b69089e5a89c347397254e35f1bf02a907728ec7fee9bf0fe837d203a
  # [/typing_extensions]
  # [PyGithub]
  PyGithub==2.3.0 --hash=sha256:65b499728be3ce7b0cd2cd760da3b32f0f4d7bc55e5e0677617f90f6564e793e
  # [/PyGithub]
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
