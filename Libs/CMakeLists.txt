PROJECT( SlicerLibs )

## GenerateGLP needs to be compiled first.
SUBDIRS(
      tclap
      ModuleDescriptionParser
      GenerateCLP
)
# Note: Order of of SUBDIRS is important if your projects depend
# on code in other projects (put dependent project later).
IF( TEEM_FOUND )
  SUBDIRS(
      vtkTeem
  )
ENDIF( TEEM_FOUND )

SUBDIRS(
      vtkITK
      FreeSurfer
      Qdec
      MRML
      SlicerIO
      ITKCommandIO
      MRMLIDImageIO
      MGHImageIO
      IGT
      #SIGN
)


