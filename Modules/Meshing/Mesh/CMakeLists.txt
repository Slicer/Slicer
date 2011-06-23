#############################################################################
# TODO If you port that module to SlicerQT and plane to use INSTALL_TARGETS #
# Use install() instead - INSTALL_TARGETS is now deprecated in CMake 2.8    #
#############################################################################

#install_targets(/lib/IaFeMesh iamMesh)

#############################################################################
# TODO If you port that module to SlicerQT and plane to use INSTALL_FILES   #
# Use install() instead - INSTALL_FILES is now deprecated in CMake 2.8      #
#############################################################################

install_files(/include/IaFeMesh/Mesh "(\\.h|\\.txx)$")
