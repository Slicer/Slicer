set(sysinfo ${NUMPY_SRC_DIR}/numpy/distutils/system_info.py)

message(STATUS "NUMPY: Patching ${sysinfo}")

file(READ ${sysinfo} sysinfo_src)
# string(REPLACE "if get_platform()[-4:] == 'i386':"
# "if 'intel' in get_platform():" sysinfo_src "${sysinfo_src}")
string(REPLACE "                if intel:
                    args.extend(['-msse3'])
                else:
                    args.extend(['-faltivec'])
" ""
sysinfo_src "${sysinfo_src}")

file(WRITE ${sysinfo} "${sysinfo_src}")


#
# To allow building without a Fortran compiler, effectively back out this change:
# https://github.com/numpy/numpy/commit/4a3fd1f40ef59b872341088a2e97712c671ea4ca
#

set(in_fcompiler_init ${CMAKE_CURRENT_LIST_DIR}/NUMPY_patched_fcompiler_init.py)
set(out_fcompiler_init ${NUMPY_SRC_DIR}/numpy/distutils/fcompiler/__init__.py)

message("Copying patched 'numpy/distutils/fcompiler/__init__.py' into source directory [${NUMPY_SRC_DIR}]
  in_fcompiler_init:${in_fcompiler_init}
  out_fcompiler_init:${out_fcompiler_init}"
  )

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${in_fcompiler_init} ${out_fcompiler_init})
