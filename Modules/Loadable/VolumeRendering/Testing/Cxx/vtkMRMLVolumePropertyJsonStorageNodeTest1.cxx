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

// STD includes
#include <cmath>
#include <fstream>
#include <iostream>

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
// Verify that loading a JSON file with duplicate x coordinates in scalarOpacity,
// gradientOpacity, and rgbTransferFunction preserves all points, separates them
// so x values are strictly increasing, and retains file order for duplicate pairs.
//
// Duplicate x values arise when VTK widget arithmetic passes coordinates through
// float32 (vtkVector2f), collapsing two near-duplicate doubles into the same
// float32 value after a shift operation.
int TestTransferFunctionDuplicateXValuesOnLoad(const char* tempDir)
{
  // Write the JSON directly to control exact x values including duplicates,
  // bypassing VTK's piecewise/color function insert which would deduplicate.
  //
  // scalarOpacity / gradientOpacity: duplicate in the middle with y=1 then y=0
  //   (a sharp opacity spike — rises then immediately falls).
  // rgbTransferFunction: duplicate in the middle with distinct colors
  //   (red then green) so file order is unambiguous.
  std::string fileName = std::string(tempDir) + "/TransferFunctionWithDuplicateX.vp.json";
  {
    std::ofstream f(fileName);
    f << R"({
    "@schema": "https://raw.githubusercontent.com/slicer/slicer/main/Modules/Loadable/VolumeRendering/Resources/Schema/volume-property-schema-v1.0.0.json#",
    "volumeProperties": [{ "components": [{
        "scalarOpacity": { "type": "piecewiseLinearFunction", "points": [
            { "x": -300.0, "y": 0.0 },
            { "x": -150.0, "y": 1.0 },
            { "x": -150.0, "y": 0.0 },
            { "x": 2000.0, "y": 0.0 }
        ]},
        "gradientOpacity": { "type": "piecewiseLinearFunction", "points": [
            { "x":    0.0, "y": 0.0 },
            { "x":  500.0, "y": 1.0 },
            { "x":  500.0, "y": 0.0 },
            { "x": 1000.0, "y": 0.0 }
        ]},
        "rgbTransferFunction": { "type": "colorTransferFunction", "points": [
            { "x":  10000.0, "color": [0.0, 0.0, 0.0] },
            { "x":  15000.0, "color": [1.0, 0.0, 0.0] },
            { "x":  15000.0, "color": [0.0, 1.0, 0.0] },
            { "x": 100000.0, "color": [1.0, 1.0, 1.0] }
        ]}
    }]}]
    })";
  }

  vtkNew<vtkMRMLVolumePropertyJsonStorageNode> storageNode;
  storageNode->SetFileName(fileName.c_str());
  vtkNew<vtkMRMLVolumePropertyNode> vpNode;
  CHECK_INT(storageNode->ReadData(vpNode), 1);

  // --- scalarOpacity ---
  {
    vtkPiecewiseFunction* opacity = vpNode->GetVolumeProperty()->GetScalarOpacity(0);
    // All 4 points must survive — without the HigherAndUnique fix, AddPoint's
    // deduplication (AllowDuplicateScalars=false) silently drops the first point.
    CHECK_INT(opacity->GetSize(), 4);
    double p[4][4];
    for (int i = 0; i < 4; ++i)
    {
      opacity->GetNodeValue(i, p[i]);
    }
    CHECK_BOOL(p[0][0] < p[1][0], true);
    CHECK_BOOL(p[1][0] < p[2][0], true);
    CHECK_BOOL(p[2][0] < p[3][0], true);
    CHECK_DOUBLE_TOLERANCE(p[0][0], -300.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[3][0], 2000.0, DOUBLE_TOLERANCE);
    CHECK_BOOL(std::abs(p[1][0] - -150.0) < 1.0, true);
    CHECK_BOOL(std::abs(p[2][0] - -150.0) < 1.0, true);
    // File order preserved: spike up (y=1) before spike down (y=0).
    CHECK_DOUBLE_TOLERANCE(p[0][1], 0.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[1][1], 1.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[2][1], 0.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[3][1], 0.0, DOUBLE_TOLERANCE);
  }

  // --- gradientOpacity ---
  {
    vtkPiecewiseFunction* gradient = vpNode->GetVolumeProperty()->GetGradientOpacity(0);
    CHECK_INT(gradient->GetSize(), 4);
    double p[4][4];
    for (int i = 0; i < 4; ++i)
    {
      gradient->GetNodeValue(i, p[i]);
    }
    CHECK_BOOL(p[0][0] < p[1][0], true);
    CHECK_BOOL(p[1][0] < p[2][0], true);
    CHECK_BOOL(p[2][0] < p[3][0], true);
    CHECK_DOUBLE_TOLERANCE(p[0][0], 0.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[3][0], 1000.0, DOUBLE_TOLERANCE);
    CHECK_BOOL(std::abs(p[1][0] - 500.0) < 1.0, true);
    CHECK_BOOL(std::abs(p[2][0] - 500.0) < 1.0, true);
    CHECK_DOUBLE_TOLERANCE(p[0][1], 0.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[1][1], 1.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[2][1], 0.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[3][1], 0.0, DOUBLE_TOLERANCE);
  }

  // --- rgbTransferFunction ---
  // GetNodeValue returns [x, r, g, b, midpoint, sharpness].
  {
    vtkColorTransferFunction* color = vpNode->GetVolumeProperty()->GetRGBTransferFunction(0);
    CHECK_INT(color->GetSize(), 4);
    double p[4][6];
    for (int i = 0; i < 4; ++i)
    {
      color->GetNodeValue(i, p[i]);
    }
    CHECK_BOOL(p[0][0] < p[1][0], true);
    CHECK_BOOL(p[1][0] < p[2][0], true);
    CHECK_BOOL(p[2][0] < p[3][0], true);
    CHECK_DOUBLE_TOLERANCE(p[0][0], 10000.0, DOUBLE_TOLERANCE);
    CHECK_DOUBLE_TOLERANCE(p[3][0], 100000.0, DOUBLE_TOLERANCE);
    CHECK_BOOL(std::abs(p[1][0] - 15000.0) < 1.0, true);
    CHECK_BOOL(std::abs(p[2][0] - 15000.0) < 1.0, true);
    // File order preserved: red before green.
    CHECK_DOUBLE_TOLERANCE(p[0][1], 0.0, DOUBLE_TOLERANCE); // black r
    CHECK_DOUBLE_TOLERANCE(p[1][1], 1.0, DOUBLE_TOLERANCE); // red   r
    CHECK_DOUBLE_TOLERANCE(p[1][2], 0.0, DOUBLE_TOLERANCE); // red   g
    CHECK_DOUBLE_TOLERANCE(p[2][1], 0.0, DOUBLE_TOLERANCE); // green r
    CHECK_DOUBLE_TOLERANCE(p[2][2], 1.0, DOUBLE_TOLERANCE); // green g
    CHECK_DOUBLE_TOLERANCE(p[3][1], 1.0, DOUBLE_TOLERANCE); // white r
    CHECK_DOUBLE_TOLERANCE(p[3][2], 1.0, DOUBLE_TOLERANCE); // white g
    CHECK_DOUBLE_TOLERANCE(p[3][3], 1.0, DOUBLE_TOLERANCE); // white b
  }

  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
