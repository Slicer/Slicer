
set(in_msvc9compiler ${CMAKE_CURRENT_LIST_DIR}/python_patched_msvc9compiler.py)
set(out_msvc9compiler ${PYTHON_SRC_DIR}/Lib/distutils/msvc9compiler.py)

#
# This custom version of 'msvc9compiler.py' has been made by applying
# the following patches to the version of 'msvc9compiler.py' shipped with Python-2.7.10.tgz:
#
#   06-msvc9compiler-vs10-support-issue16296.patch
#   07-msvc9compiler-vcexpress-and-sdk-support-issue7511.patch
#
# The patches can be downloaded here:
#
#  https://github.com/davidsansome/python-cmake-buildsystem/tree/master/cmake/patches-win32
#
#
# Additionally, an entry for Visual Studio 2013 was added to WINSDK_VERSION_MAP
# to fix the following error when building Numpy 1.9.x:
#
#  Traceback (most recent call last):
#    ...
#    File "C:\dev\S4D\NUMPY\numpy\distutils\command\config.py", line 55, in _check_compiler
#      self.compiler.initialize()
#    File "C:\dev\S4D\python-install\Lib\distutils\msvc9compiler.py", line 500, in initialize
#      vc_env = query_vcvarsall(VERSION, plat_spec)
#    File "C:\dev\S4D\python-install\Lib\distutils\msvc9compiler.py", line 371, in query_vcvarsall
#      for sdkver in WINSDK_VERSION_MAP[versionstr]:
#  KeyError: '12.0'
#

message("Copying patched 'Lib/distutils/msvc9compiler.py' into source directory [${PYTHON_SRC_DIR}]
  in_msvc9compiler:${in_msvc9compiler}
  out_msvc9compiler:${out_msvc9compiler}"
  )

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${in_msvc9compiler} ${out_msvc9compiler})
