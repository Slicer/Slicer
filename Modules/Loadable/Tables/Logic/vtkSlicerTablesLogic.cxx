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

// Tables Logic includes
#include "vtkSlicerTablesLogic.h"

// MRML includes
#include <vtkMRMLTableNode.h>
#include <vtkMRMLTableStorageNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTablesLogic);

//----------------------------------------------------------------------------
vtkSlicerTablesLogic::vtkSlicerTablesLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTablesLogic::~vtkSlicerTablesLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerTablesLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLTableNode* vtkSlicerTablesLogic
::AddTable(const char* fileName, const char* name)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("vtkSlicerTablesLogic::AddTable failed: scene is invalid");
    return 0;
    }
  if (!fileName)
    {
    vtkErrorMacro("vtkSlicerTablesLogic::AddTable failed: fileName is invalid");
    return 0;
    }

  // Storage node
  vtkNew<vtkMRMLTableStorageNode> tableStorageNode;
  tableStorageNode->SetFileName(fileName);
  this->GetMRMLScene()->AddNode(tableStorageNode.GetPointer());

  // Storable node
  vtkNew<vtkMRMLTableNode> tableNode;
  if (name)
    {
    tableNode->SetName(name);
    }
  this->GetMRMLScene()->AddNode(tableNode.GetPointer());

  // Read
  int res = tableStorageNode->ReadData(tableNode.GetPointer());
  if (res == 0) // failed to read
    {
    vtkErrorMacro("vtkSlicerTablesLogic::AddTable failed: failed to read data from file: "<<fileName);
    this->GetMRMLScene()->RemoveNode(tableStorageNode.GetPointer());
    this->GetMRMLScene()->RemoveNode(tableNode.GetPointer());
    return 0;
    }
  return tableNode.GetPointer();
}
