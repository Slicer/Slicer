#include "itkTestMain.h"

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"

// VTK includes
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// STD includes
#include <cmath>
#include <iostream>

#ifdef _WIN32
# define MODULE_IMPORT __declspec(dllimport)
#else
# define MODULE_IMPORT
#endif

extern "C" MODULE_IMPORT int ModuleEntryPoint(int, char*[]);

namespace
{

//-----------------------------------------------------------------------------
bool ReadModel(const char* fileName, vtkMRMLModelNode* modelNode)
{
  vtkNew<vtkMRMLModelStorageNode> storageNode;
  storageNode->SetFileName(fileName);
  if (!storageNode->ReadData(modelNode) || !modelNode->GetPolyData())
  {
    std::cerr << "Failed to read model " << fileName << std::endl;
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool CompareArrays(const char* name, vtkDataArray* baselineArray, vtkDataArray* testArray, double tolerance)
{
  if (!testArray)
  {
    std::cerr << "Array " << name << " is missing" << std::endl;
    return false;
  }
  if (baselineArray->GetNumberOfTuples() != testArray->GetNumberOfTuples() //
      || baselineArray->GetNumberOfComponents() != testArray->GetNumberOfComponents())
  {
    std::cerr << "Array " << name << " size mismatch: expected " << baselineArray->GetNumberOfTuples() << "x" << baselineArray->GetNumberOfComponents() << ", got "
              << testArray->GetNumberOfTuples() << "x" << testArray->GetNumberOfComponents() << std::endl;
    return false;
  }
  for (vtkIdType tupleIndex = 0; tupleIndex < baselineArray->GetNumberOfTuples(); ++tupleIndex)
  {
    for (int componentIndex = 0; componentIndex < baselineArray->GetNumberOfComponents(); ++componentIndex)
    {
      double expected = baselineArray->GetComponent(tupleIndex, componentIndex);
      double actual = testArray->GetComponent(tupleIndex, componentIndex);
      if (std::abs(expected - actual) > tolerance)
      {
        std::cerr << "Array " << name << " value mismatch at tuple " << tupleIndex << " component " << componentIndex << ": expected " << expected << ", got " << actual
                  << std::endl;
        return false;
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
bool CompareAttributeArrays(const char* attributeName, vtkDataSetAttributes* baselineAttributes, vtkDataSetAttributes* testAttributes, double tolerance)
{
  if (baselineAttributes->GetNumberOfArrays() != testAttributes->GetNumberOfArrays())
  {
    std::cerr << attributeName << " number of arrays mismatch: expected " << baselineAttributes->GetNumberOfArrays() << ", got " << testAttributes->GetNumberOfArrays()
              << std::endl;
    return false;
  }
  for (int arrayIndex = 0; arrayIndex < baselineAttributes->GetNumberOfArrays(); ++arrayIndex)
  {
    vtkDataArray* baselineArray = baselineAttributes->GetArray(arrayIndex);
    if (!baselineArray)
    {
      // Not a numeric array
      continue;
    }
    const char* arrayName = baselineArray->GetName();
    if (!CompareArrays(arrayName ? arrayName : "(unnamed)", baselineArray, testAttributes->GetArray(arrayName), tolerance))
    {
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
// Compare the content of two model files (points, cells, point and cell data).
// Files are not compared byte by byte, because the file content may differ
// for the same model, for example depending on the compression library.
int CompareModels(int argc, char* argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " baselineModel testModel" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkMRMLModelNode> baselineModelNode;
  vtkNew<vtkMRMLModelNode> testModelNode;
  if (!ReadModel(argv[1], baselineModelNode) || !ReadModel(argv[2], testModelNode))
  {
    return EXIT_FAILURE;
  }
  vtkPolyData* baseline = baselineModelNode->GetPolyData();
  vtkPolyData* test = testModelNode->GetPolyData();

  const double tolerance = 1e-6;

  if (baseline->GetNumberOfPoints() != test->GetNumberOfPoints())
  {
    std::cerr << "Number of points mismatch: expected " << baseline->GetNumberOfPoints() << ", got " << test->GetNumberOfPoints() << std::endl;
    return EXIT_FAILURE;
  }
  for (vtkIdType pointIndex = 0; pointIndex < baseline->GetNumberOfPoints(); ++pointIndex)
  {
    double expected[3] = { 0.0, 0.0, 0.0 };
    double actual[3] = { 0.0, 0.0, 0.0 };
    baseline->GetPoint(pointIndex, expected);
    test->GetPoint(pointIndex, actual);
    if (vtkMath::Distance2BetweenPoints(expected, actual) > tolerance * tolerance)
    {
      std::cerr << "Point " << pointIndex << " mismatch: expected (" << expected[0] << ", " << expected[1] << ", " << expected[2] << "), got (" << actual[0] << ", " << actual[1]
                << ", " << actual[2] << ")" << std::endl;
      return EXIT_FAILURE;
    }
  }

  if (baseline->GetNumberOfCells() != test->GetNumberOfCells())
  {
    std::cerr << "Number of cells mismatch: expected " << baseline->GetNumberOfCells() << ", got " << test->GetNumberOfCells() << std::endl;
    return EXIT_FAILURE;
  }
  vtkNew<vtkIdList> expectedPointIds;
  vtkNew<vtkIdList> actualPointIds;
  for (vtkIdType cellIndex = 0; cellIndex < baseline->GetNumberOfCells(); ++cellIndex)
  {
    if (baseline->GetCellType(cellIndex) != test->GetCellType(cellIndex))
    {
      std::cerr << "Cell " << cellIndex << " type mismatch: expected " << baseline->GetCellType(cellIndex) << ", got " << test->GetCellType(cellIndex) << std::endl;
      return EXIT_FAILURE;
    }
    baseline->GetCellPoints(cellIndex, expectedPointIds);
    test->GetCellPoints(cellIndex, actualPointIds);
    bool samePoints = (expectedPointIds->GetNumberOfIds() == actualPointIds->GetNumberOfIds());
    for (vtkIdType i = 0; samePoints && i < expectedPointIds->GetNumberOfIds(); ++i)
    {
      samePoints = (expectedPointIds->GetId(i) == actualPointIds->GetId(i));
    }
    if (!samePoints)
    {
      std::cerr << "Cell " << cellIndex << " point ids mismatch" << std::endl;
      return EXIT_FAILURE;
    }
  }

  if (!CompareAttributeArrays("Point data", baseline->GetPointData(), test->GetPointData(), tolerance) //
      || !CompareAttributeArrays("Cell data", baseline->GetCellData(), test->GetCellData(), tolerance))
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

} // end of anonymous namespace

void RegisterTests()
{
  StringToTestFunctionMap["ModuleEntryPoint"] = ModuleEntryPoint;
  StringToTestFunctionMap["CompareModels"] = CompareModels;
}
