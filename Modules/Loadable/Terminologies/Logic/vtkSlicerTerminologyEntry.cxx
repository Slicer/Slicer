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
  vtkNew<vtkSlicerTerminologyCategory> category;
  this->SetCategoryObject(category);

  vtkNew<vtkSlicerTerminologyType> type;
  this->SetTypeObject(type);

  vtkNew<vtkSlicerTerminologyType> typeModifier;
  this->SetTypeModifierObject(typeModifier);

  vtkNew<vtkSlicerTerminologyType> region;
  this->SetRegionObject(region);

  vtkNew<vtkSlicerTerminologyType> regionModifier;
  this->SetRegionModifierObject(regionModifier);
}

//----------------------------------------------------------------------------
vtkSlicerTerminologyEntry::~vtkSlicerTerminologyEntry()
{
  this->SetTerminologyContextName(nullptr);

  this->SetCategoryObject(nullptr);
  this->SetTypeObject(nullptr);
  this->SetTypeModifierObject(nullptr);

  this->SetRegionContextName(nullptr);

  this->SetRegionObject(nullptr);
  this->SetRegionModifierObject(nullptr);
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyEntry::Initialize()
{
  this->SetTerminologyContextName(nullptr);

  this->CategoryObject->Initialize();
  this->TypeObject->Initialize();
  this->TypeModifierObject->Initialize();

  this->SetRegionContextName(nullptr);

  this->RegionObject->Initialize();
  this->RegionModifierObject->Initialize();
}

//----------------------------------------------------------------------------
bool vtkSlicerTerminologyEntry::IsValid()
{
  if (this->CategoryObject == nullptr || this->CategoryObject->GetCodeMeaning() == nullptr ||
      this->CategoryObject->GetCodingSchemeDesignator() == nullptr || this->CategoryObject->GetCodeValue() == nullptr)
  {
    return false;
  }
  if (this->TypeObject == nullptr || this->TypeObject->GetCodeMeaning() == nullptr ||
      this->TypeObject->GetCodingSchemeDesignator() == nullptr || this->TypeObject->GetCodeValue() == nullptr)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerTerminologyEntry::IsEmpty()
{
  if (this->TerminologyContextName && strlen(this->TerminologyContextName) > 0)
  {
    return false;
  }
  if (!this->CategoryObject->IsEmpty())
  {
    return false;
  }
  if (!this->TypeObject->IsEmpty())
  {
    return false;
  }
  if (!this->TypeModifierObject->IsEmpty())
  {
    return false;
  }
  if (this->RegionContextName && strlen(this->RegionContextName) > 0)
  {
    return false;
  }
  if (!this->RegionObject->IsEmpty())
  {
    return false;
  }
  if (!this->RegionModifierObject->IsEmpty())
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyEntry::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "TerminologyContextName:   " << (this->TerminologyContextName ? this->TerminologyContextName : "NULL")
     << "\n";

  os << indent << "CategoryObject: ";
  if (this->CategoryObject)
  {
    this->CategoryObject->PrintSelf(os << "\n", indent.GetNextIndent());
  }
  else
  {
    os << indent.GetNextIndent() << "NULL\n";
  }
  os << indent << "TypeObject: ";
  if (this->TypeObject)
  {
    this->TypeObject->PrintSelf(os << "\n", indent.GetNextIndent());
  }
  else
  {
    os << indent.GetNextIndent() << "NULL\n";
  }
  os << indent << "TypeModifierObject: ";
  if (this->TypeModifierObject)
  {
    this->TypeModifierObject->PrintSelf(os << "\n", indent.GetNextIndent());
  }
  else
  {
    os << indent.GetNextIndent() << "NULL\n";
  }

  os << indent << "RegionContextName:   " << (this->RegionContextName ? this->RegionContextName : "NULL") << "\n";
  os << indent << "RegionObject: ";
  if (this->RegionObject)
  {
    this->RegionObject->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent.GetNextIndent() << "NULL\n";
  }
  os << indent << "RegionModifierObject: ";
  if (this->RegionModifierObject)
  {
    this->RegionModifierObject->PrintSelf(os, indent.GetNextIndent());
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

  if (!aEntry->GetCategoryObject() || !aEntry->GetTypeObject() || !aEntry->GetTypeModifierObject() ||
      !aEntry->GetRegionObject() || !aEntry->GetRegionModifierObject())
  {
    vtkErrorMacro("Copy: Invalid terminology entry given");
    // Invalidate whole terminology entry
    this->SetTerminologyContextName(nullptr);
    this->SetRegionContextName(nullptr);
    return;
  }

  this->SetTerminologyContextName(aEntry->GetTerminologyContextName());

  this->CategoryObject->Copy(aEntry->GetCategoryObject());
  this->TypeObject->Copy(aEntry->GetTypeObject());
  this->TypeModifierObject->Copy(aEntry->GetTypeModifierObject());

  this->SetRegionContextName(aEntry->GetRegionContextName());

  this->RegionObject->Copy(aEntry->GetRegionObject());
  this->RegionModifierObject->Copy(aEntry->GetRegionModifierObject());
}

//----------------------------------------------------------------------------
const char* vtkSlicerTerminologyEntry::GetAnatomicContextName()
{
  vtkWarningMacro("GetAnatomicContextName is deprecated. Use GetRegionContextName instead.");
  return this->GetRegionContextName();
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyEntry::SetAnatomicContextName(const char* name)
{
  vtkWarningMacro("SetAnatomicContextName is deprecated. Use SetRegionContextName instead.");
  this->SetRegionContextName(name);
}

//----------------------------------------------------------------------------
vtkSlicerTerminologyType* vtkSlicerTerminologyEntry::GetAnatomicRegionObject()
{
  vtkWarningMacro("GetAnatomicRegionObject is deprecated. Use GetRegionObject instead.");
  return this->GetRegionObject();
}

//----------------------------------------------------------------------------
vtkSlicerTerminologyType* vtkSlicerTerminologyEntry::GetAnatomicRegionModifierObject()
{
  vtkWarningMacro("GetAnatomicRegionModifierObject is deprecated. Use GetRegionObject instead.");
  return this->GetRegionModifierObject();
}
