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
  #  - cryptography-42.0.3-cp37-abi3-macosx_10_12_universal2.whl
  #  - cryptography-42.0.3-cp37-abi3-macosx_10_12_x86_64.whl
  #  - cryptography-42.0.3-cp37-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cryptography-42.0.3-cp37-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cryptography-42.0.3-cp37-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-42.0.3-cp37-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-42.0.3-cp37-abi3-musllinux_1_1_aarch64.whl
  #  - cryptography-42.0.3-cp37-abi3-musllinux_1_1_x86_64.whl
  #  - cryptography-42.0.3-cp37-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-42.0.3-cp37-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-42.0.3-cp37-abi3-win_amd64.whl
  #  - cryptography-42.0.3-cp39-abi3-macosx_10_12_universal2.whl
  #  - cryptography-42.0.3-cp39-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cryptography-42.0.3-cp39-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cryptography-42.0.3-cp39-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-42.0.3-cp39-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-42.0.3-cp39-abi3-musllinux_1_1_aarch64.whl
  #  - cryptography-42.0.3-cp39-abi3-musllinux_1_1_x86_64.whl
  #  - cryptography-42.0.3-cp39-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-42.0.3-cp39-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-42.0.3-cp39-abi3-win_amd64.whl
  cryptography==42.0.3 --hash=sha256:de5086cd475d67113ccb6f9fae6d8fe3ac54a4f9238fd08bfdb07b03d791ff0a \
                       --hash=sha256:935cca25d35dda9e7bd46a24831dfd255307c55a07ff38fd1a92119cffc34857 \
                       --hash=sha256:20100c22b298c9eaebe4f0b9032ea97186ac2555f426c3e70670f2517989543b \
                       --hash=sha256:2eb6368d5327d6455f20327fb6159b97538820355ec00f8cc9464d617caecead \
                       --hash=sha256:39d5c93e95bcbc4c06313fc6a500cee414ee39b616b55320c1904760ad686938 \
                       --hash=sha256:3d96ea47ce6d0055d5b97e761d37b4e84195485cb5a38401be341fabf23bc32a \
                       --hash=sha256:d1998e545081da0ab276bcb4b33cce85f775adb86a516e8f55b3dac87f469548 \
                       --hash=sha256:93fbee08c48e63d5d1b39ab56fd3fdd02e6c2431c3da0f4edaf54954744c718f \
                       --hash=sha256:90147dad8c22d64b2ff7331f8d4cddfdc3ee93e4879796f837bdbb2a0b141e0c \
                       --hash=sha256:4dcab7c25e48fc09a73c3e463d09ac902a932a0f8d0c568238b3696d06bf377b \
                       --hash=sha256:762f3771ae40e111d78d77cbe9c1035e886ac04a234d3ee0856bf4ecb3749d54 \
                       --hash=sha256:0d3ec384058b642f7fb7e7bff9664030011ed1af8f852540c76a1317a9dd0d20 \
                       --hash=sha256:35772a6cffd1f59b85cb670f12faba05513446f80352fe811689b4e439b5d89e \
                       --hash=sha256:04859aa7f12c2b5f7e22d25198ddd537391f1695df7057c8700f71f26f47a129 \
                       --hash=sha256:c3d1f5a1d403a8e640fa0887e9f7087331abb3f33b0f2207d2cc7f213e4a864c \
                       --hash=sha256:df34312149b495d9d03492ce97471234fd9037aa5ba217c2a6ea890e9166f151 \
                       --hash=sha256:de4ae486041878dc46e571a4c70ba337ed5233a1344c14a0790c4c4be4bbb8b4 \
                       --hash=sha256:0fab2a5c479b360e5e0ea9f654bcebb535e3aa1e493a715b13244f4e07ea8eec \
                       --hash=sha256:25b09b73db78facdfd7dd0fa77a3f19e94896197c86e9f6dc16bce7b37a96504 \
                       --hash=sha256:d5cf11bc7f0b71fb71af26af396c83dfd3f6eed56d4b6ef95d57867bf1e4ba65 \
                       --hash=sha256:2619487f37da18d6826e27854a7f9d4d013c51eafb066c80d09c63cf24505306
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
  pycparser==2.21 --hash=sha256:8ee45429555515e1f6b185e78100aea234072576aa43ab53aefcae078162fca9
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
  python-dateutil==2.8.2 --hash=sha256:961d03dc3453ebbc59dbdea9e4e11c5651520a876d0f4db161e8674aae935da9
  # [/python-dateutil]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [typing-extensions]
  typing-extensions==4.8.0 --hash=sha256:8f92fc8806f9a6b641eaa5318da32b44d401efaac0f6678c9bc448ba3605faa0
  # [/typing-extensions]
  # [PyGithub]
  PyGithub==2.2.0 --hash=sha256:41042ea53e4c372219db708c38d2ca1fd4fadab75475bac27d89d339596cfad1
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
