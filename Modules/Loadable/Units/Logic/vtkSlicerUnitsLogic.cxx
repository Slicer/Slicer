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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Units Logic includes
#include "vtkSlicerUnitsLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkNew.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerUnitsLogic);

//----------------------------------------------------------------------------
vtkSlicerUnitsLogic::vtkSlicerUnitsLogic()
{
  this->UnitsScene = vtkMRMLScene::New();
  this->AddBuiltInUnits(this->UnitsScene);
}

//----------------------------------------------------------------------------
vtkSlicerUnitsLogic::~vtkSlicerUnitsLogic()
{
  if (this->UnitsScene)
    {
    this->UnitsScene->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerUnitsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLUnitNode* vtkSlicerUnitsLogic
::AddUnitNode(const char* name, const char* quantity, const char* prefix,
              const char* suffix, int precision, double min, double max)
{
  return this->AddUnitNodeToScene(this->GetMRMLScene(), name, quantity,
    prefix, suffix, precision, min, max);
}

//----------------------------------------------------------------------------
vtkMRMLScene* vtkSlicerUnitsLogic::GetUnitsScene() const
{
  return this->UnitsScene;
}

//----------------------------------------------------------------------------
vtkMRMLUnitNode* vtkSlicerUnitsLogic
::AddUnitNodeToScene(vtkMRMLScene* scene, const char* name,
                     const char* quantity, const char* prefix,
                     const char* suffix, int precision, double min, double max)
{
  if (!scene)
    {
    return 0;
    }

  vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::New();
  unitNode->SetName(name);
  unitNode->SetQuantity(quantity);
  unitNode->SetPrefix(prefix);
  unitNode->SetSuffix(suffix);
  unitNode->SetPrecision(precision);
  unitNode->SetMinimumValue(min);
  unitNode->SetMaximumValue(max);

  scene->AddNode(unitNode);
  unitNode->Delete();
  return unitNode;
}

//---------------------------------------------------------------------------
void vtkSlicerUnitsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  this->Superclass::SetMRMLSceneInternal(newScene);

  this->AddDefaultsUnits();
}

//---------------------------------------------------------------------------
void vtkSlicerUnitsLogic::AddDefaultsUnits()
{
  vtkMRMLUnitNode* node =
    this->AddUnitNode("ApplicationLength", "length", "", "mm", 3);
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());

  node = this->AddUnitNode("ApplicationTime", "time", "", "s", 3);
  node->SetSaveWithScene(false);
  this->SetDefaultUnit(node->GetQuantity(), node->GetID());
}

//---------------------------------------------------------------------------
void vtkSlicerUnitsLogic::AddBuiltInUnits(vtkMRMLScene* scene)
{
  if (!scene)
    {
    return;
    }

  this->RegisterNodesInternal(scene);

  // Add defaults nodes here
  this->AddUnitNodeToScene(scene,
    "metre", "length", "", "m", 3);
  this->AddUnitNodeToScene(scene,
    "centimetre", "length", "", "cm", 3);
  this->AddUnitNodeToScene(scene,
    "millimetre", "length", "", "mm", 3);
  this->AddUnitNodeToScene(scene,
    "micrometre", "length", "", "µm", 3);
  this->AddUnitNodeToScene(scene,
    "nanometre", "length", "", "nm", 3);

  this->AddUnitNodeToScene(scene,
    "year", "time", "", "year", 3);
  this->AddUnitNodeToScene(scene,
    "month", "time", "", "month", 3);
  this->AddUnitNodeToScene(scene,
    "day", "time", "", "day", 3);
  this->AddUnitNodeToScene(scene,
    "hour", "time", "", "h", 3);
  this->AddUnitNodeToScene(scene,
    "second", "time", "", "s", 3);
  this->AddUnitNodeToScene(scene,
    "millisecond", "time", "", "ms", 3);
  this->AddUnitNodeToScene(scene,
    "microsecond", "time", "", "µs", 3);
}

//-----------------------------------------------------------------------------
void vtkSlicerUnitsLogic::SetDefaultUnit(const char* quantity, const char* id)
{
  if (!quantity || !this->GetMRMLScene())
    {
    return;
    }

  vtkMRMLSelectionNode* selectionNode =  vtkMRMLSelectionNode::SafeDownCast(
    this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (selectionNode)
    {
    selectionNode->SetUnitNodeID(quantity, id);
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerUnitsLogic::RegisterNodes()
{
  this->RegisterNodesInternal(this->GetMRMLScene());
}

//-----------------------------------------------------------------------------
void vtkSlicerUnitsLogic::RegisterNodesInternal(vtkMRMLScene* scene)
{
  assert(scene != 0);

  vtkNew<vtkMRMLUnitNode> unitNode;
  scene->RegisterNodeClass(unitNode.GetPointer());
}
