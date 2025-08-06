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
  #  - scipy-1.13.1-cp312-cp312-macosx_10_9_x86_64.whl
  #  - scipy-1.13.1-cp312-cp312-macosx_12_0_arm64.whl
  #  - scipy-1.13.1-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - scipy-1.13.1-cp312-cp312-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - scipy-1.13.1-cp312-cp312-musllinux_1_1_x86_64.whl
  #  - scipy-1.13.1-cp312-cp312-win_amd64.whl
  scipy==1.13.1 --hash=sha256:5d72782f39716b2b3509cd7c33cdc08c96f2f4d2b06d51e52fb45a19ca0c86a1 \
                --hash=sha256:017367484ce5498445aade74b1d5ab377acdc65e27095155e448c88497755a5d \
                --hash=sha256:949ae67db5fa78a86e8fa644b9a6b07252f449dcf74247108c50e1d20d2b4627 \
                --hash=sha256:de3ade0e53bc1f21358aa74ff4830235d716211d7d077e340c7349bc3542e884 \
                --hash=sha256:2ac65fb503dad64218c228e2dc2d0a0193f7904747db43014645ae139c8fad16 \
                --hash=sha256:cdd7dacfb95fea358916410ec61bbc20440f7860333aee6d882bb8046264e949
  # [/scipy]
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
