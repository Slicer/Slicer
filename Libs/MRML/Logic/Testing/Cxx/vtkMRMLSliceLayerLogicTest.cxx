/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLLogic includes
#include "vtkMRMLSliceLayerLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkAssignAttribute.h>
#include <vtkDataSetAttributes.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageInterpolator.h>
#include <vtkImageReslice.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkTrivialProducer.h>

namespace
{
bool testDTIPipeline();
}

//----------------------------------------------------------------------------
int vtkMRMLSliceLayerLogicTest(int , char * [] )
{
  // Test setting volume node and then scene
  {
    vtkNew<vtkMRMLSliceLayerLogic> logic;

    EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

    TEST_GET_OBJECT(logic, Reslice);
    TEST_GET_OBJECT(logic, ResliceUVW);

    vtkNew<vtkMRMLScalarVolumeNode> VolumeNode;
    TEST_SET_GET_VALUE(logic, VolumeNode, VolumeNode.GetPointer());

    vtkNew<vtkMRMLScene> MRMLScene;
    TEST_SET_GET_VALUE(logic, MRMLScene, MRMLScene.GetPointer());
  }

  // Test setting scene and then the volume
  {
    vtkNew<vtkMRMLSliceLayerLogic> logic;

    vtkNew<vtkMRMLScene> MRMLScene;
    TEST_SET_GET_VALUE(logic, MRMLScene, MRMLScene.GetPointer());

    vtkNew<vtkMRMLScalarVolumeNode> VolumeNode;
    TEST_SET_GET_VALUE(logic, VolumeNode, VolumeNode.GetPointer());
  }

  bool res = true;
  res = res && testDTIPipeline();
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

namespace{

//----------------------------------------------------------------------------
bool testDTIPipeline()
{
  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(2,2,2);
  vtkNew<vtkFloatArray> tensors;
  // \fixme Name should not be needed VTK#14693
  tensors->SetName("tensors");
  tensors->SetNumberOfComponents(9);
  tensors->SetNumberOfTuples(2*2*2);
  for (int i = 0; i < 2*2*2;++i)
    {
    tensors->SetTuple9(i,1.,0.,0.,0.,1.,0.,0.,0.,1.);
    }
  imageData->GetPointData()->SetTensors(tensors.GetPointer());

  vtkNew<vtkTrivialProducer> tp;
  tp->SetOutput(imageData.GetPointer());
  // \fixme Set scalar type to Float should not be needed VTK#14692
  tp->UpdateInformation();
  vtkDataObject::SetPointDataActiveScalarInfo(
    tp->GetOutputInformation(0), VTK_FLOAT, 9);

  vtkNew<vtkAssignAttribute> tensorsToScalars;
  tensorsToScalars->Assign(vtkDataSetAttributes::TENSORS,
                           vtkDataSetAttributes::SCALARS,
                           vtkAssignAttribute::POINT_DATA);
  tensorsToScalars->SetInputConnection(tp->GetOutputPort());
  bool verbose = false;
  if (verbose)
    {
    tensorsToScalars->Update();
    vtkImageData* output = vtkImageData::SafeDownCast(tensorsToScalars->GetOutputDataObject(0));
    std::cout << "Output: " << output << std::endl;
    std::cout << "Point data: " << output->GetPointData() << std::endl;
    std::cout << "Scalars: " << output->GetPointData()->GetScalars() << std::endl;
    std::cout << "Tensors: " << output->GetPointData()->GetTensors() << std::endl;
    std::cout << "Scalar type: " << output->GetScalarType() << std::endl;
    std::cout << "Input scalar type: " << imageData->GetScalarType() << std::endl;
    }

  vtkNew<vtkImageReslice> reslicer;
  reslicer->GenerateStencilOutputOn();
  reslicer->SetNumberOfThreads(1);
  reslicer->SetInputConnection(tensorsToScalars->GetOutputPort());
  reslicer->SetOutputExtentToDefault();
  reslicer->SetOutputDimensionality(2);

  vtkNew<vtkAssignAttribute> scalarsToTensors;
  scalarsToTensors->Assign(vtkDataSetAttributes::SCALARS,
                           vtkDataSetAttributes::TENSORS,
                           vtkAssignAttribute::POINT_DATA);
  scalarsToTensors->SetInputConnection(reslicer->GetOutputPort());

  scalarsToTensors->Update();
  return true;
}

}
