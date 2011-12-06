
#-----------------------------------------------------------------------------
# RigidRegistration tests
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(CLP RigidRegistration)

add_executable(${CLP}Test ${CLP}Test.cxx)
target_link_libraries(${CLP}Test ${CLP}Lib)
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})

set(testname ${CLP}Test000)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint --help
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test00)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest00.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest00Transform.txt
  --initialtransform ${TEST_DATA}/itkAffineTransform00.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr
  ${TEST_DATA}/CTHeadAxial.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test01)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest01.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest01Transform.txt
  --spatialsamples 10000
  --histogrambins 100
  --iterations 50
  --learningrate 0.001
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr
  ${TEST_DATA}/CTHeadAxial.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test02)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest02.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest02Transform.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr
  ${TEST_DATA}/CTHeadAxial.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test03)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest03.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest03Transform.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr
  ${TEST_DATA}/CTHeadAxial.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}Test04)
add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest04.nhdr
  --outputtransform ${TEMP}/RigidRegistrationTest04Transform.txt
  --spatialsamples 10000
  --histogrambins 32
  --iterations 50
  --learningrate 0.01
  --translationscale 100
  ${TEST_DATA}/CTHeadAxial.nhdr
  ${TEST_DATA}/CTHeadAxial.nhdr
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

# This test reproduces exactly the registration step on
# the meningioma growth detection (tutorial) data for ChangeTracker
# Setting parameters explicitly, because default values are not passed to CLI
# In testing mode, this test is supposed to fail if the error at the image
# corner (as compared to the transform passed with --initialtransform) exceeds
# 0.1
#set(testname ${CLP}Test02)
#add_test(NAME ${testname} COMMAND ${Launcher_Command} $<TARGET_FILE:${CLP}Test>
#  --compare ${BASELINE}/RigidRegistrationTest02.nrrd
#   ${TEMP}/RigidRegistrationTest02.nrrd
#  RigidRegistrationTest
#  --testingmode
#  --initialtransform ${TEST_DATA}/MRMeningioma01.tfm
#  --resampledmovingfilename  ${TEMP}/RigidRegistrationTest02.nrrd
#  --outputtransform ${TEMP}/RigidRegistrationTest02Transform.txt
#  --iterations 100,100,50,20
#  --fixedsmoothingfactor 0
#  --movingsmoothingfactor 0
#  --histogrambins 30
#  --spatialsamples 10000
#  --learningrate 0.01,0.005,0.0005,0.0002
#  --translationscale 100
#  ${TEST_DATA}/MRMeningioma0.nrrd
#  ${TEST_DATA}/MRMeningioma1.nrrd
#  )
#set_property(TEST ${testname} PROPERTY LABELS ${CLP})

