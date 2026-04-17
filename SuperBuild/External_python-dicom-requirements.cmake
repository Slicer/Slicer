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
    highdicom
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
  pydicom==3.0.1 --hash=sha256:db32f78b2641bd7972096b8289111ddab01fb221610de8d7afa835eb938adb41
  # [/pydicom]
  # [six]
  six==1.17.0 --hash=sha256:4721f391ed90541fddacab5acf947aa0d3dc7d27b2e1e8eda2be8970586c3274
  # [/six]
  # [pillow]
  # Hashes correspond to the following packages:
  #  - pillow-12.0.0-cp312-cp312-macosx_10_13_x86_64.whl
  #  - pillow-12.0.0-cp312-cp312-macosx_11_0_arm64.whl
  #  - pillow-12.0.0-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - pillow-12.0.0-cp312-cp312-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - pillow-12.0.0-cp312-cp312-manylinux_2_27_aarch64.manylinux_2_28_aarch64.whl
  #  - pillow-12.0.0-cp312-cp312-manylinux_2_27_x86_64.manylinux_2_28_x86_64.whl
  #  - pillow-12.0.0-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - pillow-12.0.0-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - pillow-12.0.0-cp312-cp312-win_amd64.whl
  #  - pillow-12.0.0-cp312-cp312-win_arm64.whl
  pillow==12.0.0 --hash=sha256:53561a4ddc36facb432fae7a9d8afbfaf94795414f5cdc5fc52f28c1dca90371 \
                 --hash=sha256:71db6b4c1653045dacc1585c1b0d184004f0d7e694c7b34ac165ca70c0838082 \
                 --hash=sha256:2fa5f0b6716fc88f11380b88b31fe591a06c6315e955c096c35715788b339e3f \
                 --hash=sha256:82240051c6ca513c616f7f9da06e871f61bfd7805f566275841af15015b8f98d \
                 --hash=sha256:55f818bd74fe2f11d4d7cbc65880a843c4075e0ac7226bc1a23261dbea531953 \
                 --hash=sha256:b87843e225e74576437fd5b6a4c2205d422754f84a06942cfaf1dc32243e45a8 \
                 --hash=sha256:c607c90ba67533e1b2355b821fef6764d1dd2cbe26b8c1005ae84f7aea25ff79 \
                 --hash=sha256:21f241bdd5080a15bc86d3466a9f6074a9c2c2b314100dd896ac81ee6db2f1ba \
                 --hash=sha256:9fe611163f6303d1619bbcb653540a4d60f9e55e622d60a3108be0d5b441017a \
                 --hash=sha256:7dfb439562f234f7d57b1ac6bc8fe7f838a4bd49c79230e0f6a1da93e82f1fad
  # [/pillow]
  # [retrying]
  retrying==1.4.2 --hash=sha256:bbc004aeb542a74f3569aeddf42a2516efefcdaff90df0eb38fbfbf19f179f59
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.60.1 --hash=sha256:e128866a797b7acdcd4ad280a10ebced5af77a3ce1d1bde709389ad56583955d
  # [/dicomweb-client]
  # [dcmqi]
  # Binary distribution of the DCMQI tools (segimage2itkimage, itkimage2segimage,
  # paramap2itkimage, tid1500reader, tid1500writer, etc.) used by DICOM plugins.
  # Note: macOS wheels require macOS 13+; Linux wheel is manylinux_2_28 x86_64 only.
  # Hashes correspond to the following packages:
  #  - dcmqi-0.3.3-py3-none-macosx_13_0_arm64.whl
  #  - dcmqi-0.3.3-py3-none-macosx_13_0_x86_64.whl
  #  - dcmqi-0.3.3-py3-none-manylinux2014_x86_64.manylinux_2_17_x86_64.manylinux_2_28_x86_64.whl
  #  - dcmqi-0.3.3-py3-none-win32.whl
  #  - dcmqi-0.3.3-py3-none-win_amd64.whl
  dcmqi==0.3.3 --hash=sha256:92f5f942d0349357e511c6a5d3a3703884c3f2127e822e2a204dec8021aa83c4 \
               --hash=sha256:cb007a8991928ff09c864d4c468255f460f6457629a449ad3d52390bf11489f3 \
               --hash=sha256:5a689b17c1163094a73468d8a624611d460ad86969d24ffd22aeccfee7cb4dc1 \
               --hash=sha256:daf19ba9326574eb5df6bed91598e6ee55852637c277e37ff29231936526d5ab \
               --hash=sha256:ca684f09857323389355196cba3c58fce30f4581f1d5220725e2ab08596f6e0f
  # [/dcmqi]
  # [highdicom]
  # Pure-Python package for creating/reading complex DICOM objects (SR, SEG, PM, etc.)
  # Hashes correspond to the following packages:
  #  - highdicom-0.27.0-py3-none-any.whl
  highdicom==0.27.0 --hash=sha256:90a14ef75f67b7df2c3fd83df9c0bddd787dfdfab2b08c8c68828207389aed3f
  # [/highdicom]
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
