project(PrecompileVtkHeaders)

# No way found to make it work with add_library, this error appears in the project which tries to reuse the precompiled headers:
# cmake_pch.hxx.gch: not used because `PrecompileVtkHeaders_EXPORTS' not defined [-Winvalid-pch]
add_executable(${PROJECT_NAME} PrecompileVtkHeaders.cxx)
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

set(VTK_BUILD_DIR ${VTK_PREFIX_PATH})

# Precompile only low level most common headers because header precompilation takes time
set(precompiled_headers
  "${VTK_SOURCE_DIR}/Charts/Core/vtkPlot.h"
  "${VTK_SOURCE_DIR}/Charts/Core/vtkPlotLine.h"

  "${VTK_SOURCE_DIR}/Common/Core/vtkBitArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkCallbackCommand.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkCollection.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkCommand.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkDataArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkDebugLeaks.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkDoubleArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkFloatArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkShortArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkSignedCharArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkUnsignedCharArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkUnsignedLongArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkInformation.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkInformationVector.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkIntArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkLookupTable.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkMath.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkMultiThreader.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkNew.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkObject.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkObjectFactory.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkPoints.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkSmartPointer.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkStdString.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkStringArray.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkVersion.h"
  "${VTK_SOURCE_DIR}/Common/Core/vtkWeakPointer.h"

  "${VTK_SOURCE_DIR}/Common/DataModel/vtkBoundingBox.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkCell.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkColor.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkDataObject.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkFieldData.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkImageData.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkPlane.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkPlanes.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkPointData.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkPolyData.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkTable.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkUnstructuredGrid.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkVector.h"
  "${VTK_SOURCE_DIR}/Common/DataModel/vtkVoxel.h"

  "${VTK_SOURCE_DIR}/Common/ExecutionModel/vtkAlgorithm.h"
  "${VTK_SOURCE_DIR}/Common/ExecutionModel/vtkAlgorithmOutput.h"
  "${VTK_SOURCE_DIR}/Common/ExecutionModel/vtkImageAlgorithm.h"
  "${VTK_SOURCE_DIR}/Common/ExecutionModel/vtkStreamingDemandDrivenPipeline.h"
  "${VTK_SOURCE_DIR}/Common/ExecutionModel/vtkTrivialProducer.h"

  "${VTK_SOURCE_DIR}/Common/Math/vtkMatrix3x3.h"
  "${VTK_SOURCE_DIR}/Common/Math/vtkMatrix4x4.h"

  "${VTK_SOURCE_DIR}/Common/Misc/vtkErrorCode.h"

  "${VTK_SOURCE_DIR}/Common/System/vtkTimerLog.h"

  "${VTK_SOURCE_DIR}/Common/Transforms/vtkGeneralTransform.h"
  "${VTK_SOURCE_DIR}/Common/Transforms/vtkHomogeneousTransform.h"
  "${VTK_SOURCE_DIR}/Common/Transforms/vtkTransform.h"

  "${VTK_SOURCE_DIR}/Filters/Core/vtkAppendPolyData.h"
  "${VTK_SOURCE_DIR}/Filters/Core/vtkAssignAttribute.h"
  "${VTK_SOURCE_DIR}/Filters/Core/vtkTriangleFilter.h"
  "${VTK_SOURCE_DIR}/Filters/Core/vtkTubeFilter.h"

  "${VTK_SOURCE_DIR}/Filters/Geometry/vtkGeometryFilter.h"

  "${VTK_SOURCE_DIR}/Filters/Sources/vtkSphereSource.h"

  "${VTK_SOURCE_DIR}/Imaging/Core/vtkImageAppendComponents.h"
  "${VTK_SOURCE_DIR}/Imaging/Core/vtkImageCast.h"
  "${VTK_SOURCE_DIR}/Imaging/Core/vtkImageChangeInformation.h"
  "${VTK_SOURCE_DIR}/Imaging/Core/vtkImageConstantPad.h"
  "${VTK_SOURCE_DIR}/Imaging/Core/vtkImageExtractComponents.h"
  "${VTK_SOURCE_DIR}/Imaging/Core/vtkImageReslice.h"
  "${VTK_SOURCE_DIR}/Imaging/Core/vtkImageThreshold.h"

  "${VTK_SOURCE_DIR}/Imaging/Math/vtkImageMathematics.h"

  "${VTK_SOURCE_DIR}/Imaging/Statistics/vtkImageAccumulate.h"

  "${VTK_SOURCE_DIR}/Imaging/Stencil/vtkImageStencil.h"

  "${VTK_SOURCE_DIR}/IO/Image/vtkPNGWriter.h"

  "${VTK_SOURCE_DIR}/Rendering/Core/vtkActor.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkActor2D.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkCamera.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkColorTransferFunction.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkImageMapper.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkImageMapper3D.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkProperty.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkProperty2D.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkRenderer.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkRenderWindow.h"
  "${VTK_SOURCE_DIR}/Rendering/Core/vtkTextProperty.h"

  "${VTK_SOURCE_DIR}/Utilities/Python/vtkPython.h"

  "${VTK_SOURCE_DIR}/Wrapping/PythonCore/vtkPythonUtil.h"

  "${VTK_BUILD_DIR}/Utilities/KWSys/vtksys/Directory.hxx"
  "${VTK_BUILD_DIR}/Utilities/KWSys/vtksys/RegularExpression.hxx"
  "${VTK_BUILD_DIR}/Utilities/KWSys/vtksys/SystemTools.hxx"
  )

target_precompile_headers(${PROJECT_NAME} PUBLIC ${precompiled_headers})
