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
  six==1.17.0 --hash=sha256:4721f391ed90541fddacab5acf947aa0d3dc7d27b2e1e8eda2be8970586c3274
  # [/six]
  # [pillow]
  # Hashes correspond to the following packages:
  #  - pillow-11.2.1-cp312-cp312-macosx_10_13_x86_64.whl
  #  - pillow-11.2.1-cp312-cp312-macosx_11_0_arm64.whl
  #  - pillow-11.2.1-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - pillow-11.2.1-cp312-cp312-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - pillow-11.2.1-cp312-cp312-manylinux_2_28_aarch64.whl
  #  - pillow-11.2.1-cp312-cp312-manylinux_2_28_x86_64.whl
  #  - pillow-11.2.1-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - pillow-11.2.1-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - pillow-11.2.1-cp312-cp312-win_amd64.whl
  #  - pillow-11.2.1-cp312-cp312-win_arm64.whl
  pillow==11.2.1 --hash=sha256:78afba22027b4accef10dbd5eed84425930ba41b3ea0a86fa8d20baaf19d807f \
                 --hash=sha256:78092232a4ab376a35d68c4e6d5e00dfd73454bd12b230420025fbe178ee3b0b \
                 --hash=sha256:25a5f306095c6780c52e6bbb6109624b95c5b18e40aab1c3041da3e9e0cd3e2d \
                 --hash=sha256:0c7b29dbd4281923a2bfe562acb734cee96bbb129e96e6972d315ed9f232bef4 \
                 --hash=sha256:3e645b020f3209a0181a418bffe7b4a93171eef6c4ef6cc20980b30bebf17b7d \
                 --hash=sha256:b2dbea1012ccb784a65349f57bbc93730b96e85b42e9bf7b01ef40443db720b4 \
                 --hash=sha256:da3104c57bbd72948d75f6a9389e6727d2ab6333c3617f0a89d72d4940aa0443 \
                 --hash=sha256:598174aef4589af795f66f9caab87ba4ff860ce08cd5bb447c6fc553ffee603c \
                 --hash=sha256:14e33b28bf17c7a38eede290f77db7c664e4eb01f7869e37fa98a5aa95978941 \
                 --hash=sha256:21e1470ac9e5739ff880c211fc3af01e3ae505859392bf65458c224d0bf283eb
  # [/pillow]
  # [retrying]
  retrying==1.3.4 --hash=sha256:8cc4d43cb8e1125e0ff3344e9de678fefd85db3b750b81b2240dc0183af37b35
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.60.0 --hash=sha256:fde6743036f9c9da74580dd643c66121a1650d16737184695bfbe076f3875e71
  # [/dicomweb-client]
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
