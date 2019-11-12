set(proj python-dicom-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools python-pip)

set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
file(WRITE ${requirements_file} [===[
pydicom==1.3.0 --hash=sha256:ee3a94de180c6337e1443ce346730a784f9027f56175f61ba4dfda632693b843
# NOTE: The requirements specified below are needed for dicomweb_client to install correctly and not in a project dependency
# Hashes correspond to the following packages:
# - numpy-1.17.3-cp36-cp36m-win_amd64.whl
# - numpy-1.17.3-cp36-cp36m-macosx_10_9_x86_64.whl
# - numpy-1.17.3-cp36-cp36m-manylinux1_x86_64.whl
numpy==1.17.3 --hash=sha256:2e418f0a59473dac424f888dd57e85f77502a593b207809211c76e5396ae4f5c \
              --hash=sha256:669795516d62f38845c7033679c648903200980d68935baaa17ac5c7ae03ae0c \
              --hash=sha256:4f2a2b279efde194877aff1f76cf61c68e840db242a5c7169f1ff0fd59a2b1e2
# Hashes correspond to the following packages:
#  - Pillow-6.2.1-cp36-cp36m-win_amd64.whl
#  - Pillow-6.2.1-cp36-cp36m-macosx_10_6_intel.whl
#  - Pillow-6.2.1-cp36-cp36m-manylinux1_x86_64.whl
pillow==6.2.1 --hash=sha256:83792cb4e0b5af480588601467c0764242b9a483caea71ef12d22a0d0d6bdce2 \
              --hash=sha256:846fa202bd7ee0f6215c897a1d33238ef071b50766339186687bd9b7a6d26ac5 \
              --hash=sha256:e0697b826da6c2472bb6488db4c0a7fa8af0d52fa08833ceb3681358914b14e5
six==1.12.0 --hash=sha256:3350809f0555b11f552448330d0b52d5f24c91a322ea4a15ef22629740f3761c
certifi==2019.9.11 --hash=sha256:fd7c7c74727ddcf00e9acd26bba8da604ffec95bf1c2144e67aff7a8b50e6cef
idna==2.8 --hash=sha256:ea8b7f6188e6fa117537c3df7da9fc686d485087abf6ac197f9c46432f7e4a3c
chardet==3.0.4 --hash=sha256:fc323ffcaeaed0e0a02bf4d117757b98aed530d9ed4531e3e15460124c106691
urllib3==1.25.6 --hash=sha256:3de946ffbed6e6746608990594d08faac602528ac7015ac28d33cee6a45b7398
requests==2.22.0 --hash=sha256:9cf5292fcd0f598c671cfc1e0d7d1a7f13bb8085e9a590f48c010551dc6c4b31
dicomweb_client==0.14.0 --hash=sha256:6391a8a8be33919c2aa869f6538eeeb3d00f2546cb5abd29c8e03bc30cf90e44
]===])

if(NOT DEFINED Slicer_USE_SYSTEM_${proj})
  set(Slicer_USE_SYSTEM_${proj} ${Slicer_USE_SYSTEM_python})
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  foreach(module_name IN ITEMS pydicom numpy pillow six certifi idna chardet urllib3 requests dicomweb_client)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
endif()

if(NOT Slicer_USE_SYSTEM_${proj})

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

