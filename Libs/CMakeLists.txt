PROJECT( SlicerLibs )

# Note: Order of of SUBDIRS is important if your projects depend
# on code in other projects (put dependent project later).
SUBDIRS(
    vtkITK
    FreeSurfer
    MRML
    ModuleDescriptionParser
    GenerateCLP
    SlicerIO
    ITKCommandIO
    MRMLIDImageIO
)

