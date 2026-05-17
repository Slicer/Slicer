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
  pydicom==3.0.2 --hash=sha256:abf971a5440f84dbaf42c4b6758e30e62480902584f8b270b9a5d146e278a07b
  # [/pydicom]
  # [six]
  six==1.17.0 --hash=sha256:4721f391ed90541fddacab5acf947aa0d3dc7d27b2e1e8eda2be8970586c3274
  # [/six]
  # [pillow]
  # Hashes correspond to the following packages:
  #  - pillow-12.2.0-cp312-cp312-macosx_10_13_x86_64.whl
  #  - pillow-12.2.0-cp312-cp312-macosx_11_0_arm64.whl
  #  - pillow-12.2.0-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - pillow-12.2.0-cp312-cp312-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - pillow-12.2.0-cp312-cp312-manylinux_2_27_aarch64.manylinux_2_28_aarch64.whl
  #  - pillow-12.2.0-cp312-cp312-manylinux_2_27_x86_64.manylinux_2_28_x86_64.whl
  #  - pillow-12.2.0-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - pillow-12.2.0-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - pillow-12.2.0-cp312-cp312-win_amd64.whl
  #  - pillow-12.2.0-cp312-cp312-win_arm64.whl
  pillow==12.2.0 --hash=sha256:2d192a155bbcec180f8564f693e6fd9bccff5a7af9b32e2e4bf8c9c69dbad6b5 \
                 --hash=sha256:f3f40b3c5a968281fd507d519e444c35f0ff171237f4fdde090dd60699458421 \
                 --hash=sha256:03e7e372d5240cc23e9f07deca4d775c0817bffc641b01e9c3af208dbd300987 \
                 --hash=sha256:b86024e52a1b269467a802258c25521e6d742349d760728092e1bc2d135b4d76 \
                 --hash=sha256:7371b48c4fa448d20d2714c9a1f775a81155050d383333e0a6c15b1123dda005 \
                 --hash=sha256:62f5409336adb0663b7caa0da5c7d9e7bdbaae9ce761d34669420c2a801b2780 \
                 --hash=sha256:01afa7cf67f74f09523699b4e88c73fb55c13346d212a59a2db1f86b0a63e8c5 \
                 --hash=sha256:fc3d34d4a8fbec3e88a79b92e5465e0f9b842b628675850d860b8bd300b159f5 \
                 --hash=sha256:7f84204dee22a783350679a0333981df803dac21a0190d706a50475e361c93f5 \
                 --hash=sha256:af73337013e0b3b46f175e79492d96845b16126ddf79c438d7ea7ff27783a414
  # [/pillow]
  # [retrying]
  retrying==1.4.2 --hash=sha256:bbc004aeb542a74f3569aeddf42a2516efefcdaff90df0eb38fbfbf19f179f59
  # [/retrying]
  # [dicomweb-client]
  dicomweb-client==0.61.0 --hash=sha256:dc3385f5dbbeb167c521d04a70eac8bf219c4302c14574af05b5bc577a7e22c5
  # [/dicomweb-client]
  # [dcmqi]
  # Hashes correspond to the following packages:
  #  - dcmqi-0.4.0-py3-none-macosx_13_0_arm64.whl
  #  - dcmqi-0.4.0-py3-none-macosx_13_0_x86_64.whl
  #  - dcmqi-0.4.0-py3-none-manylinux2014_x86_64.manylinux_2_17_x86_64.manylinux_2_28_x86_64.whl
  #  - dcmqi-0.4.0-py3-none-win_amd64.whl
  dcmqi==0.4.0 --hash=sha256:2138fc78e79c28449348572eb7ff0483f98f6a71c821814ca3cf28e3357ad848 \
               --hash=sha256:60f6a9e4d4828c5399e932f44b500e4619becbe27dd4e4574cc94a802cbdcd00 \
               --hash=sha256:1721bf1308fa416b00d3f9895c99b2376524a9982a408f15cf281ceef159df5b \
               --hash=sha256:c3cc2c32f196e96017e08b27550c4b05ac84fde411ef743f882eb8e00ab5709a
  # [/dcmqi]
  # [highdicom]
  # Pure-Python package for creating/reading complex DICOM objects (SR, SEG, PM, etc.)
  # Hashes correspond to the following packages:
  #  - highdicom-0.27.0-py3-none-any.whl
  highdicom==0.27.0 --hash=sha256:90a14ef75f67b7df2c3fd83df9c0bddd787dfdfab2b08c8c68828207389aed3f
  # [/highdicom]
  # [pyjpegls]
  # JPEG-LS codec used by highdicom for DICOM image compression.
  # Platform-specific C extension; no win_arm64 wheel is published upstream.
  # Hashes correspond to the following packages:
  #  - pyjpegls-1.5.1-cp312-cp312-macosx_10_13_x86_64.whl
  #  - pyjpegls-1.5.1-cp312-cp312-macosx_11_0_arm64.whl
  #  - pyjpegls-1.5.1-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - pyjpegls-1.5.1-cp312-cp312-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - pyjpegls-1.5.1-cp312-cp312-win_amd64.whl
  #  - pyjpegls-1.5.1.tar.gz (source dist, fallback for platforms without a binary wheel)
  pyjpegls==1.5.1 --hash=sha256:a5430dcd735bc4c0c1eb478510d73a5178a4743cbad1d72113926accdd738921 \
                  --hash=sha256:c3e6355a4bc9d3da46f7d4377793576b261bf60df297cea7167b3556adb98eb7 \
                  --hash=sha256:45630167eb37990701854f882455f4611c24902a2de1d27635076c3fed4a04a7 \
                  --hash=sha256:5e8c7257bfd1700d563c4fdc60a8794519e00da21798221d964271bc5a027a49 \
                  --hash=sha256:14d7cf5e61ffdb9343a888dde5e3f9b7e527cc805c31c7f48a1e2cdb73d9d9fa \
                  --hash=sha256:d0fe09dfb7f75ce78e3b1e0519912a42f1827d68bbc3609652f4f8743c956025
  # [/pyjpegls]
  # [typing-extensions]
  # Pure-Python backport of typing module features; required by highdicom on Python < 3.13.
  # Hashes correspond to the following packages:
  #  - typing_extensions-4.15.0-py3-none-any.whl
  typing_extensions==4.15.0 --hash=sha256:f0fa19c6845758ab08074a0cfa8b7aecb71c999ca73d62883bc25cc018c4e548
  # [/typing-extensions]
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
