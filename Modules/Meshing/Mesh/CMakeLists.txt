#############################################################################
# TODO If you port that module to SlicerQT and plane to use INSTALL_TARGETS #
# Use INSTALL() instead - INSTALL_TARGETS is now deprecated in CMake 2.8    #
#############################################################################

#INSTALL_TARGETS(/lib/IaFeMesh iamMesh)

#############################################################################
# TODO If you port that module to SlicerQT and plane to use INSTALL_FILES   #
# Use INSTALL() instead - INSTALL_FILES is now deprecated in CMake 2.8      #
#############################################################################

INSTALL_FILES(/include/IaFeMesh/Mesh "(\\.h|\\.txx)$")
