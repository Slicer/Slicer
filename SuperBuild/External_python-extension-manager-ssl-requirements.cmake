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
  #  - cryptography-48.0.0-cp311-abi3-macosx_10_9_universal2.whl
  #  - cryptography-48.0.0-cp311-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - cryptography-48.0.0-cp311-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - cryptography-48.0.0-cp311-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-48.0.0-cp311-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-48.0.0-cp311-abi3-manylinux_2_34_aarch64.whl
  #  - cryptography-48.0.0-cp311-abi3-manylinux_2_34_x86_64.whl
  #  - cryptography-48.0.0-cp311-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-48.0.0-cp311-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-48.0.0-cp311-abi3-win_amd64.whl
  #  - cryptography-48.0.0-cp39-abi3-macosx_10_9_universal2.whl
  #  - cryptography-48.0.0-cp39-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - cryptography-48.0.0-cp39-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - cryptography-48.0.0-cp39-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-48.0.0-cp39-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-48.0.0-cp39-abi3-manylinux_2_34_aarch64.whl
  #  - cryptography-48.0.0-cp39-abi3-manylinux_2_34_x86_64.whl
  #  - cryptography-48.0.0-cp39-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-48.0.0-cp39-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-48.0.0-cp39-abi3-win_amd64.whl
  cryptography==48.0.0 --hash=sha256:0c558d2cdffd8f4bbb30fc7134c74d2ca9a476f830bb053074498fbc86f41ed6 \
                       --hash=sha256:f5333311663ea94f75dd408665686aaf426563556bb5283554a3539177e03b8c \
                       --hash=sha256:7995ef305d7165c3f11ae07f2517e5a4f1d5c18da1376a0a9ed496336b69e5f3 \
                       --hash=sha256:40ba1f85eaa6959837b1d51c9767e230e14612eea4ef110ee8854ada22da1bf5 \
                       --hash=sha256:a0e692c683f4df67815a2d258b324e66f4738bd7a96a218c826dce4f4bd05d8f \
                       --hash=sha256:7e8eac43dfca5c4cccc6dad9a80504436fca53bb9bc3100a2386d730fbe6b602 \
                       --hash=sha256:bd72e68b06bb1e96913f97dd4901119bc17f39d4586a5adf2d3e47bc2b9d58b5 \
                       --hash=sha256:59baa2cb386c4f0b9905bd6eb4c2a79a69a128408fd31d32ca4d7102d4156321 \
                       --hash=sha256:9249e3cd978541d665967ac2cb2787fd6a62bddf1e75b3e347a594d7dacf4f74 \
                       --hash=sha256:5b012212e08b8dd5edc78ef54da83dd9892fd9105323b3993eff6bea65dc21d7 \
                       --hash=sha256:58d00498e8933e4a194f3076aee1b4a97dfec1a6da444535755822fe5d8b0b86 \
                       --hash=sha256:614d0949f4790582d2cc25553abd09dd723025f0c0e7c67376a1d77196743d6e \
                       --hash=sha256:7ce4bfae76319a532a2dc68f82cc32f5676ee792a983187dac07183690e5c66f \
                       --hash=sha256:2eb992bbd4661238c5a397594c83f5b4dc2bc5b848c365c8f991b6780efcc5c7 \
                       --hash=sha256:2b4d59804e8408e2fea7d1fbaf218e5ec984325221db76e6a241a9abd6cdd95c \
                       --hash=sha256:5a5ed8fde7a1d09376ca0b40e68cd59c69fe23b1f9768bd5824f54681626032a \
                       --hash=sha256:9071196d81abc88b3516ac8cdfad32e2b66dd4a5393a8e68a961e9161ddc6239 \
                       --hash=sha256:1e2d54c8be6152856a36f0882ab231e70f8ec7f14e93cf87db8a2ed056bf160c \
                       --hash=sha256:a5da777e32ffed6f85a7b2b3f7c5cbc88c146bfcd0a1d7baf5fcc6c52ee35dd4 \
                       --hash=sha256:16cd65b9330583e4619939b3a3843eec1e6e789744bb01e7c7e2e62e33c239c8
  # [/cryptography]
  # Specifying "[crypto]" is required since PyGithub 1.58.1
  # See https://github.com/Slicer/Slicer/issues/7112
  # [PyJWT]
  PyJWT==2.13.0 --hash=sha256:66adcc2aff09b3f1bbd95fc1e1577df8ac8723c978552fd43304c8a290ac5728
  # [/PyJWT]
  # [wrapt]
  # Hashes correspond to the following packages:
  #  - wrapt-2.2.1-cp312-cp312-macosx_10_13_x86_64.whl
  #  - wrapt-2.2.1-cp312-cp312-macosx_11_0_arm64.whl
  #  - wrapt-2.2.1-cp312-cp312-manylinux1_x86_64.manylinux_2_28_x86_64.manylinux_2_5_x86_64.whl
  #  - wrapt-2.2.1-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.manylinux_2_28_aarch64.whl
  #  - wrapt-2.2.1-cp312-cp312-manylinux_2_31_riscv64.manylinux_2_39_riscv64.whl
  #  - wrapt-2.2.1-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - wrapt-2.2.1-cp312-cp312-musllinux_1_2_riscv64.whl
  #  - wrapt-2.2.1-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - wrapt-2.2.1-cp312-cp312-win_amd64.whl
  #  - wrapt-2.2.1-cp312-cp312-win_arm64.whl
  #  - wrapt-2.2.1-py3-none-any.whl
  wrapt==2.2.1 --hash=sha256:3ffad790d9d11d8ecf9f17c4bb671a5b4089e4d8b575c46c5129597f41f836b0 \
               --hash=sha256:628f5220c7a904d5fc78f7075c8d7871433eb6d035c94728a22fdf85f193d2a8 \
               --hash=sha256:61acce4257a9883669703c525447c5b4c392edf0f987ae77ec32668440158f0e \
               --hash=sha256:727ab4244622cd6ad2390f322642090c877d2e83a608d2653a7643ae5368d926 \
               --hash=sha256:03df9ebed4c73ab93fa8c07e3d41d818dfca1852b15731a3de59457b27814624 \
               --hash=sha256:0d9ff006f420b2ec8296aa56ade43ea7da3e997e85769f0aafc5e0661aacb710 \
               --hash=sha256:844c858fc3bb7eacc0ba8efa904935d16aac6a4470948ad1e7e55c9f5a2a665f \
               --hash=sha256:87bacdaf225117a342a20d9c03438d701c02112f6e3f351ce9b7f32354f14797 \
               --hash=sha256:6ce32763ac31ce94fe9aada947e479b1975012bff166da409b4b9e4e376cf7e5 \
               --hash=sha256:8d1b4d0e0c2119587a31f5c029abd547e0c81d93b89d394566fe1588659eb579 \
               --hash=sha256:3aafea2975caef8ca49400640dde02cc7426e798f24870ed01f490bc3cffd32f
  # [/wrapt]
  # [Deprecated]
  Deprecated==1.3.1 --hash=sha256:597bfef186b6f60181535a29fbe44865ce137a5079f295b479886c82729d5f3f
  # [/Deprecated]
  # [pycparser]
  pycparser==3.0 --hash=sha256:b727414169a36b7d524c1c3e31839a521725078d7b2ff038656844266160a992
  # [/pycparser]
  # [cffi]
  # Hashes correspond to the following packages:
  #  - cffi-2.0.0-cp312-cp312-macosx_10_13_x86_64.whl
  #  - cffi-2.0.0-cp312-cp312-macosx_11_0_arm64.whl
  #  - cffi-2.0.0-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - cffi-2.0.0-cp312-cp312-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - cffi-2.0.0-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - cffi-2.0.0-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - cffi-2.0.0-cp312-cp312-win_amd64.whl
  #  - cffi-2.0.0-cp312-cp312-win_arm64.whl
  cffi==2.0.0 --hash=sha256:6d02d6655b0e54f54c4ef0b94eb6be0607b70853c45ce98bd278dc7de718be5d \
              --hash=sha256:8eca2a813c1cb7ad4fb74d368c2ffbbb4789d377ee5bb8df98373c2cc0dee76c \
              --hash=sha256:b21e08af67b8a103c71a250401c78d5e0893beff75e28c53c98f4de42f774062 \
              --hash=sha256:3e17ed538242334bf70832644a32a7aae3d83b57567f9fd60a26257e992b79ba \
              --hash=sha256:3925dd22fa2b7699ed2617149842d2e6adde22b262fcbfada50e3d195e4b3a94 \
              --hash=sha256:2c8f814d84194c9ea681642fd164267891702542f028a15fc97d4674b6206187 \
              --hash=sha256:da68248800ad6320861f129cd9c1bf96ca849a2771a59e0344e88681905916f5 \
              --hash=sha256:4671d9dd5ec934cb9a73e7ee9676f9362aba54f7f34910956b84d727b0d73fb6
  # [/cffi]
  # [PyNaCl]
  # Hashes correspond to the following packages:
  #  - pynacl-1.6.2-cp38-abi3-macosx_10_10_universal2.whl
  #  - pynacl-1.6.2-cp38-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - pynacl-1.6.2-cp38-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - pynacl-1.6.2-cp38-abi3-manylinux_2_26_aarch64.manylinux_2_28_aarch64.whl
  #  - pynacl-1.6.2-cp38-abi3-manylinux_2_26_x86_64.manylinux_2_28_x86_64.whl
  #  - pynacl-1.6.2-cp38-abi3-manylinux_2_34_aarch64.whl
  #  - pynacl-1.6.2-cp38-abi3-manylinux_2_34_x86_64.whl
  #  - pynacl-1.6.2-cp38-abi3-musllinux_1_2_aarch64.whl
  #  - pynacl-1.6.2-cp38-abi3-musllinux_1_2_x86_64.whl
  #  - pynacl-1.6.2-cp38-abi3-win_amd64.whl
  #  - pynacl-1.6.2-cp38-abi3-win_arm64.whl
  PyNaCl==1.6.2 --hash=sha256:c949ea47e4206af7c8f604b8278093b674f7c79ed0d4719cc836902bf4517465 \
                --hash=sha256:8845c0631c0be43abdd865511c41eab235e0be69c81dc66a50911594198679b0 \
                --hash=sha256:22de65bb9010a725b0dac248f353bb072969c94fa8d6b1f34b87d7953cf7bbe4 \
                --hash=sha256:46065496ab748469cdd999246d17e301b2c24ae2fdf739132e580a0e94c94a87 \
                --hash=sha256:8a66d6fb6ae7661c58995f9c6435bda2b1e68b54b598a6a10247bfcdadac996c \
                --hash=sha256:26bfcd00dcf2cf160f122186af731ae30ab120c18e8375684ec2670dccd28130 \
                --hash=sha256:c8a231e36ec2cab018c4ad4358c386e36eede0319a0c41fed24f840b1dac59f6 \
                --hash=sha256:68be3a09455743ff9505491220b64440ced8973fe930f270c8e07ccfa25b1f9e \
                --hash=sha256:8b097553b380236d51ed11356c953bf8ce36a29a3e596e934ecabe76c985a577 \
                --hash=sha256:62985f233210dee6548c223301b6c25440852e13d59a8b81490203c3227c5ba0 \
                --hash=sha256:834a43af110f743a754448463e8fd61259cd4ab5bbedcf70f9dabad1d28a394c
  # [/PyNaCl]
  # [python-dateutil]
  python-dateutil==2.9.0.post0 --hash=sha256:a8b2bc7bffae282281c8140a97d3aa9c14da0b136dfe83f850eea9a5f7470427
  # [/python-dateutil]
  # [six]
  six==1.17.0 --hash=sha256:4721f391ed90541fddacab5acf947aa0d3dc7d27b2e1e8eda2be8970586c3274
  # [/six]
  # [typing_extensions]
  typing_extensions==4.15.0 --hash=sha256:f0fa19c6845758ab08074a0cfa8b7aecb71c999ca73d62883bc25cc018c4e548
  # [/typing_extensions]
  # [PyGithub]
  PyGithub==2.9.1 --hash=sha256:2ec78fca30092d51a42d76f4ddb02131b6f0c666a35dfdf364cf302cdda115b9
  # [/PyGithub]
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
