PROJECT( MeshCommandLine )

SUBDIRS(
  VoxelMeshingModule
)


IF (Slicer3_BUILD_TESTING)
    SUBDIRS( Testing )
ENDIF (Slicer3_BUILD_TESTING)