// Verify that gradient opacity points stored out of x order in the JSON are
// sorted correctly on load.
int TestTransferFunctionUnsortedXValuesOnLoad(const char* tempDir)
{
  std::string fileName = std::string(tempDir) + "/GradientOpacityUnsorted.vp.json";
  {
    std::ofstream f(fileName);
    f << R"({
    "@schema": "https://raw.githubusercontent.com/slicer/slicer/main/Modules/Loadable/VolumeRendering/Resources/Schema/volume-property-schema-v1.0.0.json#",
    "volumeProperties": [{ "components": [{ "gradientOpacity": {
        "type": "piecewiseLinearFunction",
        "points": [
            { "x": 1000.0, "y": 0.0 },
            { "x":    0.0, "y": 0.0 },
            { "x":  500.0, "y": 0.5 },
            { "x":  750.0, "y": 1.0 }
    ]}}]}]
    })";
  }

  vtkNew<vtkMRMLVolumePropertyJsonStorageNode> storageNode;
  storageNode->SetFileName(fileName.c_str());
  vtkNew<vtkMRMLVolumePropertyNode> vpNode;
  CHECK_INT(storageNode->ReadData(vpNode), 1);

  vtkPiecewiseFunction* gradient = vpNode->GetVolumeProperty()->GetGradientOpacity(0);
  CHECK_INT(gradient->GetSize(), 4);

  double p[4][4];
  for (int i = 0; i < 4; ++i)
  {
    gradient->GetNodeValue(i, p[i]);
  }

  // Points must be sorted by x regardless of file order.
  CHECK_DOUBLE_TOLERANCE(p[0][0], 0.0, DOUBLE_TOLERANCE);
  CHECK_DOUBLE_TOLERANCE(p[1][0], 500.0, DOUBLE_TOLERANCE);
  CHECK_DOUBLE_TOLERANCE(p[2][0], 750.0, DOUBLE_TOLERANCE);
  CHECK_DOUBLE_TOLERANCE(p[3][0], 1000.0, DOUBLE_TOLERANCE);

  // y values must follow the sorted x values, not the original file order.
  CHECK_DOUBLE_TOLERANCE(p[0][1], 0.0, DOUBLE_TOLERANCE);
  CHECK_DOUBLE_TOLERANCE(p[1][1], 0.5, DOUBLE_TOLERANCE);
  CHECK_DOUBLE_TOLERANCE(p[2][1], 1.0, DOUBLE_TOLERANCE);
  CHECK_DOUBLE_TOLERANCE(p[3][1], 0.0, DOUBLE_TOLERANCE);

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

  CHECK_EXIT_SUCCESS(TestTransferFunctionDuplicateXValuesOnLoad(tempDir));
  CHECK_EXIT_SUCCESS(TestTransferFunctionUnsortedXValuesOnLoad(tempDir));

  return EXIT_SUCCESS;
}
