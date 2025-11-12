set(proj python-scipy)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-ensurepip
  python-numpy
  python-pip
  python-setuptools
  python-wheel
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS scipy)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [scipy]
  # Hashes correspond to the following packages:
  #  - scipy-1.16.3-cp312-cp312-macosx_10_14_x86_64.whl
  #  - scipy-1.16.3-cp312-cp312-macosx_12_0_arm64.whl
  #  - scipy-1.16.3-cp312-cp312-macosx_14_0_arm64.whl
  #  - scipy-1.16.3-cp312-cp312-macosx_14_0_x86_64.whl
  #  - scipy-1.16.3-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - scipy-1.16.3-cp312-cp312-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - scipy-1.16.3-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - scipy-1.16.3-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - scipy-1.16.3-cp312-cp312-win_amd64.whl
  #  - scipy-1.16.3-cp312-cp312-win_arm64.whl
  scipy==1.16.3 --hash=sha256:81fc5827606858cf71446a5e98715ba0e11f0dbc83d71c7409d05486592a45d6 \
                --hash=sha256:c97176013d404c7346bf57874eaac5187d969293bf40497140b0a2b2b7482e07 \
                --hash=sha256:2b71d93c8a9936046866acebc915e2af2e292b883ed6e2cbe5c34beb094b82d9 \
                --hash=sha256:3d4a07a8e785d80289dfe66b7c27d8634a773020742ec7187b85ccc4b0e7b686 \
                --hash=sha256:0553371015692a898e1aa858fed67a3576c34edefa6b7ebdb4e9dde49ce5c203 \
                --hash=sha256:72d1717fd3b5e6ec747327ce9bda32d5463f472c9dce9f54499e81fbd50245a1 \
                --hash=sha256:1fb2472e72e24d1530debe6ae078db70fb1605350c88a3d14bc401d6306dbffe \
                --hash=sha256:c5192722cffe15f9329a3948c4b1db789fbb1f05c97899187dcf009b283aea70 \
                --hash=sha256:56edc65510d1331dae01ef9b658d428e33ed48b4f77b1d51caf479a0253f96dc \
                --hash=sha256:a8a26c78ef223d3e30920ef759e25625a0ecdd0d60e5a8818b7513c3e5384cf2
  # [/scipy]
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
