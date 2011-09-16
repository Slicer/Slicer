#
# this script will be used inside the EM_InstallTasks.cmake file to copy files using the -P argument
#

file(COPY ${SRC} DESTINATION ${DST} PATTERN .svn EXCLUDE)
