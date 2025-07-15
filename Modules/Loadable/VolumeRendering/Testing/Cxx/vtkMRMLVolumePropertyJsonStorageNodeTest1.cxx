/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyJsonStorageNode.h"

// vtk includes
#include <vtkContourValues.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

//---------------------------------------------------------------------------
const double DOUBLE_TOLERANCE = 1e-6;

//---------------------------------------------------------------------------
int ComparePiecewiseFunction(vtkPiecewiseFunction* actualFunction, vtkPiecewiseFunction* expectedFunction)
{
  CHECK_NOT_NULL(expectedFunction);
  CHECK_NOT_NULL(actualFunction);

  CHECK_INT(actualFunction->GetSize(), expectedFunction->GetSize());

  for (int i = 0; i < expectedFunction->GetSize(); ++i)
  {
    double expectedValues[4];
    double actualValues[4];
    expectedFunction->GetNodeValue(i, expectedValues);
    actualFunction->GetNodeValue(i, actualValues);
    for (int j = 0; j < 4; ++j)
    {
      CHECK_DOUBLE_TOLERANCE(actualValues[j], expectedValues[j], DOUBLE_TOLERANCE);
    }
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int CompareColorTransferFunction(vtkColorTransferFunction* actualFunction, vtkColorTransferFunction* expectedFunction)
{
  CHECK_NOT_NULL(expectedFunction);
  CHECK_NOT_NULL(actualFunction);

  CHECK_INT(actualFunction->GetSize(), expectedFunction->GetSize());
  for (int i = 0; i < expectedFunction->GetSize(); ++i)
  {
    double expectedValues[6];
    double actualValues[6];
    expectedFunction->GetNodeValue(i, expectedValues);
    actualFunction->GetNodeValue(i, actualValues);
    for (int j = 0; j < 6; ++j)
    {
      CHECK_DOUBLE_TOLERANCE(actualValues[j], expectedValues[j], DOUBLE_TOLERANCE);
    }
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int CompareVolumeProperty(vtkVolumeProperty* actualVolumeProperty, vtkVolumeProperty* expectedVolumeProperty)
{
  CHECK_NOT_NULL(actualVolumeProperty);
  CHECK_NOT_NULL(expectedVolumeProperty);

  CHECK_INT(actualVolumeProperty->GetIsoSurfaceValues()->GetNumberOfContours(), expectedVolumeProperty->GetIsoSurfaceValues()->GetNumberOfContours());
  for (int i = 0; i < actualVolumeProperty->GetIsoSurfaceValues()->GetNumberOfContours(); ++i)
  {
    CHECK_DOUBLE_TOLERANCE(actualVolumeProperty->GetIsoSurfaceValues()->GetValue(i), expectedVolumeProperty->GetIsoSurfaceValues()->GetValue(i), DOUBLE_TOLERANCE);
  }

  CHECK_INT(actualVolumeProperty->GetInterpolationType(), expectedVolumeProperty->GetInterpolationType());
  CHECK_INT(actualVolumeProperty->GetIndependentComponents(), expectedVolumeProperty->GetIndependentComponents());
  CHECK_DOUBLE_TOLERANCE(actualVolumeProperty->GetClippedVoxelIntensity(), expectedVolumeProperty->GetClippedVoxelIntensity(), DOUBLE_TOLERANCE);
  CHECK_BOOL(actualVolumeProperty->GetUseClippedVoxelIntensity(), expectedVolumeProperty->GetUseClippedVoxelIntensity());

  for (int i = 0; i < VTK_MAX_VRCOMP; ++i)
  {
    std::cout << "Component: " << i << std::endl;

    CHECK_DOUBLE_TOLERANCE(actualVolumeProperty->GetAmbient(i), expectedVolumeProperty->GetAmbient(i), DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(actualVolumeProperty->GetDiffuse(i), expectedVolumeProperty->GetDiffuse(i), DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(actualVolumeProperty->GetSpecular(i), expectedVolumeProperty->GetSpecular(i), DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(actualVolumeProperty->GetSpecularPower(i), expectedVolumeProperty->GetSpecularPower(i), DOUBLE_TOLERANCE);
    CHECK_BOOL(actualVolumeProperty->GetShade(i), expectedVolumeProperty->GetShade(i));
    CHECK_DOUBLE_TOLERANCE(actualVolumeProperty->GetComponentWeight(i), expectedVolumeProperty->GetComponentWeight(i), DOUBLE_TOLERANCE);

    CHECK_EXIT_SUCCESS(CompareColorTransferFunction(actualVolumeProperty->GetRGBTransferFunction(i), expectedVolumeProperty->GetRGBTransferFunction(i)));
    CHECK_EXIT_SUCCESS(ComparePiecewiseFunction(actualVolumeProperty->GetScalarOpacity(i), expectedVolumeProperty->GetScalarOpacity(i)));
    CHECK_EXIT_SUCCESS(ComparePiecewiseFunction(actualVolumeProperty->GetGradientOpacity(i), expectedVolumeProperty->GetGradientOpacity(i)));
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyJsonStorageNodeTest1(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }
  const char* tempDir = argv[1];

  vtkNew<vtkMRMLVolumePropertyJsonStorageNode> jsonStorageNode;
  EXERCISE_ALL_BASIC_MRML_METHODS(jsonStorageNode.GetPointer());

  vtkNew<vtkMRMLVolumePropertyNode> expectedVolumePropertyNode;
  expectedVolumePropertyNode->SetNumberOfIndependentComponents(VTK_MAX_VRCOMP);
  expectedVolumePropertyNode->SetName("ExpectedVolumePropertyNode");
  expectedVolumePropertyNode->SetEffectiveRange(0.0, 255.0);

  vtkVolumeProperty* expectedVolumeProperty = expectedVolumePropertyNode->GetVolumeProperty();
  CHECK_NOT_NULL(expectedVolumeProperty);

  vtkContourValues* contourValues = expectedVolumeProperty->GetIsoSurfaceValues();
  CHECK_NOT_NULL(contourValues);
  contourValues->SetNumberOfContours(5);
  contourValues->SetValue(0, 0.0);
  contourValues->SetValue(1, 50.0);
  contourValues->SetValue(2, 100.0);
  contourValues->SetValue(3, 150.0);
  contourValues->SetValue(4, 200.0);

  expectedVolumeProperty->SetIndependentComponents(true);
  expectedVolumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
  expectedVolumeProperty->SetUseClippedVoxelIntensity(true);
  expectedVolumeProperty->SetClippedVoxelIntensity(0.0);
  for (int i = 0; i < VTK_MAX_VRCOMP; ++i)
  {
    double fractionalValue = (0.5 + i) / VTK_MAX_VRCOMP;

    expectedVolumeProperty->SetComponentWeight(i, fractionalValue);

    vtkNew<vtkPiecewiseFunction> scalarOpacityFunction;
    scalarOpacityFunction->AddPoint(0.0, 0.0);
    scalarOpacityFunction->AddPoint(255.0 / (i + 1), fractionalValue);
    scalarOpacityFunction->AddPoint(255.0, 1.0);
    expectedVolumeProperty->SetScalarOpacity(i, scalarOpacityFunction);

    vtkNew<vtkPiecewiseFunction> gradientOpacityFunction;
    gradientOpacityFunction->AddPoint(0.0, 0.0);
    gradientOpacityFunction->AddPoint(255.0 / (i + 1), fractionalValue);
    gradientOpacityFunction->AddPoint(255.0, 1.0);
    expectedVolumeProperty->SetGradientOpacity(i, gradientOpacityFunction);

    expectedVolumeProperty->SetAmbient(i, fractionalValue);
    expectedVolumeProperty->SetDiffuse(i, fractionalValue);
    expectedVolumeProperty->SetSpecular(i, fractionalValue);
    expectedVolumeProperty->SetSpecularPower(i, fractionalValue);
    expectedVolumeProperty->SetShade(i, true);

    vtkNew<vtkPiecewiseFunction> grayTransferFunction;
    grayTransferFunction->AddPoint(0.0, 0.0, 0.0, 0.0);
    grayTransferFunction->AddPoint(255.0 / (i + 1), fractionalValue, fractionalValue, fractionalValue);
    grayTransferFunction->AddPoint(255.0, 1.0, 1.0, 1.0);
    expectedVolumeProperty->GetGrayTransferFunction(i)->DeepCopy(grayTransferFunction);
  }

  std::stringstream fileNameStream;
  fileNameStream << tempDir << "/VP1.vp.json";
  std::string fileNameString = fileNameStream.str();
  jsonStorageNode->SetFileName(fileNameString.c_str());
  CHECK_INT(jsonStorageNode->WriteData(expectedVolumePropertyNode), 1);

  vtkNew<vtkMRMLVolumePropertyNode> actualVolumePropertyNode;
  CHECK_INT(jsonStorageNode->ReadData(actualVolumePropertyNode), 1);
  actualVolumePropertyNode->SetName("ActualVolumePropertyNode");

  CHECK_DOUBLE_TOLERANCE(actualVolumePropertyNode->GetEffectiveRange()[0], expectedVolumePropertyNode->GetEffectiveRange()[0], DOUBLE_TOLERANCE);
  CHECK_DOUBLE_TOLERANCE(actualVolumePropertyNode->GetEffectiveRange()[1], expectedVolumePropertyNode->GetEffectiveRange()[1], DOUBLE_TOLERANCE);

  CHECK_EXIT_SUCCESS(CompareVolumeProperty(actualVolumePropertyNode->GetVolumeProperty(), expectedVolumePropertyNode->GetVolumeProperty()));

  return EXIT_SUCCESS;
}
