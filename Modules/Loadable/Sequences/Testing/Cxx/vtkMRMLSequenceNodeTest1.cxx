/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// MRML includes
#include <vtkMRMLSequenceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkTestingOutputWindow.h"

//-----------------------------------------------------------------------------
bool testAddInvalidFile(const char* filePath);
bool testAddFile(const char* filePath);

//-----------------------------------------------------------------------------
bool SequenceSortedByIndex(vtkMRMLSequenceNode* seqNode)
{
  for (int i = 1; i < seqNode->GetNumberOfDataNodes(); i++)
    {
    double previousIndexValue = atof(seqNode->GetNthIndexValue(i - 1).c_str());
    double currentIndexValue = atof(seqNode->GetNthIndexValue(i).c_str());
    if (previousIndexValue >= currentIndexValue)
      {
      std::cout << "Sequence is not sorted: index[" << i - 1 << "] = " << previousIndexValue
        << ", index[" << i << "] = " << currentIndexValue << "\n";
      return false;
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
int vtkMRMLSequenceNodeTest1( int, char * [] )
{
  vtkNew< vtkMRMLSequenceNode > seqNode;
  EXERCISE_ALL_BASIC_MRML_METHODS(seqNode.GetPointer());

  seqNode->SetIndexType(vtkMRMLSequenceNode::NumericIndex);

  // Add data nodes at indices 5, 15, 25, 35, ..., 495
  vtkNew<vtkMRMLTransformNode> dataNode;
  vtkNew<vtkMatrix4x4> transformMatrix;
  int numberOfDataNodes = 50;
  for (int i = 0; i < numberOfDataNodes; i++)
    {
    std::ostringstream indexStr;
    indexStr << i*10.0+5.0;
    transformMatrix->SetElement(1, 3, i * 20.0);
    dataNode->SetMatrixTransformFromParent(transformMatrix.GetPointer());
    seqNode->SetDataNodeAtValue(dataNode.GetPointer(), indexStr.str().c_str());
    }
  CHECK_INT(seqNode->GetNumberOfDataNodes(), numberOfDataNodes);

  // Updating and existing data node
  transformMatrix->SetElement(0, 3, 5.0);
  dataNode->SetMatrixTransformFromParent(transformMatrix.GetPointer());
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "35");
  CHECK_INT(seqNode->GetNumberOfDataNodes(), numberOfDataNodes);

  // Updating and existing data node, with different string formatting
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "35.0");
  CHECK_INT(seqNode->GetNumberOfDataNodes(), numberOfDataNodes);

  // Updating and existing data node, with tolerance
  seqNode->SetNumericIndexValueTolerance(0.001);
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "35.0001");
  CHECK_INT(seqNode->GetNumberOfDataNodes(), numberOfDataNodes);

  // Adding new data node
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "35.1");
  CHECK_INT(seqNode->GetNumberOfDataNodes(), numberOfDataNodes+1);

  // Adding a few more data nodes to check sorting
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "1.0");
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "-1.0");
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "1000");
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "25");
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "15");
  seqNode->SetDataNodeAtValue(dataNode.GetPointer(), "203");

  // Check if items are sorted correctly after a new node was inserted
  CHECK_BOOL(SequenceSortedByIndex(seqNode.GetPointer()), true);

  // Check if items are sorted correctly index values are modified
  seqNode->UpdateIndexValue("25", "26");
  CHECK_BOOL(SequenceSortedByIndex(seqNode.GetPointer()), true);
  seqNode->UpdateIndexValue("15", "96");
  CHECK_BOOL(SequenceSortedByIndex(seqNode.GetPointer()), true);
  seqNode->UpdateIndexValue("96", "32");
  CHECK_BOOL(SequenceSortedByIndex(seqNode.GetPointer()), true);

  /*
  bool res = true;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddInvalidFile(0) && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddInvalidFile("") && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddInvalidFile("non existing file.badextension") && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  res = testAddInvalidFile("non existing file.vtk") && res;
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (argc > 1)
    {
    res = testAddFile(argv[1]) && res;
    }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
  */
  return EXIT_SUCCESS;
}

/*
//-----------------------------------------------------------------------------
bool testAddInvalidFile(const char * filePath)
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerTablesLogic> tablesLogic;
  tablesLogic->SetMRMLScene(scene.GetPointer());

  int nodeCount = scene->GetNumberOfNodes();
  vtkMRMLTableNode* table = tablesLogic->AddTable(filePath);

  if (table != 0 ||
      scene->GetNumberOfNodes() != nodeCount)
    {
    std::cerr << "Line " << __LINE__
              << ": Adding an invalid file ("<< (filePath ? filePath : 0)
              << ") shall not add nodes in scene. "
              << scene->GetNumberOfNodes() << " vs " << nodeCount
              << std::endl;
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool testAddFile(const char * filePath)
{
  vtkNew<vtkSlicerTablesLogic> tablesLogic;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkMRMLTableNode* table = tablesLogic->AddTable(filePath);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (table != 0)
    {
    std::cerr << "Line " << __LINE__
              << ": File can't be loaded if no scene is set."
              << std::endl;
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  tablesLogic->SetMRMLScene(scene.GetPointer());
  int nodeCount = scene->GetNumberOfNodes();
  table = tablesLogic->AddTable(filePath);

  if (table == 0 ||
      scene->GetNumberOfNodes() != nodeCount + 2)
    {
    std::cerr << "Adding an table should create 2 nodes" << std::endl;
    return false;
    }

  if (table->GetNumberOfRows()==0 || table->GetNumberOfColumns()==0)
    {
    std::cerr << "Table is empty" << std::endl;
    return false;
    }

  return true;
}
*/
