project(SlicerLibs)

set(dirs
  tclap
  ModuleDescriptionParser
  GenerateCLP
  LoadableModule
  )

# Note: Order of of SUBDIRS is important if your projects depend
# on code in other projects (put dependent project later).

if(TEEM_FOUND)
  set(dirs ${dirs} vtkTeem)
endif(TEEM_FOUND)

set(dirs ${dirs}
  vtkITK
  FreeSurfer
  Qdec
  MRML
  GenerateLM
  SlicerIO
  ITKCommandIO
  MRMLIDImageIO
  MGHImageIO
  IGT
  RemoteIO
  #SIGN
  )

set(include_dirs)
set(libs)

set(ModuleDescriptionParser_USE_PYTHON ${Slicer3_USE_PYTHON})
set(LoadableModule_USE_PYTHON ${Slicer3_USE_PYTHON})

set(IGT_USE_IGSTK ${Slicer3_USE_IGSTK})
set(IGT_USE_NAVITRACK ${Slicer3_USE_NAVITRACK})
set(ITKCommandIO_INSTALL_ITKFACTORIES_DIR ${Slicer3_INSTALL_ITKFACTORIES_DIR})
set(MGHImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer3_INSTALL_ITKFACTORIES_DIR})
set(MRMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${Slicer3_INSTALL_ITKFACTORIES_DIR})

foreach(dir ${dirs})

  set(include_dirs ${include_dirs}
    ${CMAKE_CURRENT_SOURCE_DIR}/${dir}
    ${CMAKE_CURRENT_BINARY_DIR}/${dir}
    )

  set(libs ${libs} ${dir})
endforeach(dir)

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS ${tmp} ${include_dirs})
# Inconsistent library name vs. project/dir name :(

string(REPLACE ITKCommandIO CommandIO libs "${libs}")
string(REPLACE SlicerIO slicerio libs "${libs}")
string(REPLACE MGHImageIO MGHIO libs "${libs}")
string(REPLACE MRMLIDImageIO MRMLIDIO libs "${libs}")

# Those are not libs

list(REMOVE_ITEM libs tclap GenerateCLP GenerateLM)

slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES ${tmp} ${libs})

subdirs(${dirs})
