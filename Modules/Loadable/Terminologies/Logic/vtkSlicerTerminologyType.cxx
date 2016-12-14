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
vtkStandardNewMacro(vtkSlicerTerminologyType);

//----------------------------------------------------------------------------
vtkSlicerTerminologyType::vtkSlicerTerminologyType()
{
  this->RecommendedDisplayRGBValue[0] = this->RecommendedDisplayRGBValue[1] = this->RecommendedDisplayRGBValue[2] = 127;
  this->CodeMeaning = NULL;
  this->CodingScheme = NULL;
  this->SlicerLabel = NULL;
  this->SNOMEDCTConceptID = NULL;
  this->UMLSConceptUID = NULL;
  this->Cid = NULL;
  this->CodeValue = NULL;
  this->ContextGroupName = NULL;
  this->HasModifiers = false;
}

//----------------------------------------------------------------------------
vtkSlicerTerminologyType::~vtkSlicerTerminologyType()
{
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyType::Initialize()
{
  this->RecommendedDisplayRGBValue[0] = this->RecommendedDisplayRGBValue[1] = this->RecommendedDisplayRGBValue[2] = 127;
  this->CodeMeaning = NULL;
  this->CodingScheme = NULL;
  this->SlicerLabel = NULL;
  this->SNOMEDCTConceptID = NULL;
  this->UMLSConceptUID = NULL;
  this->Cid = NULL;
  this->CodeValue = NULL;
  this->ContextGroupName = NULL;
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
  os << indent << "CodeMeaning:   " << (this->CodeMeaning?this->CodeMeaning:"NULL") << "\n";
  os << indent << "CodingScheme:   " << (this->CodingScheme?this->CodingScheme:"NULL") << "\n";
  os << indent << "SlicerLabel:   " << (this->SlicerLabel?this->SlicerLabel:"NULL") << "\n";
  os << indent << "SNOMEDCTConceptID:   " << (this->SNOMEDCTConceptID?this->SNOMEDCTConceptID:"NULL") << "\n";
  os << indent << "UMLSConceptUID:   " << (this->UMLSConceptUID?this->UMLSConceptUID:"NULL") << "\n";
  os << indent << "Cid:   " << (this->Cid?this->Cid:"NULL") << "\n";
  os << indent << "CodeValue:   " << (this->CodeValue?this->CodeValue:"NULL") << "\n";
  os << indent << "ContextGroupName:   " << (this->ContextGroupName?this->ContextGroupName:"NULL") << "\n";
  os << indent << "HasModifiers:   " << (this->HasModifiers?"true":"false") << "\n";
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologyType::Copy(vtkSlicerTerminologyType* aType)
{
  if (!aType)
    {
    return;
    }

  this->SetRecommendedDisplayRGBValue(aType->GetRecommendedDisplayRGBValue());
  this->SetCodeMeaning(aType->GetCodeMeaning());
  this->SetCodingScheme(aType->GetCodingScheme());
  this->SetSlicerLabel(aType->GetSlicerLabel());
  this->SetSNOMEDCTConceptID(aType->GetSNOMEDCTConceptID());
  this->SetUMLSConceptUID(aType->GetUMLSConceptUID());
  this->SetCid(aType->GetCid());
  this->SetCodeValue(aType->GetCodeValue());
  this->SetContextGroupName(aType->GetContextGroupName());
  this->SetHasModifiers(aType->GetHasModifiers());
}
