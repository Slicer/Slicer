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
  #  - cryptography-41.0.4-cp37-abi3-macosx_10_12_universal2.whl
  #  - cryptography-41.0.4-cp37-abi3-macosx_10_12_x86_64.whl
  #  - cryptography-41.0.4-cp37-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cryptography-41.0.4-cp37-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cryptography-41.0.4-cp37-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-41.0.4-cp37-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-41.0.4-cp37-abi3-musllinux_1_1_aarch64.whl
  #  - cryptography-41.0.4-cp37-abi3-musllinux_1_1_x86_64.whl
  #  - cryptography-41.0.4-cp37-abi3-win_amd64.whl
  cryptography==41.0.4 --hash=sha256:80907d3faa55dc5434a16579952ac6da800935cd98d14dbd62f6f042c7f5e839 \
                       --hash=sha256:35c00f637cd0b9d5b6c6bd11b6c3359194a8eba9c46d4e875a3660e3b400005f \
                       --hash=sha256:cecfefa17042941f94ab54f769c8ce0fe14beff2694e9ac684176a2535bf9714 \
                       --hash=sha256:e40211b4923ba5a6dc9769eab704bdb3fbb58d56c5b336d30996c24fcf12aadb \
                       --hash=sha256:23a25c09dfd0d9f28da2352503b23e086f8e78096b9fd585d1d14eca01613e13 \
                       --hash=sha256:2ed09183922d66c4ec5fdaa59b4d14e105c084dd0febd27452de8f6f74704143 \
                       --hash=sha256:5a0f09cefded00e648a127048119f77bc2b2ec61e736660b5789e638f43cc397 \
                       --hash=sha256:9eeb77214afae972a00dee47382d2591abe77bdae166bda672fb1e24702a3860 \
                       --hash=sha256:c880eba5175f4307129784eca96f4e70b88e57aa3f680aeba3bab0e980b0f37d
  # [/cryptography]
  # Specifying "[crypto]" is required since PyGithub 1.58.1
  # See https://github.com/Slicer/Slicer/issues/7112
  # [PyJWT]
  PyJWT[crypto]==2.8.0 --hash=sha256:59127c392cc44c2da5bb3192169a91f429924e17aff6534d70fdc02ab3e04320
  # [/PyJWT]
  # [wrapt]
  # Hashes correspond to the following packages:
  #  - wrapt-1.15.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - wrapt-1.15.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - wrapt-1.15.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - wrapt-1.15.0-cp39-cp39-manylinux_2_5_x86_64.manylinux1_x86_64.manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - wrapt-1.15.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - wrapt-1.15.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - wrapt-1.15.0-cp39-cp39-win_amd64.whl
  #  - wrapt-1.15.0-py3-none-any.whl
  wrapt==1.15.0 --hash=sha256:2e51de54d4fb8fb50d6ee8327f9828306a959ae394d3e01a1ba8b2f937747d86 \
                --hash=sha256:0970ddb69bba00670e58955f8019bec4a42d1785db3faa043c33d81de2bf843c \
                --hash=sha256:76407ab327158c510f44ded207e2f76b657303e17cb7a572ffe2f5a8a48aa04d \
                --hash=sha256:9d37ac69edc5614b90516807de32d08cb8e7b12260a285ee330955604ed9dd29 \
                --hash=sha256:078e2a1a86544e644a68422f881c48b84fef6d18f8c7a957ffd3f2e0a74a0d4a \
                --hash=sha256:7dc0713bf81287a00516ef43137273b23ee414fe41a3c14be10dd95ed98a2df9 \
                --hash=sha256:eef4d64c650f33347c1f9266fa5ae001440b232ad9b98f1f43dfe7a79435c0a6 \
                --hash=sha256:64b1df0f83706b4ef4cfb4fb0e4c2669100fd7ecacfb59e091fad300d4e04640
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
  # [typing-extensions]
  typing-extensions==4.8.0 --hash=sha256:8f92fc8806f9a6b641eaa5318da32b44d401efaac0f6678c9bc448ba3605faa0
  # [/typing-extensions]
  # [PyGithub]
  PyGithub==2.1.1 --hash=sha256:4b528d5d6f35e991ea5fd3f942f58748f24938805cb7fcf24486546637917337
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

  ExternalProject_GenerateProjectDescription_Step(${proj}
    VERSION ${_version}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

