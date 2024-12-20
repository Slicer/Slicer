project(PrecompileItkHeaders)

# No way found to make it work with add_library, this error appears in the project which tries to reuse the precompiled headers:
# cmake_pch.hxx.gch: not used because `PrecompileItkHeaders_EXPORTS' not defined [-Winvalid-pch]
add_executable(${PROJECT_NAME} PrecompileItkHeaders.cxx)
target_link_libraries(${PROJECT_NAME} ${ITK_LIBRARIES})

set(ITK_SOURCE_DIR "${ITK_CMAKE_DIR}/..")

# Do not precompile VTK/Common/Archive headers because they do not compile because of missing vtkCommonArchiveModule.h
# Do not precompile VTK/Imaging/OpenGL2 headers because they do not compile because of missing vtkImagingOpenGL2Module.h
file(GLOB_RECURSE precompiled_headers
  ${ITK_SOURCE_DIR}/Modules/Core/Common/include/*.h
  ${ITK_SOURCE_DIR}/Modules/Core/ImageFunction/include/*.h
  ${ITK_SOURCE_DIR}/Modules/Core/Transform/include/*.h
  ${ITK_SOURCE_DIR}/Modules/Filtering/ImageGrid/include/*.h
  ${ITK_SOURCE_DIR}/Modules/Filtering/ImageFilterBase/include/*.h
  ${ITK_SOURCE_DIR}/Modules/Filtering/DiffusionTensorImage/include/*.h
  ${ITK_SOURCE_DIR}/Modules/IO/NRRD/include/*.h
  ${ITK_SOURCE_DIR}/Modules/IO/ImageBase/include/*.h
  ${ITK_SOURCE_DIR}/Modules/IO/SpatialObjects/include/*.h
  ${ITK_SOURCE_DIR}/Modules/IO/TransformBase/include/*.h
  ${ITK_SOURCE_DIR}/Modules/IO/TransformFactory/include/*.h
)

list(REMOVE_ITEM precompiled_headers ${ITK_SOURCE_DIR}/Modules/Core/Common/include/itkExceptionObject.h)

cmake_path(GET ITK_CONFIG_TARGETS_FILE PARENT_PATH ITK_BUILD_DIR)
list(APPEND precompiled_headers
${ITK_BUILD_DIR}/Modules/ThirdParty/KWSys/src/itksys/Directory.hxx
  ${ITK_BUILD_DIR}/Modules/ThirdParty/KWSys/src/itksys/SystemTools.hxx
  ${ITK_BUILD_DIR}/Modules/Core/Common/itkConfigure.h
)

target_precompile_headers(${PROJECT_NAME} PUBLIC ${precompiled_headers})
