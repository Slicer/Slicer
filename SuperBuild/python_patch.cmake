
set(in_msvc9compiler ${CMAKE_CURRENT_LIST_DIR}/python_patched_msvc9compiler.py)
set(out_msvc9compiler ${PYTHON_SRC_DIR}/Lib/distutils/msvc9compiler.py)

#
# This custom version of 'msvc9compiler.py' has been made by applying
# the following patches to the version of 'msvc9compiler.py' shipped with Python-2.7.3.tgz:
#
#   06-msvc9compiler-vs10-support-issue16296.patch
#   07-msvc9compiler-vcexpress-and-sdk-support-issue7511.patch
#
# The patches can be downloaded here:
#
#  https://github.com/davidsansome/python-cmake-buildsystem/tree/master/cmake/patches-win32
#

message("Copying patched 'Lib/distutils/msvc9compiler.py' into source directory [${PYTHON_SRC_DIR}]
  in_msvc9compiler:${in_msvc9compiler}
  out_msvc9compiler:${out_msvc9compiler}"
  )

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${in_msvc9compiler} ${out_msvc9compiler})
