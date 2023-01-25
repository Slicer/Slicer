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
  #  - wrapt-1.14.1-cp39-cp39-macosx_10_9_x86_64.whl
  #  - wrapt-1.14.1-cp39-cp39-macosx_11_0_arm64.whl
  #  - wrapt-1.14.1-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - wrapt-1.14.1-cp39-cp39-manylinux_2_5_x86_64.manylinux1_x86_64.manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - wrapt-1.14.1-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - wrapt-1.14.1-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - wrapt-1.14.1-cp39-cp39-win_amd64.whl
  wrapt==1.14.1 --hash=sha256:3232822c7d98d23895ccc443bbdf57c7412c5a65996c30442ebe6ed3df335383 \
                --hash=sha256:988635d122aaf2bdcef9e795435662bcd65b02f4f4c1ae37fbee7401c440b3a7 \
                --hash=sha256:9cca3c2cdadb362116235fdbd411735de4328c61425b0aa9f872fd76d02c4e86 \
                --hash=sha256:40e7bc81c9e2b2734ea4bc1aceb8a8f0ceaac7c5299bc5d69e37c44d9081d43b \
                --hash=sha256:b9b7a708dd92306328117d8c4b62e2194d00c365f18eff11a9b53c6f923b01e3 \
                --hash=sha256:34aa51c45f28ba7f12accd624225e2b1e5a3a45206aa191f6f9aac931d9d56fe \
                --hash=sha256:dee60e1de1898bde3b238f18340eec6148986da0455d8ba7848d50470a7a32fb
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
  PyGithub==1.57 --hash=sha256:5822febeac2391f1306c55a99af2bc8f86c8bf82ded000030cd02c18f31b731f
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

