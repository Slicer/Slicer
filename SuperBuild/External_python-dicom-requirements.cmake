set(proj python-dicom-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
  python-numpy
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
  foreach(module_name IN ITEMS
    pydicom
    six
    )
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
  ExternalProject_FindPythonPackage(
    MODULE_NAME PIL #pillow
    REQUIRED
    )
  ExternalProject_FindPythonPackage(
    MODULE_NAME dicomweb_client
    NO_VERSION_PROPERTY
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [pydicom]
  pydicom==2.4.4 --hash=sha256:f9f8e19b78525be57aa6384484298833e4d06ac1d6226c79459131ddb0bd7c42
  # [/pydicom]
  # [six]
  six==1.16.0 --hash=sha256:8abb2f1d86890a2dfb989f9a77cfcfd3e47c2a354b01111771326f8aa26e0254
  # [/six]
  # [Pillow]
  # Hashes correspond to the following packages:
  #  - pillow-10.2.0-cp39-cp39-macosx_10_10_x86_64.whl
  #  - pillow-10.2.0-cp39-cp39-macosx_11_0_arm64.whl
  #  - pillow-10.2.0-cp39-cp39-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - pillow-10.2.0-cp39-cp39-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - pillow-10.2.0-cp39-cp39-manylinux_2_28_aarch64.whl
  #  - pillow-10.2.0-cp39-cp39-manylinux_2_28_x86_64.whl
  #  - pillow-10.2.0-cp39-cp39-musllinux_1_1_aarch64.whl
  #  - pillow-10.2.0-cp39-cp39-musllinux_1_1_x86_64.whl
  #  - pillow-10.2.0-cp39-cp39-win_amd64.whl
  #  - pillow-10.2.0-cp39-cp39-win_arm64.whl
  Pillow==10.2.0 --hash=sha256:b792a349405fbc0163190fde0dc7b3fef3c9268292586cf5645598b48e63dc67 \
                 --hash=sha256:c570f24be1e468e3f0ce7ef56a89a60f0e05b30a3669a459e419c6eac2c35364 \
                 --hash=sha256:d8ecd059fdaf60c1963c58ceb8997b32e9dc1b911f5da5307aab614f1ce5c2fb \
                 --hash=sha256:c365fd1703040de1ec284b176d6af5abe21b427cb3a5ff68e0759e1e313a5e7e \
                 --hash=sha256:70c61d4c475835a19b3a5aa42492409878bbca7438554a1f89d20d58a7c75c01 \
                 --hash=sha256:b6f491cdf80ae540738859d9766783e3b3c8e5bd37f5dfa0b76abdecc5081f13 \
                 --hash=sha256:9d189550615b4948f45252d7f005e53c2040cea1af5b60d6f79491a6e147eef7 \
                 --hash=sha256:49d9ba1ed0ef3e061088cd1e7538a0759aab559e2e0a80a36f9fd9d8c0c21591 \
                 --hash=sha256:0304004f8067386b477d20a518b50f3fa658a28d44e4116970abfcd94fac34a8 \
                 --hash=sha256:0fb3e7fc88a14eacd303e90481ad983fd5b69c761e9e6ef94c983f91025da869
  # [/Pillow]
  # [retrying]
  retrying==1.3.4 --hash=sha256:8cc4d43cb8e1125e0ff3344e9de678fefd85db3b750b81b2240dc0183af37b35
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.59.1 --hash=sha256:ad4af95e1bdeb3691841cf8b28894fc6d9ba7ac5b7892bff70a506eedbd20d79
  # [/dicomweb-client]
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
