set(proj python-extension-manager-ssl-requirements)

# Set dependency list
set(${proj}_DEPENDENCIES
  python
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
  foreach(module_name IN ITEMS jwt wrapt deprecated pycparser cffi nacl dateutil)
    ExternalProject_FindPythonPackage(
      MODULE_NAME "${module_name}"
      REQUIRED
      )
  endforeach()
  ExternalProject_FindPythonPackage(
    MODULE_NAME github
    NO_VERSION_PROPERTY
    REQUIRED
    )
endif()

if(NOT Slicer_USE_SYSTEM_${proj})
  set(requirements_file ${CMAKE_BINARY_DIR}/${proj}-requirements.txt)
  file(WRITE ${requirements_file} [===[
  # [cryptography]
  # Hashes correspond to the following packages:
  #  - cryptography-45.0.3-cp311-abi3-macosx_10_9_universal2.whl
  #  - cryptography-45.0.3-cp311-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cryptography-45.0.3-cp311-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cryptography-45.0.3-cp311-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-45.0.3-cp311-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-45.0.3-cp311-abi3-manylinux_2_34_aarch64.whl
  #  - cryptography-45.0.3-cp311-abi3-manylinux_2_34_x86_64.whl
  #  - cryptography-45.0.3-cp311-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-45.0.3-cp311-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-45.0.3-cp311-abi3-win_amd64.whl
  #  - cryptography-45.0.3-cp37-abi3-macosx_10_9_universal2.whl
  #  - cryptography-45.0.3-cp37-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cryptography-45.0.3-cp37-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cryptography-45.0.3-cp37-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-45.0.3-cp37-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-45.0.3-cp37-abi3-manylinux_2_34_aarch64.whl
  #  - cryptography-45.0.3-cp37-abi3-manylinux_2_34_x86_64.whl
  #  - cryptography-45.0.3-cp37-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-45.0.3-cp37-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-45.0.3-cp37-abi3-win_amd64.whl
  cryptography==45.0.3 --hash=sha256:7573d9eebaeceeb55285205dbbb8753ac1e962af3d9640791d12b36864065e71 \
                       --hash=sha256:d377dde61c5d67eb4311eace661c3efda46c62113ff56bf05e2d679e02aebb5b \
                       --hash=sha256:fae1e637f527750811588e4582988932c222f8251f7b7ea93739acb624e1487f \
                       --hash=sha256:ca932e11218bcc9ef812aa497cdf669484870ecbcf2d99b765d6c27a86000942 \
                       --hash=sha256:2f8f8f0b73b885ddd7f3d8c2b2234a7d3ba49002b0223f58cfde1bedd9563c56 \
                       --hash=sha256:9cc80ce69032ffa528b5e16d217fa4d8d4bb7d6ba8659c1b4d74a1b0f4235fca \
                       --hash=sha256:c824c9281cb628015bfc3c59335163d4ca0540d49de4582d6c2637312907e4b1 \
                       --hash=sha256:5833bb4355cb377ebd880457663a972cd044e7f49585aee39245c0d592904578 \
                       --hash=sha256:9bb5bf55dcb69f7067d80354d0a348368da907345a2c448b0babc4215ccd3497 \
                       --hash=sha256:97787952246a77d77934d41b62fb1b6f3581d83f71b44796a4158d93b8f5c490 \
                       --hash=sha256:c92519d242703b675ccefd0f0562eb45e74d438e001f8ab52d628e885751fb06 \
                       --hash=sha256:c5edcb90da1843df85292ef3a313513766a78fbbb83f584a5a58fb001a5a9d57 \
                       --hash=sha256:38deed72285c7ed699864f964a3f4cf11ab3fb38e8d39cfcd96710cd2b5bb716 \
                       --hash=sha256:5555365a50efe1f486eed6ac7062c33b97ccef409f5970a0b6f205a7cfab59c8 \
                       --hash=sha256:cfd84777b4b6684955ce86156cfb5e08d75e80dc2585e10d69e47f014f0a5342 \
                       --hash=sha256:a2b56de3417fd5f48773ad8e91abaa700b678dc7fe1e0c757e1ae340779acf7b \
                       --hash=sha256:57a6500d459e8035e813bd8b51b671977fb149a8c95ed814989da682314d0782 \
                       --hash=sha256:f22af3c78abfbc7cbcdf2c55d23c3e022e1a462ee2481011d518c7fb9c9f3d65 \
                       --hash=sha256:232954730c362638544758a8160c4ee1b832dc011d2c41a306ad8f7cccc5bb0b \
                       --hash=sha256:d54ae41e6bd70ea23707843021c778f151ca258081586f0cfa31d936ae43d1b2
  # [/cryptography]
  # Specifying "[crypto]" is required since PyGithub 1.58.1
  # See https://github.com/Slicer/Slicer/issues/7112
  # [PyJWT]
  PyJWT==2.10.1 --hash=sha256:dcdd193e30abefd5debf142f9adfcdd2b58004e644f25406ffaebd50bd98dacb
  # [/PyJWT]
  # [wrapt]
  # Hashes correspond to the following packages:
  #  - wrapt-1.17.2-cp312-cp312-macosx_10_13_universal2.whl
  #  - wrapt-1.17.2-cp312-cp312-macosx_10_13_x86_64.whl
  #  - wrapt-1.17.2-cp312-cp312-macosx_11_0_arm64.whl
  #  - wrapt-1.17.2-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - wrapt-1.17.2-cp312-cp312-manylinux_2_5_x86_64.manylinux1_x86_64.manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - wrapt-1.17.2-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - wrapt-1.17.2-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - wrapt-1.17.2-cp312-cp312-win_amd64.whl
  #  - wrapt-1.17.2-py3-none-any.whl
  wrapt==1.17.2 --hash=sha256:d5e2439eecc762cd85e7bd37161d4714aa03a33c5ba884e26c81559817ca0925 \
                --hash=sha256:3fc7cb4c1c744f8c05cd5f9438a3caa6ab94ce8344e952d7c45a8ed59dd88392 \
                --hash=sha256:8fdbdb757d5390f7c675e558fd3186d590973244fab0c5fe63d373ade3e99d40 \
                --hash=sha256:5bb1d0dbf99411f3d871deb6faa9aabb9d4e744d67dcaaa05399af89d847a91d \
                --hash=sha256:bc570b5f14a79734437cb7b0500376b6b791153314986074486e0b0fa8d71d98 \
                --hash=sha256:6d9187b01bebc3875bac9b087948a2bccefe464a7d8f627cf6e48b1bbae30f82 \
                --hash=sha256:e8b2816ebef96d83657b56306152a93909a83f23994f4b30ad4573b00bd11bb9 \
                --hash=sha256:ec89ed91f2fa8e3f52ae53cd3cf640d6feff92ba90d62236a81e4e563ac0e991 \
                --hash=sha256:b18f2d1533a71f069c7f82d524a52599053d4c7166e9dd374ae2136b7f40f7c8
  # [/wrapt]
  # [Deprecated]
  Deprecated==1.2.18 --hash=sha256:bd5011788200372a32418f888e326a09ff80d0214bd961147cfed01b5c018eec
  # [/Deprecated]
  # [pycparser]
  pycparser==2.22 --hash=sha256:c3702b6d3dd8c7abc1afa565d7e63d53a1d0bd86cdc24edd75470f4de499cfcc
  # [/pycparser]
  # [cffi]
  # Hashes correspond to the following packages:
  #  - cffi-1.17.1-cp312-cp312-macosx_10_9_x86_64.whl
  #  - cffi-1.17.1-cp312-cp312-macosx_11_0_arm64.whl
  #  - cffi-1.17.1-cp312-cp312-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - cffi-1.17.1-cp312-cp312-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - cffi-1.17.1-cp312-cp312-musllinux_1_1_aarch64.whl
  #  - cffi-1.17.1-cp312-cp312-musllinux_1_1_x86_64.whl
  #  - cffi-1.17.1-cp312-cp312-win_amd64.whl
  cffi==1.17.1 --hash=sha256:805b4371bf7197c329fcb3ead37e710d1bca9da5d583f5073b799d5c5bd1eee4 \
               --hash=sha256:733e99bc2df47476e3848417c5a4540522f234dfd4ef3ab7fafdf555b082ec0c \
               --hash=sha256:da95af8214998d77a98cc14e3a3bd00aa191526343078b530ceb0bd710fb48a5 \
               --hash=sha256:b62ce867176a75d03a665bad002af8e6d54644fad99a3c70905c543130e39d93 \
               --hash=sha256:386c8bf53c502fff58903061338ce4f4950cbdcb23e2902d86c0f722b786bbe3 \
               --hash=sha256:4ceb10419a9adf4460ea14cfd6bc43d08701f0835e979bf821052f1805850fe8 \
               --hash=sha256:51392eae71afec0d0c8fb1a53b204dbb3bcabcb3c9b807eedf3e1e6ccf2de903
  # [/cffi]
  # [PyNaCl]
  # Hashes correspond to the following packages:
  #  - PyNaCl-1.5.0-cp36-abi3-macosx_10_10_universal2.whl
  #  - PyNaCl-1.5.0-cp36-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.manylinux_2_24_aarch64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-manylinux_2_17_aarch64.manylinux2014_aarch64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.manylinux_2_24_x86_64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-musllinux_1_1_aarch64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-musllinux_1_1_x86_64.whl
  #  - PyNaCl-1.5.0-cp36-abi3-win_amd64.whl
  PyNaCl==1.5.0 --hash=sha256:401002a4aaa07c9414132aaed7f6836ff98f59277a234704ff66878c2ee4a0d1 \
                --hash=sha256:52cb72a79269189d4e0dc537556f4740f7f0a9ec41c1322598799b0bdad4ef92 \
                --hash=sha256:a36d4a9dda1f19ce6e03c9a784a2921a4b726b02e1c736600ca9c22029474394 \
                --hash=sha256:0c84947a22519e013607c9be43706dd42513f9e6ae5d39d3613ca1e142fba44d \
                --hash=sha256:06b8f6fa7f5de8d5d2f7573fe8c863c051225a27b61e6860fd047b1775807858 \
                --hash=sha256:a422368fc821589c228f4c49438a368831cb5bbc0eab5ebe1d7fac9dded6567b \
                --hash=sha256:61f642bf2378713e2c2e1de73444a3778e5f0a38be6fee0fe532fe30060282ff \
                --hash=sha256:20f42270d27e1b6a29f54032090b972d97f0a1b0948cc52392041ef7831fee93
  # [/PyNaCl]
  # [python-dateutil]
  python-dateutil==2.9.0.post0 --hash=sha256:a8b2bc7bffae282281c8140a97d3aa9c14da0b136dfe83f850eea9a5f7470427
  # [/python-dateutil]
  # [six]
  six==1.17.0 --hash=sha256:4721f391ed90541fddacab5acf947aa0d3dc7d27b2e1e8eda2be8970586c3274
  # [/six]
  # [typing_extensions]
  typing_extensions==4.14.0 --hash=sha256:a1514509136dd0b477638fc68d6a91497af5076466ad0fa6c338e44e359944af
  # [/typing_extensions]
  # [PyGithub]
  PyGithub==2.6.1 --hash=sha256:6f2fa6d076ccae475f9fc392cc6cdbd54db985d4f69b8833a28397de75ed6ca3
  # [/PyGithub]
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
