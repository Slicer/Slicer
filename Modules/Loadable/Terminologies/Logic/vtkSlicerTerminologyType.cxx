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
#include "vtkSlicerTerminologyType.h"

// VTK includes
#include <vtkObjectFactory.h>

//------------------------------------------------------------------------------
int vtkSlicerTerminologyType::INVALID_COLOR[3] = {127, 127, 127};

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTerminologyType);

//----------------------------------------------------------------------------
vtkSlicerTerminologyType::vtkSlicerTerminologyType()
{
  this->RecommendedDisplayRGBValue[0] = INVALID_COLOR[0];
  this->RecommendedDisplayRGBValue[1] = INVALID_COLOR[1];
  this->RecommendedDisplayRGBValue[2] = INVALID_COLOR[2];
  this->SlicerLabel = nullptr;
  this->SNOMEDCTConceptID = nullptr;
  this->UMLSConceptUID = nullptr;
  this->Cid = nullptr;
  this->ContextGroupName = nullptr;
  this->HasModifiers = false;
}

//----------------------------------------------------------------------------
vtkSlicerTerminologyType::~vtkSlicerTerminologyType()
{
  this->Initialize();
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyType::Initialize()
{
  Superclass::Initialize();

  this->SetSlicerLabel(nullptr);
  this->SetSNOMEDCTConceptID(nullptr);
  this->SetUMLSConceptUID(nullptr);
  this->SetCid(nullptr);
  this->SetContextGroupName(nullptr);
  this->RecommendedDisplayRGBValue[0] = INVALID_COLOR[0];
  this->RecommendedDisplayRGBValue[1] = INVALID_COLOR[1];
  this->RecommendedDisplayRGBValue[2] = INVALID_COLOR[2];
  this->HasModifiers = false;
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyType::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "RecommendedDisplayRGBValue:   ("
    << this->RecommendedDisplayRGBValue[0] << ","
    << this->RecommendedDisplayRGBValue[1] << ","
    << this->RecommendedDisplayRGBValue[2] << ")\n";
  os << indent << "SlicerLabel:   " << (this->SlicerLabel?this->SlicerLabel:"NULL") << "\n";
  os << indent << "SNOMEDCTConceptID:   " << (this->SNOMEDCTConceptID?this->SNOMEDCTConceptID:"NULL") << "\n";
  os << indent << "UMLSConceptUID:   " << (this->UMLSConceptUID?this->UMLSConceptUID:"NULL") << "\n";
  os << indent << "Cid:   " << (this->Cid?this->Cid:"NULL") << "\n";
  os << indent << "ContextGroupName:   " << (this->ContextGroupName?this->ContextGroupName:"NULL") << "\n";
  os << indent << "HasModifiers:   " << (this->HasModifiers?"true":"false") << "\n";
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyType::Copy(vtkCodedEntry* aType)
{
  if (!aType)
    {
    return;
    }

  this->Superclass::Copy(aType);

  vtkSlicerTerminologyType *aTerminologyType =
      vtkSlicerTerminologyType::SafeDownCast(aType);

  this->SetRecommendedDisplayRGBValue(aTerminologyType->GetRecommendedDisplayRGBValue());
  this->SetSlicerLabel(aTerminologyType->GetSlicerLabel());
  this->SetSNOMEDCTConceptID(aTerminologyType->GetSNOMEDCTConceptID());
  this->SetUMLSConceptUID(aTerminologyType->GetUMLSConceptUID());
  this->SetCid(aTerminologyType->GetCid());
  this->SetContextGroupName(aTerminologyType->GetContextGroupName());
  this->SetHasModifiers(aTerminologyType->GetHasModifiers());
}
