if( ("${MSVC_VERSION}" VERSION_GREATER "1599"))
  message("Fixing source in ${PYTHON_SRC_DIR} for MSVC 2010/2012 compiler")
  set(msvc9c ${PYTHON_SRC_DIR}/Lib/distutils/msvc9compiler.py)

  file(READ ${msvc9c} msvc9c_src)
  string(REPLACE "mfinfo = self.manifest_get_embed_info(target_desc, ld_args)"
    "mfinfo = None"
    msvc9c_src "${msvc9c_src}")

  file(WRITE ${msvc9c} "${msvc9c_src}")
endif()
