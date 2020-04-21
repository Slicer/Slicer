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

// DoubleArrays Logic includes
#include "vtkSlicerDoubleArraysLogic.h"

// MRML includes
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLDoubleArrayStorageNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerDoubleArraysLogic);

//----------------------------------------------------------------------------
vtkSlicerDoubleArraysLogic::vtkSlicerDoubleArraysLogic() = default;

//----------------------------------------------------------------------------
vtkSlicerDoubleArraysLogic::~vtkSlicerDoubleArraysLogic() = default;

//----------------------------------------------------------------------------
void vtkSlicerDoubleArraysLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLDoubleArrayNode* vtkSlicerDoubleArraysLogic
::AddDoubleArray(const char* fileName, const char* name)
{
  if (!fileName)
    {
    return nullptr;
    }
  if (fileName[0] == '\0')
    {
    return nullptr;
    }
  if (this->GetMRMLScene() == nullptr)
    {
    return nullptr;
    }

  // Storage node
  vtkNew<vtkMRMLDoubleArrayStorageNode> doubleArrayStorageNode;
  doubleArrayStorageNode->SetFileName(fileName);
  this->GetMRMLScene()->AddNode(doubleArrayStorageNode.GetPointer());

  // Storable node
  vtkNew<vtkMRMLDoubleArrayNode> doubleArrayNode;
  this->GetMRMLScene()->AddNode(doubleArrayNode.GetPointer());

  // Read
  int res = doubleArrayStorageNode->ReadData(doubleArrayNode.GetPointer());
  if (res == 0) // failed to read
    {
    this->GetMRMLScene()->RemoveNode(doubleArrayStorageNode.GetPointer());
    this->GetMRMLScene()->RemoveNode(doubleArrayNode.GetPointer());
    return nullptr;
    }
  if (name)
    {
    doubleArrayNode->SetName(name);
    }
  return doubleArrayNode.GetPointer();
}
