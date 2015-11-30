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

#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"

#include "vtkTable.h"
#include "vtkTestErrorObserver.h"

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLCoreTestingUtilities.h"

int vtkMRMLTableNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLTableNode > node1 = vtkSmartPointer< vtkMRMLTableNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLTableNode, node1);


  vtkSmartPointer< vtkMRMLTableNode > node2 = vtkSmartPointer< vtkMRMLTableNode >::New();

  vtkSmartPointer<vtkTest::ErrorObserver> errorWarningObserver = vtkSmartPointer<vtkTest::ErrorObserver>::New();
  node2->AddObserver(vtkCommand::WarningEvent, errorWarningObserver);
  node2->AddObserver(vtkCommand::ErrorEvent, errorWarningObserver);

  vtkTable* table = node2->GetTable();
  CHECK_NOT_NULL(table);

  // Verify if a proper storage node is created
  vtkSmartPointer< vtkMRMLTableStorageNode > storageNode = vtkSmartPointer< vtkMRMLTableStorageNode >::Take(vtkMRMLTableStorageNode::SafeDownCast(node2->CreateDefaultStorageNode()));
  CHECK_NOT_NULL(storageNode);

  // Verify basic add/remove column methods

  CHECK_NOT_NULL(node2->AddColumn());
  CHECK_INT(table->GetNumberOfColumns(), 1);

  vtkSmartPointer< vtkStringArray > newEmptyArray = vtkSmartPointer< vtkStringArray >::New();
  CHECK_NOT_NULL(node2->AddColumn(newEmptyArray));
  CHECK_INT(table->GetNumberOfColumns(), 2);

  CHECK_BOOL(node2->RemoveColumn(1), true);
  CHECK_INT(table->GetNumberOfColumns(), 1);

  CHECK_INT(node2->AddEmptyRow(), 0);
  CHECK_INT(table->GetNumberOfRows(), 1);

  CHECK_INT(node2->AddEmptyRow(), 1);
  CHECK_INT(node2->AddEmptyRow(), 2);
  CHECK_INT(node2->AddEmptyRow(), 3);
  CHECK_INT(table->GetNumberOfRows(), 4);

  CHECK_BOOL(node2->RemoveRow(1), true);
  CHECK_INT(table->GetNumberOfRows(), 3);

  // Verify that arrays that are shorter than the table size are extended to match the current table size

  vtkSmartPointer< vtkStringArray > newShortArray = vtkSmartPointer< vtkStringArray >::New();
  newShortArray->InsertNextValue("something");
  CHECK_NOT_NULL(node2->AddColumn(newShortArray));
  CHECK_INT(table->GetNumberOfRows(), 3);
  CHECK_INT(table->GetNumberOfColumns(), 2);
  CHECK_INT(newShortArray->GetNumberOfTuples(), table->GetNumberOfRows());

  // Verify that arrays that are shorter than the table extend the table

  vtkSmartPointer< vtkStringArray > newLongArray = vtkSmartPointer< vtkStringArray >::New();
  newLongArray->InsertNextValue("something1");
  newLongArray->InsertNextValue("something2");
  newLongArray->InsertNextValue("something3");
  newLongArray->InsertNextValue("something4");
  newLongArray->InsertNextValue("something5");
  newLongArray->InsertNextValue("something6");
  newLongArray->InsertNextValue("something7");
  newLongArray->InsertNextValue("something8");
  newLongArray->InsertNextValue("something9");
  newLongArray->InsertNextValue("something10");

  CHECK_NOT_NULL(node2->AddColumn(newLongArray));
  CHECK_INT(table->GetNumberOfRows(), 10);
  CHECK_INT(table->GetNumberOfColumns(), 3);

  // Test GetCellText

  CHECK_INT(newLongArray->GetNumberOfTuples(), table->GetNumberOfRows());
  CHECK_STD_STRING(node2->GetCellText(2,2), "something3");

  CHECK_BOOL(errorWarningObserver->GetError(), false);
  CHECK_BOOL(errorWarningObserver->GetWarning(), false);

  CHECK_STD_STRING(node2->GetCellText(20,2), ""); // error log is expected
  CHECK_BOOL(errorWarningObserver->GetError(), true);
  errorWarningObserver->Clear();

  CHECK_STD_STRING(node2->GetCellText(20,2), ""); // error log is expected
  CHECK_BOOL(errorWarningObserver->GetError(), true);
  errorWarningObserver->Clear();

  CHECK_BOOL(node2->SetCellText(2,2,"ModifiedText"), true);
  CHECK_STD_STRING(node2->GetCellText(2,2), "ModifiedText");

  // Test SetCellText

  CHECK_BOOL(node2->SetCellText(20,2,"invalid"), false);
  CHECK_BOOL(node2->SetCellText(2,20,"invalid"), false);

  // Verify that Copy method creates a true independent copy
  vtkSmartPointer< vtkMRMLTableNode > node2copy = vtkSmartPointer< vtkMRMLTableNode >::New();
  node2copy->Copy(node2);
  // After copying the contents of the tables should be the same
  CHECK_STD_STRING(node2->GetCellText(0,0), node2copy->GetCellText(0,0));

  // After modifying the copied version, the tables should be different
  // (if there was a shallow copy only, the original table would have been changed, too)
  CHECK_BOOL(node2copy->SetCellText(0,0,"someModifiedText"), true);
  CHECK_STD_STRING_DIFFERENT(node2->GetCellText(0,0), node2copy->GetCellText(0,0));

  std::cout << "vtkMRMLTableNodeTest1 completed successfully" << std::endl;
  return EXIT_SUCCESS;
}
