find_package( ITK )
if( ITK_FOUND )
  include(${ITK_USE_FILE})
else( ITK_FOUND )
  message( FATAL_ERROR "Cannot build without ITK" )
endif( ITK_FOUND )

find_package(GenerateCLP NO_MODULE REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

set( CLPEXAMPLE1_SOURCE CLPExample1.cxx )
GENERATECLP(CLPEXAMPLE1_SOURCE CLPExample1.xml)
add_executable( CLPExample1 ${CLPEXAMPLE1_SOURCE})
target_link_libraries( CLPExample1 ITKEXPAT)

set( POINTEXAMPLE1_SOURCE PointExample1.cxx )
GENERATECLP(POINTEXAMPLE1_SOURCE PointExample1.xml)
add_executable( PointExample1 ${POINTEXAMPLE1_SOURCE})
target_link_libraries( PointExample1 ITKEXPAT)

set( SLICEREXAMPLE_SOURCE Slicer3Example.cxx )
GENERATECLP(SLICEREXAMPLE_SOURCE Slicer3Example.xml)
add_executable( SlicerExample ${SLICEREXAMPLE_SOURCE})

add_test(NAME SlicerCLPExample1 COMMAND $<TARGET_FILE:SlicerExample> --help)
add_test(NAME SlicerCLPExample2 COMMAND $<TARGET_FILE:SlicerExample> --echo --load_freesurfer_volume FSV --stereo)
add_test(NAME SlicerCLPExample3 COMMAND $<TARGET_FILE:SlicerExample>)


