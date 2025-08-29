set(proj python-uv)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-pip
  )

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS uv)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [uv]
  uv==0.5.24 \
    --hash=sha256:13f4f1e36c2b566c19d6fefd09cca5a2a6e8b1cf2ffa16b670351d0eed0c7d4f \
    --hash=sha256:188bacdbe39c7f76ad1999ce0dee0dfd2b79d248e121498bbaf94ba6089fb7ee \
    --hash=sha256:1e933341300f107d228608af9cc23e08c3a386c0077181db23b4cd26535c64d7 \
    --hash=sha256:421304f2b31799f9f926faa75add2b7bbf653e850b9f97c08cb74e5fb5e7f661 \
    --hash=sha256:455d23e0f41577eb9edbdac8e41d37875cb2885758760acee96469084f31571c \
    --hash=sha256:57b473602e7f3f356ba4d2cb5e2c6c7c691f21dbbffd7202c12fc9db29b7ad7a \
    --hash=sha256:6640a2cb919cc04c0fb599d5630a579bacfa8166b928b7e49e3b71280f30f5d9 \
    --hash=sha256:9059775b0c74a68799f3e665a08c429f527d3cdf61c15227992cfff75e31c327 \
    --hash=sha256:a47acfda27654c212af6aeec0b8d12fc0150295b7e6afabbdc741a7eb39d898f \
    --hash=sha256:ac8919fc145aeba090adce202e470c21f2df7dfcfa01ca67a9575d41adbf3de5 \
    --hash=sha256:b7895b66a182bf5e5b88e470ab04beaf76361a64970db6058363293a18c9dd2a \
    --hash=sha256:cfcabf26314411dde69c7a62e9757d5688aa475731c7a3b3749f389488121768 \
    --hash=sha256:d36da9ab6291eaac151695247cdfb3049e6416d6c210c9890ffcda6d0aa6ad19 \
    --hash=sha256:d3a9e63b098055830b58b47552ea4fc38c94a95b5f2de44a8d288ef5decce265 \
    --hash=sha256:d7421d59c80bc805103f1030777f909904feab338620db5b5d81f9c10767304d \
    --hash=sha256:ecb71bcada372274db34bd32f6a9214974b26b6cdc3145a26d07a710f2ea7f18 \
    --hash=sha256:f2ccd95cecfcc9e106b9d8d154f6bfc59d8263952f72a720d0158f3b94dc69cb
  # [/uv]
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

  #-----------------------------------------------------------------------------
  # Slicer Launcher setting specific to build tree

  # environment variables
  set(${proj}_ENVVARS_LAUNCHER_BUILD
          "UV_PYTHON_DOWNLOADS=never"
  )
  mark_as_superbuild(
          VARS ${proj}_ENVVARS_LAUNCHER_BUILD
          LABELS "ENVVARS_LAUNCHER_BUILD"
  )

  #-----------------------------------------------------------------------------
  # Slicer Launcher setting specific to install tree

  # environment variables
  set(${proj}_ENVVARS_LAUNCHER_INSTALLED
          "UV_PYTHON_DOWNLOADS=never"
          # todo UV_CACHE_DIR
  )
  mark_as_superbuild(
          VARS ${proj}_ENVVARS_LAUNCHER_INSTALLED
          LABELS "ENVVARS_LAUNCHER_INSTALLED"
  )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()
