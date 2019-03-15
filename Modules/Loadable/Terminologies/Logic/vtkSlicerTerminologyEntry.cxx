/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// DICOMLib includes
#include "vtkSlicerTerminologyEntry.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTerminologyEntry);

//----------------------------------------------------------------------------
vtkSlicerTerminologyEntry::vtkSlicerTerminologyEntry()
{
  this->TerminologyContextName = nullptr;

  this->CategoryObject = nullptr;
  vtkSmartPointer<vtkSlicerTerminologyCategory> category = vtkSmartPointer<vtkSlicerTerminologyCategory>::New();
  this->SetCategoryObject(category);

  this->TypeObject = nullptr;
  vtkSmartPointer<vtkSlicerTerminologyType> type = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  this->SetTypeObject(type);

  this->TypeModifierObject = nullptr;
  vtkSmartPointer<vtkSlicerTerminologyType> typeModifier = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  this->SetTypeModifierObject(typeModifier);

  this->AnatomicContextName = nullptr;

  this->AnatomicRegionObject = nullptr;
  vtkSmartPointer<vtkSlicerTerminologyType> anatomicRegion = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  this->SetAnatomicRegionObject(anatomicRegion);

  this->AnatomicRegionModifierObject = nullptr;
  vtkSmartPointer<vtkSlicerTerminologyType> anatomicRegionModifier = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  this->SetAnatomicRegionModifierObject(anatomicRegionModifier);
}

//----------------------------------------------------------------------------
vtkSlicerTerminologyEntry::~vtkSlicerTerminologyEntry()
{
  this->SetTerminologyContextName(nullptr);

  this->SetCategoryObject(nullptr);
  this->SetTypeObject(nullptr);
  this->SetTypeModifierObject(nullptr);

  this->SetAnatomicContextName(nullptr);

  this->SetAnatomicRegionObject(nullptr);
  this->SetAnatomicRegionModifierObject(nullptr);
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyEntry::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "TerminologyContextName:   " << (this->TerminologyContextName?this->TerminologyContextName:"NULL") << "\n";

  os << indent << "CategoryObject: ";
  if (this->CategoryObject)
    {
    this->CategoryObject->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent.GetNextIndent() << "NULL\n";
    }
  os << indent << "TypeObject: ";
  if (this->TypeObject)
    {
    this->TypeObject->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent.GetNextIndent() << "NULL\n";
    }
  os << indent << "TypeModifierObject: ";
  if (this->TypeModifierObject)
    {
    this->TypeModifierObject->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent.GetNextIndent() << "NULL\n";
    }

  os << indent << "AnatomicContextName:   " << (this->AnatomicContextName?this->AnatomicContextName:"NULL") << "\n";
  os << indent << "AnatomicRegionObject: ";
  if (this->AnatomicRegionObject)
    {
    this->AnatomicRegionObject->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent.GetNextIndent() << "NULL\n";
    }
  os << indent << "AnatomicRegionModifierObject: ";
  if (this->AnatomicRegionModifierObject)
    {
    this->AnatomicRegionModifierObject->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent.GetNextIndent() << "NULL\n";
    }
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyEntry::Copy(vtkSlicerTerminologyEntry* aEntry)
{
  if (!aEntry)
    {
    return;
    }

  if (!aEntry->GetCategoryObject() || !aEntry->GetTypeObject() || !aEntry->GetTypeModifierObject()
    || !aEntry->GetAnatomicRegionObject() || !aEntry->GetAnatomicRegionModifierObject() )
    {
    vtkErrorMacro("Copy: Invalid terminology entry given");
    // Invalidate whole terminology entry
    this->SetTerminologyContextName(nullptr);
    this->SetAnatomicContextName(nullptr);
    return;
    }

  this->SetTerminologyContextName(aEntry->GetTerminologyContextName());

  this->CategoryObject->Copy(aEntry->GetCategoryObject());
  this->TypeObject->Copy(aEntry->GetTypeObject());
  this->TypeModifierObject->Copy(aEntry->GetTypeModifierObject());

  this->SetAnatomicContextName(aEntry->GetAnatomicContextName());

  this->AnatomicRegionObject->Copy(aEntry->GetAnatomicRegionObject());
  this->AnatomicRegionModifierObject->Copy(aEntry->GetAnatomicRegionModifierObject());
}
