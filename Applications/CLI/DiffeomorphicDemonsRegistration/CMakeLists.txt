# INCLUDE(${CMAKE_SOURCE_DIR}/IJMacros.txt)

#Change PROJECT_NAME to the name of your project
PROJECT(DemonsRegistration)

SET (CLP  DemonsRegistration)
SET ( ${CLP}_SOURCE ${CLP}.cxx)
GENERATECLP(${CLP}_SOURCE ${CLP}.xml)

#The following lines are required to use Dart
ENABLE_TESTING()
INCLUDE(Dart)

#Declare any external dependencies that your project may have here.
#examples include: ITK, VTK, JPEG, PNG, OpenGL, ZLIB, Perl, Java
#If you're not sure what name to use, look in the Modules directory of your
#cmake install and check that a file named Find(Package).cmake exists
FIND_PACKAGE(ITK REQUIRED)
# SET(Required_Packages
# ITK
# )

#this foreach loads all of the packages that you specified as required.
#It shouldn't need to be modified.
#FOREACH(Package ${Required_Packages})
#  LOADPACKAGE(${Package})
#ENDFOREACH(Package)


#Set any libraries that your project depends on.
#examples: ITKCommon, VTKRendering, etc
SET(Libraries
    ITKCommon
    ITKIO
    ITKStatistics
)

#the following block of code is an example of how to build an executable in
#cmake.  Unmodified, it will add an executable called "MyExe" to the project.
#MyExe will be built using the files MyClass.h and MyClass.cxx, and it will
#be linked to all the libraries you specified above. 
#You can build more than one executable per project
SET(CurrentExe "ImageCompare")
ADD_EXECUTABLE(${CurrentExe} ImageCompare.cxx) 
TARGET_LINK_LIBRARIES(${CurrentExe} ${Libraries})

SET(CurrentExe "DemonsRegistration")
ADD_EXECUTABLE(${CurrentExe} DemonsRegistration.cxx) 
TARGET_LINK_LIBRARIES(${CurrentExe} ${Libraries})

#the following line is an example of how to add a test to your project.
#Testname is the title for this particular test.  ExecutableToRun is the
#program which will be running this test.  It can either be a part of this
#project or an external executable.  After that list any args that are needed
#for this test.  Include as many tests as you like.  If your project doesn't have
#any tests you can comment out or delete the following line.
IF (WIN32) 
 ADD_TEST(DemonsRegistrationTest1 ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}/${CLP} --help)
ELSE(WIN32) 
  ADD_TEST(DemonsRegistrationTest1 ${EXECUTABLE_OUTPUT_PATH}/${CLP} --help) 
ENDIF (WIN32)

# IF (WIN32) 
#  ADD_TEST(DemonsRegistrationTest2 ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}/${CLP}  -f ${CMAKE_SOURCE_DIR}/RatLungSlice1.mha -m ${CMAKE_SOURCE_DIR}/RatLungSlice2.mha -v -s2 -g1 -i 20x20x20 )
# ELSE(WIN32) 
#   ADD_TEST(DemonsRegistrationTest2 ${EXECUTABLE_OUTPUT_PATH}/${CLP} -f ${CMAKE_SOURCE_DIR}/RatLungSlice1.mha -m ${CMAKE_SOURCE_DIR}/RatLungSlice2.mha -v -s2 -g1 -i 20x20x20) 
# ENDIF (WIN32)

# ADD_TEST(CompareImage ImageCompare ${CMAKE_SOURCE_DIR}/RatLungSlice2-Reg.mha output.mha)

#Once you're done modifying this template, you should rename it to "CMakeLists.txt"
