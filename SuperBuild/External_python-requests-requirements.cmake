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
  certifi==2025.4.26 --hash=sha256:30350364dfe371162649852c63336a15c70c6510c2ad5015b21c2345311805f3
  # [/certifi]
  # [idna]
  idna==3.10 --hash=sha256:946d195a0d259cbba61165e88e65941f16e9b36ea6ddb97f00452bae8b1287d3
  # [/idna]
  # [charset-normalizer]
  # Hashes correspond to the following packages:
  #  - charset_normalizer-3.4.2-cp312-cp312-macosx_10_13_universal2.whl
  #  - charset_normalizer-3.4.2-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - charset_normalizer-3.4.2-cp312-cp312-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - charset_normalizer-3.4.2-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - charset_normalizer-3.4.2-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - charset_normalizer-3.4.2-cp312-cp312-win_amd64.whl
  #  - charset_normalizer-3.4.2-py3-none-any.whl
  charset-normalizer==3.4.2 --hash=sha256:0c29de6a1a95f24b9a1aa7aefd27d2487263f00dfd55a77719b530788f75cff7 \
                            --hash=sha256:cddf7bd982eaa998934a91f69d182aec997c6c468898efe6679af88283b498d3 \
                            --hash=sha256:4e594135de17ab3866138f496755f302b72157d115086d100c3f19370839dd3a \
                            --hash=sha256:a370b3e078e418187da8c3674eddb9d983ec09445c99a3a263c2011993522981 \
                            --hash=sha256:dedb8adb91d11846ee08bec4c8236c8549ac721c245678282dcb06b221aab59f \
                            --hash=sha256:5a9979887252a82fefd3d3ed2a8e3b937a7a809f65dcb1e068b090e165bbe99e \
                            --hash=sha256:7f56930ab0abd1c45cd15be65cc741c28b1c9a34876ce8c17a2fa107810c0af0
  # [/charset-normalizer]
  # [urllib3]
  urllib3==2.4.0 --hash=sha256:4e16665048960a0900c702d4a66415956a584919c03361cac9f1df5c5dd7e813
  # [/urllib3]
  # [requests]
  requests==2.32.3 --hash=sha256:70761cfe03c773ceb22aa2f671b4757976145175cdfca038c02654d061d6dcc6
  # [/requests]
  ]===])

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${Python3_EXECUTABLE} -m pip install --require-hashes -r ${requirements_file}
    LOG_INSTALL 1
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
