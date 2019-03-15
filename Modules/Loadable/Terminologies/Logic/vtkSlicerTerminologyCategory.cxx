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
#include "vtkSlicerTerminologyCategory.h"

// VTK includes
#include <vtkObjectFactory.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTerminologyCategory);

//----------------------------------------------------------------------------
vtkSlicerTerminologyCategory::vtkSlicerTerminologyCategory()
{
  this->SNOMEDCTConceptID = nullptr;
  this->UMLSConceptUID = nullptr;
  this->Cid = nullptr;
  this->ContextGroupName = nullptr;
  this->ShowAnatomy = false;
}

//----------------------------------------------------------------------------
vtkSlicerTerminologyCategory::~vtkSlicerTerminologyCategory()
{
  this->Initialize();
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyCategory::Initialize()
{
  Superclass::Initialize();

  this->SetSNOMEDCTConceptID(nullptr);
  this->SetUMLSConceptUID(nullptr);
  this->SetCid(nullptr);
  this->SetContextGroupName(nullptr);
  this->ShowAnatomy = false;
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyCategory::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "SNOMEDCTConceptID:   " << (this->SNOMEDCTConceptID?this->SNOMEDCTConceptID:"NULL") << "\n";
  os << indent << "UMLSConceptUID:   " << (this->UMLSConceptUID?this->UMLSConceptUID:"NULL") << "\n";
  os << indent << "Cid:   " << (this->Cid?this->Cid:"NULL") << "\n";
  os << indent << "ContextGroupName:   " << (this->ContextGroupName?this->ContextGroupName:"NULL") << "\n";
  os << indent << "ShowAnatomy:   " << (this->ShowAnatomy?"true":"false") << "\n";
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyCategory::Copy(vtkCodedEntry* aCategory)
{
  if (!aCategory)
    {
    return;
    }

  this->Superclass::Copy(aCategory);

  vtkSlicerTerminologyCategory *aTerminologyCategory =
      vtkSlicerTerminologyCategory::SafeDownCast(aCategory);

  this->SetSNOMEDCTConceptID(aTerminologyCategory->GetSNOMEDCTConceptID());
  this->SetUMLSConceptUID(aTerminologyCategory->GetUMLSConceptUID());
  this->SetCid(aTerminologyCategory->GetCid());
  this->SetContextGroupName(aTerminologyCategory->GetContextGroupName());
  this->SetShowAnatomy(aTerminologyCategory->GetShowAnatomy());
}
