
file(READ ${in} code)
string(REPLACE "${find}" "${replace}" code "${code}")
file(WRITE ${out} "${code}")
