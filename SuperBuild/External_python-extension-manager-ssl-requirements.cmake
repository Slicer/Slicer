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
  PyJWT==2.3.0 --hash=sha256:e0c4bb8d9f0af0c7f5b1ec4c5036309617d03d56932877f2f7a0beeb5318322f
  # [/PyJWT]
  # [wrapt]
  # Hashes correspond to the following packages:
  #  - wrapt-1.13.3-cp39-cp39-macosx_10_9_x86_64.whl
  #  - wrapt-1.13.3-cp39-cp39-manylinux_2_5_x86_64.manylinux1_x86_64.manylinux_2_12_x86_64.manylinux2010_x86_64.whl
  #  - wrapt-1.13.3-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - wrapt-1.13.3-cp39-cp39-win_amd64.whl
  wrapt==1.13.3 --hash=sha256:981da26722bebb9247a0601e2922cedf8bb7a600e89c852d063313102de6f2cb \
                --hash=sha256:25b1b1d5df495d82be1c9d2fad408f7ce5ca8a38085e2da41bb63c914baadff7 \
                --hash=sha256:865c0b50003616f05858b22174c40ffc27a38e67359fa1495605f96125f76640 \
                --hash=sha256:81bd7c90d28a4b2e1df135bfbd7c23aee3050078ca6441bead44c42483f9ebfb
  # [/wrapt]
  # [Deprecated]
  Deprecated==1.2.13 --hash=sha256:64756e3e14c8c5eea9795d93c524551432a0be75629f8f29e67ab8caf076c76d
  # [/Deprecated]
  # [pycparser]
  pycparser==2.21 --hash=sha256:8ee45429555515e1f6b185e78100aea234072576aa43ab53aefcae078162fca9
  # [/pycparser]
  # [cffi]
  # Hashes correspond to the following packages:
  #  - cffi-1.15.0-cp39-cp39-macosx_10_9_x86_64.whl
  #  - cffi-1.15.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - cffi-1.15.0-cp39-cp39-manylinux_2_12_x86_64.manylinux2010_x86_64.whl
  #  - cffi-1.15.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cffi-1.15.0-cp39-cp39-win_amd64.whl
  cffi==1.15.0 --hash=sha256:45e8636704eacc432a206ac7345a5d3d2c62d95a507ec70d62f23cd91770482a \
               --hash=sha256:31fb708d9d7c3f49a60f04cf5b119aeefe5644daba1cd2a0fe389b674fd1de37 \
               --hash=sha256:74fdfdbfdc48d3f47148976f49fab3251e550a8720bebc99bf1483f5bfb5db3e \
               --hash=sha256:ffaa5c925128e29efbde7301d8ecaf35c8c60ffbcd6a1ffd3a552177c8e5e796 \
               --hash=sha256:3773c4d81e6e818df2efbc7dd77325ca0dcb688116050fb2b3011218eda36139
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
  PyGithub==1.55 --hash=sha256:2caf0054ea079b71e539741ae56c5a95e073b81fa472ce222e81667381b9601b
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

