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
  foreach(module_name IN ITEMS jwt wrapt deprecated pycparser cffi nacl)
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
  # [PyJWT]
  PyJWT==2.6.0 --hash=sha256:d83c3d892a77bbb74d3e1a2cfa90afaadb60945205d1095d9221f04466f64c14
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
  Deprecated==1.2.13 --hash=sha256:64756e3e14c8c5eea9795d93c524551432a0be75629f8f29e67ab8caf076c76d
  # [/Deprecated]
  # [pycparser]
  pycparser==2.21 --hash=sha256:8ee45429555515e1f6b185e78100aea234072576aa43ab53aefcae078162fca9
  # [/pycparser]
  # [cffi]
  # Hashes correspond to the following packages:
  #  - cffi-1.15.1-cp39-cp39-macosx_10_9_x86_64.whl
  #  - cffi-1.15.1-cp39-cp39-macosx_11_0_arm64.whl
  #  - cffi-1.15.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cffi-1.15.1-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cffi-1.15.1-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - cffi-1.15.1-cp39-cp39-win_amd64.whl
  cffi==1.15.1 --hash=sha256:54a2db7b78338edd780e7ef7f9f6c442500fb0d41a5a4ea24fff1c929d5af585 \
               --hash=sha256:fcd131dd944808b5bdb38e6f5b53013c5aa4f334c5cad0c72742f6eba4b73db0 \
               --hash=sha256:6c9a799e985904922a4d207a94eae35c78ebae90e128f0c4e521ce339396be9d \
               --hash=sha256:5d598b938678ebf3c67377cdd45e09d431369c3b1a5b331058c338e201f12b27 \
               --hash=sha256:98d85c6a2bef81588d9227dde12db8a7f47f639f4a17c9ae08e773aa9c697bf3 \
               --hash=sha256:70df4e3b545a17496c9b3f41f5115e69a4f2e77e94e1d2a8e1070bc0c38c8a3c
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
  # [PyGithub]
  PyGithub==1.58.0 --hash=sha256:b7bac601492a2b6c876ef326e4ffa3c1923e32707e415da76bfb8307ee8ffb7e
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

