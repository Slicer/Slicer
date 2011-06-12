
file(READ ${in} code)
string(REPLACE "${find}" "${replace}" code_updated "${code}")
# Write output file only if required
if(NOT "${code}" STREQUAL "${code_updated}")
  file(WRITE ${out} "${code_updated}")
endif()

