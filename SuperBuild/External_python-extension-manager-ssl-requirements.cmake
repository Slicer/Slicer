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
  #  - cryptography-46.0.3-cp311-abi3-macosx_10_9_universal2.whl
  #  - cryptography-46.0.3-cp311-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - cryptography-46.0.3-cp311-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - cryptography-46.0.3-cp311-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-46.0.3-cp311-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-46.0.3-cp311-abi3-manylinux_2_34_aarch64.whl
  #  - cryptography-46.0.3-cp311-abi3-manylinux_2_34_x86_64.whl
  #  - cryptography-46.0.3-cp311-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-46.0.3-cp311-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-46.0.3-cp311-abi3-win_amd64.whl
  #  - cryptography-46.0.3-cp311-abi3-win_arm64.whl
  #  - cryptography-46.0.3-cp38-abi3-macosx_10_9_universal2.whl
  #  - cryptography-46.0.3-cp38-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - cryptography-46.0.3-cp38-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - cryptography-46.0.3-cp38-abi3-manylinux_2_28_aarch64.whl
  #  - cryptography-46.0.3-cp38-abi3-manylinux_2_28_x86_64.whl
  #  - cryptography-46.0.3-cp38-abi3-manylinux_2_34_aarch64.whl
  #  - cryptography-46.0.3-cp38-abi3-manylinux_2_34_x86_64.whl
  #  - cryptography-46.0.3-cp38-abi3-musllinux_1_2_aarch64.whl
  #  - cryptography-46.0.3-cp38-abi3-musllinux_1_2_x86_64.whl
  #  - cryptography-46.0.3-cp38-abi3-win_amd64.whl
  #  - cryptography-46.0.3-cp38-abi3-win_arm64.whl
  cryptography==46.0.3 --hash=sha256:109d4ddfadf17e8e7779c39f9b18111a09efb969a301a31e987416a0191ed93a \
                       --hash=sha256:09859af8466b69bc3c27bdf4f5d84a665e0f7ab5088412e9e2ec49758eca5cbc \
                       --hash=sha256:01ca9ff2885f3acc98c29f1860552e37f6d7c7d013d7334ff2a9de43a449315d \
                       --hash=sha256:6eae65d4c3d33da080cff9c4ab1f711b15c1d9760809dad6ea763f3812d254cb \
                       --hash=sha256:a2c0cd47381a3229c403062f764160d57d4d175e022c1df84e168c6251a22eec \
                       --hash=sha256:549e234ff32571b1f4076ac269fcce7a808d3bf98b76c8dd560e42dbc66d7d91 \
                       --hash=sha256:10b01676fc208c3e6feeb25a8b83d81767e8059e1fe86e1dc62d10a3018fa926 \
                       --hash=sha256:0abf1ffd6e57c67e92af68330d05760b7b7efb243aab8377e583284dbab72c71 \
                       --hash=sha256:a04bee9ab6a4da801eb9b51f1b708a1b5b5c9eb48c03f74198464c66f0d344ac \
                       --hash=sha256:a9a3008438615669153eb86b26b61e09993921ebdd75385ddd748702c5adfddb \
                       --hash=sha256:5d7f93296ee28f68447397bf5198428c9aeeab45705a55d53a6343455dcb2c3c \
                       --hash=sha256:cb3d760a6117f621261d662bccc8ef5bc32ca673e037c83fbe565324f5c46936 \
                       --hash=sha256:4b7387121ac7d15e550f5cb4a43aef2559ed759c35df7336c402bb8275ac9683 \
                       --hash=sha256:15ab9b093e8f09daab0f2159bb7e47532596075139dd74365da52ecc9cb46c5d \
                       --hash=sha256:46acf53b40ea38f9c6c229599a4a13f0d46a6c3fa9ef19fc1a124d62e338dfa0 \
                       --hash=sha256:1000713389b75c449a6e979ffc7dcc8ac90b437048766cef052d4d30b8220971 \
                       --hash=sha256:b02cf04496f6576afffef5ddd04a0cb7d49cf6be16a9059d793a30b035f6b6ac \
                       --hash=sha256:402b58fc32614f00980b66d6e56a5b4118e6cb362ae8f3fda141ba4689bd4506 \
                       --hash=sha256:ef639cb3372f69ec44915fafcd6698b6cc78fbe0c2ea41be867f6ed612811963 \
                       --hash=sha256:3b51b8ca4f1c6453d8829e1eb7299499ca7f313900dd4d89a24b8b87c0a780d4 \
                       --hash=sha256:416260257577718c05135c55958b674000baef9a1c7d9e8f306ec60d71db850f \
                       --hash=sha256:d89c3468de4cdc4f08a57e214384d0471911a3830fcdaf7a8cc587e42a866372
  # [/cryptography]
  # Specifying "[crypto]" is required since PyGithub 1.58.1
  # See https://github.com/Slicer/Slicer/issues/7112
  # [PyJWT]
  PyJWT==2.10.1 --hash=sha256:dcdd193e30abefd5debf142f9adfcdd2b58004e644f25406ffaebd50bd98dacb
  # [/PyJWT]
  # [wrapt]
  # Hashes correspond to the following packages:
  #  - wrapt-2.0.1-cp312-cp312-macosx_10_13_universal2.whl
  #  - wrapt-2.0.1-cp312-cp312-macosx_10_13_x86_64.whl
  #  - wrapt-2.0.1-cp312-cp312-macosx_11_0_arm64.whl
  #  - wrapt-2.0.1-cp312-cp312-manylinux1_x86_64.manylinux_2_28_x86_64.manylinux_2_5_x86_64.whl
  #  - wrapt-2.0.1-cp312-cp312-manylinux2014_aarch64.manylinux_2_17_aarch64.manylinux_2_28_aarch64.whl
  #  - wrapt-2.0.1-cp312-cp312-manylinux_2_31_riscv64.manylinux_2_39_riscv64.whl
  #  - wrapt-2.0.1-cp312-cp312-musllinux_1_2_aarch64.whl
  #  - wrapt-2.0.1-cp312-cp312-musllinux_1_2_riscv64.whl
  #  - wrapt-2.0.1-cp312-cp312-musllinux_1_2_x86_64.whl
  #  - wrapt-2.0.1-cp312-cp312-win_amd64.whl
  #  - wrapt-2.0.1-cp312-cp312-win_arm64.whl
  #  - wrapt-2.0.1-py3-none-any.whl
  wrapt==2.0.1 --hash=sha256:1fdbb34da15450f2b1d735a0e969c24bdb8d8924892380126e2a293d9902078c \
               --hash=sha256:3d32794fe940b7000f0519904e247f902f0149edbe6316c710a8562fb6738841 \
               --hash=sha256:386fb54d9cd903ee0012c09291336469eb7b244f7183d40dc3e86a16a4bace62 \
               --hash=sha256:7b219cb2182f230676308cdcacd428fa837987b89e4b7c5c9025088b8a6c9faf \
               --hash=sha256:641e94e789b5f6b4822bb8d8ebbdfc10f4e4eae7756d648b717d980f657a9eb9 \
               --hash=sha256:fe21b118b9f58859b5ebaa4b130dee18669df4bd111daad082b7beb8799ad16b \
               --hash=sha256:17fb85fa4abc26a5184d93b3efd2dcc14deb4b09edcdb3535a536ad34f0b4dba \
               --hash=sha256:b89ef9223d665ab255ae42cc282d27d69704d94be0deffc8b9d919179a609684 \
               --hash=sha256:a453257f19c31b31ba593c30d997d6e5be39e3b5ad9148c2af5a7314061c63eb \
               --hash=sha256:2da620b31a90cdefa9cd0c2b661882329e2e19d1d7b9b920189956b76c564d75 \
               --hash=sha256:aea9c7224c302bc8bfc892b908537f56c430802560e827b75ecbde81b604598b \
               --hash=sha256:4d2ce1bf1a48c5277d7969259232b57645aae5686dba1eaeade39442277afbca
  # [/wrapt]
  # [Deprecated]
  Deprecated==1.3.1 --hash=sha256:597bfef186b6f60181535a29fbe44865ce137a5079f295b479886c82729d5f3f
  # [/Deprecated]
  # [pycparser]
  pycparser==2.23 --hash=sha256:e5c6e8d3fbad53479cab09ac03729e0a9faf2bee3db8208a550daf5af81a5934
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
  #  - pynacl-1.6.1-cp38-abi3-macosx_10_10_universal2.whl
  #  - pynacl-1.6.1-cp38-abi3-manylinux2014_aarch64.manylinux_2_17_aarch64.whl
  #  - pynacl-1.6.1-cp38-abi3-manylinux2014_x86_64.manylinux_2_17_x86_64.whl
  #  - pynacl-1.6.1-cp38-abi3-manylinux_2_26_aarch64.manylinux_2_28_aarch64.whl
  #  - pynacl-1.6.1-cp38-abi3-manylinux_2_26_x86_64.manylinux_2_28_x86_64.whl
  #  - pynacl-1.6.1-cp38-abi3-manylinux_2_34_aarch64.whl
  #  - pynacl-1.6.1-cp38-abi3-manylinux_2_34_x86_64.whl
  #  - pynacl-1.6.1-cp38-abi3-musllinux_1_1_aarch64.whl
  #  - pynacl-1.6.1-cp38-abi3-musllinux_1_1_x86_64.whl
  #  - pynacl-1.6.1-cp38-abi3-musllinux_1_2_aarch64.whl
  #  - pynacl-1.6.1-cp38-abi3-musllinux_1_2_x86_64.whl
  #  - pynacl-1.6.1-cp38-abi3-win_amd64.whl
  #  - pynacl-1.6.1-cp38-abi3-win_arm64.whl
  PyNaCl==1.6.1 --hash=sha256:a6f9fd6d6639b1e81115c7f8ff16b8dedba1e8098d2756275d63d208b0e32021 \
                --hash=sha256:e49a3f3d0da9f79c1bec2aa013261ab9fa651c7da045d376bd306cf7c1792993 \
                --hash=sha256:7713f8977b5d25f54a811ec9efa2738ac592e846dd6e8a4d3f7578346a841078 \
                --hash=sha256:5a3becafc1ee2e5ea7f9abc642f56b82dcf5be69b961e782a96ea52b55d8a9fc \
                --hash=sha256:4ce50d19f1566c391fedc8dc2f2f5be265ae214112ebe55315e41d1f36a7f0a9 \
                --hash=sha256:543f869140f67d42b9b8d47f922552d7a967e6c116aad028c9bfc5f3f3b3a7b7 \
                --hash=sha256:a2bb472458c7ca959aeeff8401b8efef329b0fc44a89d3775cffe8fad3398ad8 \
                --hash=sha256:3206fa98737fdc66d59b8782cecc3d37d30aeec4593d1c8c145825a345bba0f0 \
                --hash=sha256:53543b4f3d8acb344f75fd4d49f75e6572fce139f4bfb4815a9282296ff9f4c0 \
                --hash=sha256:319de653ef84c4f04e045eb250e6101d23132372b0a61a7acf91bac0fda8e58c \
                --hash=sha256:262a8de6bba4aee8a66f5edf62c214b06647461c9b6b641f8cd0cb1e3b3196fe \
                --hash=sha256:a569a4069a7855f963940040f35e87d8bc084cb2d6347428d5ad20550a0a1a21 \
                --hash=sha256:5953e8b8cfadb10889a6e7bd0f53041a745d1b3d30111386a1bb37af171e6daf
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
  PyGithub==2.8.1 --hash=sha256:23a0a5bca93baef082e03411bf0ce27204c32be8bfa7abc92fe4a3e132936df0
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
