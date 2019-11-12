set(proj python-dicom-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES python python-setuptools python-pip)

set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
file(WRITE ${requirements_file} [===[
pydicom==1.2.2 --hash=sha256:44a11323e9ee5c189da20958c28ca426656778ee7c7942a2126bf0a9f6aa4e95
# NOTE: The requirements specified below are needed for dicomweb_client to install correctly and not in a project dependency
# Hashes correspond to the following packages:
# - numpy-1.16.2-cp36-cp36m-win_amd64.whl
# - numpy-1.16.2-cp36-cp36m-macosx_10_6_intel.macosx_10_9_intel.macosx_10_9_x86_64.macosx_10_10_intel.macosx_10_10_x86_64.whl
# - numpy-1.16.2-cp36-cp36m-manylinux1_x86_64.whl
numpy==1.16.2 --hash=sha256:d20c0360940f30003a23c0adae2fe50a0a04f3e48dc05c298493b51fd6280197 \
              --hash=sha256:bd2834d496ba9b1bdda3a6cf3de4dc0d4a0e7be306335940402ec95132ad063d \
              --hash=sha256:23cc40313036cffd5d1873ef3ce2e949bdee0646c5d6f375bf7ee4f368db2511
# Hashes correspond to the following packages:
#  - Pillow-6.2.0-cp36-cp36m-win_amd64.whl
#  - Pillow-6.2.0-cp36-cp36m-macosx_10_6_intel.macosx_10_9_intel.macosx_10_9_x86_64.macosx_10_10_intel.macosx_10_10_x86_64.whl
#  - Pillow-6.2.0-cp36-cp36m-manylinux1_x86_64.whl
pillow==6.2.0 --hash=sha256:338581b30b908e111be578f0297255f6b57a51358cd16fa0e6f664c9a1f88bff \
              --hash=sha256:ec883b8e44d877bda6f94a36313a1c6063f8b1997aa091628ae2f34c7f97c8d5 \
              --hash=sha256:ceb76935ac4ebdf6d7bc845482a4450b284c6ccfb281e34da51d510658ab34d8
six==1.12.0 --hash=sha256:3350809f0555b11f552448330d0b52d5f24c91a322ea4a15ef22629740f3761c
certifi==2018.11.29 --hash=sha256:993f830721089fef441cdfeb4b2c8c9df86f0c63239f06bd025a76a7daddb033
idna==2.8 --hash=sha256:ea8b7f6188e6fa117537c3df7da9fc686d485087abf6ac197f9c46432f7e4a3c
chardet==3.0.4 --hash=sha256:fc323ffcaeaed0e0a02bf4d117757b98aed530d9ed4531e3e15460124c106691
urllib3==1.24.1 --hash=sha256:61bf29cada3fc2fbefad4fdf059ea4bd1b4a86d2b6d15e1c7c0b582b9752fe39
requests==2.21.0 --hash=sha256:7bf2a778576d825600030a110f3c0e3e8edc51dfaafe1c146e39a2027784957b
dicomweb_client==0.11.0 --hash=sha256:7e92bfdcb8aac7d1ffe059bb77df66dc7276df32aa7e6bc56ab74635ea918739
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

