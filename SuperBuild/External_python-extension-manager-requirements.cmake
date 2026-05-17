set(proj python-extension-manager-requirements)

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
  foreach(module_name IN ITEMS
    chardet
    couchdb
    git
    gitdb
    six
    smmap
    )
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [chardet]
  # Hashes correspond to the following packages:
  #  - chardet-7.4.3-cp312-cp312-macosx_10_13_x86_64.whl
  #  - chardet-7.4.3-cp312-cp312-macosx_11_0_arm64.whl
  #  - chardet-7.4.3-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.manylinux_2_28_aarch64.whl
  #  - chardet-7.4.3-cp312-cp312-manylinux2014_x86_64.manylinux_2_17_x86_64.manylinux_2_28_x86_64.whl
  #  - chardet-7.4.3-cp312-cp312-manylinux_2_31_riscv64.manylinux_2_39_riscv64.whl
  #  - chardet-7.4.3-cp312-cp312-win_amd64.whl
  #  - chardet-7.4.3-py3-none-any.whl
  chardet==7.4.3 --hash=sha256:75d3c65cc16bddf40b8da1fd25ba84fca5f8070f2b14e86083653c1c85aee971 \
                 --hash=sha256:29af5999f654e8729d251f1724a62b538b1262d9292cccaefddf8a02aae1ef6a \
                 --hash=sha256:626f00299ad62dfe937058a09572beed442ccc7b58f87aa667949b20fd3db235 \
                 --hash=sha256:9a4904dd5f071b7a7d7f50b4a67a86db3c902d243bf31708f1d5cde2f68239cb \
                 --hash=sha256:5d2879598bc220689e8ce509fe9c3f37ad2fca53a36be9c9bd91abdd91dd364f \
                 --hash=sha256:4b2799bd58e7245cfa8d4ab2e8ad1d76a5c3a5b1f32318eb6acca4c69a3e7101 \
                 --hash=sha256:1173b74051570cf08099d7429d92e4882d375ad4217f92a6e5240ccfb26f231e
  # [/chardet]
  # [CouchDB]
  couchdb==1.2 --hash=sha256:13a28a1159c49f8346732e8724b9a4d65cba54bec017c4a7eeb1499fe88151d1
  # [/CouchDB]
  # [gitdb]
  gitdb==4.0.12 --hash=sha256:67073e15955400952c6565cc3e707c554a4eea2e428946f7a4c162fab9bd9bcf
  # [/gitdb]
  # [smmap]
  smmap==5.0.3 --hash=sha256:c106e05d5a61449cf6ba9a1e650227ecfb141590d2a98412103ff35d89fc7b2f
  # [/smmap]
  # [GitPython]
  GitPython==3.1.50 --hash=sha256:d352abe2908d07355014abdd21ddf798c2a961469239afec4962e9da884858f9
  # [/GitPython]
  # [six]
  six==1.17.0 --hash=sha256:4721f391ed90541fddacab5acf947aa0d3dc7d27b2e1e8eda2be8970586c3274
  # [/six]
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
