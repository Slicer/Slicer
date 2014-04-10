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
