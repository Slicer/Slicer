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

#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableSQLiteStorageNode.h"

#include "vtkFloatArray.h"
#include "vtkTable.h"
#include "vtkTestErrorObserver.h"

// ITKSYS includes
#include <itksys/SystemTools.hxx>

#include "vtkMRMLCoreTestingMacros.h"

static int removeFile(char *fileName)
{
  int removed = 1;
  if (itksys::SystemTools::FileExists(fileName))
  {
    removed = itksys::SystemTools::RemoveFile(fileName);
  }
  return removed;
}

int vtkMRMLTableSQLiteStorageNodeTest(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLTableNode> tableNode;
  scene->AddNode(tableNode.GetPointer());

  vtkNew< vtkMRMLTableSQLiteStorageNode > storageNode;
  scene->AddNode(storageNode.GetPointer());

  tableNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // Create a table with some points in it...
  vtkNew<vtkTable> table;
  vtkNew<vtkFloatArray> arrX;
  arrX->SetName("X_Axis");
  table->AddColumn(arrX.GetPointer());
  vtkNew<vtkFloatArray> arrC;
  arrC->SetName("Cosine");
  table->AddColumn(arrC.GetPointer());
  vtkNew<vtkFloatArray> arrS;
  arrS->SetName("Sine");
  table->AddColumn(arrS.GetPointer());

  // add few  points...
  int numPoints = 29;
  float inc = 7.0 / (numPoints-1);
  table->SetNumberOfRows(numPoints);
  for (int i = 0; i < numPoints; ++i)
    {
    table->SetValue(i, 0, i * inc);
    table->SetValue(i, 1, cos(i * inc) + 0.0);
    table->SetValue(i, 2, sin(i * inc) + 0.0);
    }

  tableNode->SetAndObserveTable(table.GetPointer());

  storageNode->SetFileName("testSQLite.db");
  storageNode->SetTableName("SinCos");
  removeFile(storageNode->GetFileName());

  storageNode->WriteData(tableNode.GetPointer());

  tableNode->RemoveAllColumns();
  if (tableNode->GetNumberOfColumns() != 0)
    {
    std::cerr << "Unable to remove columns " << std::endl;
    removeFile(storageNode->GetFileName());
    return EXIT_FAILURE;
    }

  // read table from the database
  storageNode->ReadData(tableNode.GetPointer());

  if (tableNode->GetNumberOfColumns() != 3)
    {
    std::cerr << "Unable to read table columns from the database " << storageNode->GetFileName() <<std::endl;
    removeFile(storageNode->GetFileName());
    return EXIT_FAILURE;
    }

  if (tableNode->GetNumberOfRows() != numPoints)
    {
    std::cerr << "Unable to read table rows from the database " << storageNode->GetFileName() <<std::endl;
    removeFile(storageNode->GetFileName());
    return EXIT_FAILURE;
    }

  // clean up
  removeFile(storageNode->GetFileName());

  std::cout << "vtkMRMLTableSQLiteStorageNodeTest completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
