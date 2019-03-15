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
#include "vtkSlicerDICOMLoadable.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerDICOMLoadable);

//----------------------------------------------------------------------------
vtkSlicerDICOMLoadable::vtkSlicerDICOMLoadable()
{
  this->Name = nullptr;
  this->SetName("Unknown");
  this->Tooltip = nullptr;
  this->SetTooltip("No further information available");
  this->Warning = nullptr;
  this->Selected = false;
  this->Confidence = 0.5;

  this->Files = nullptr;
  vtkSmartPointer<vtkStringArray> files = vtkSmartPointer<vtkStringArray>::New();
  this->SetFiles(files);

  this->ReferencedInstanceUIDs = nullptr;
  vtkSmartPointer<vtkStringArray> referencedInstanceUIDs = vtkSmartPointer<vtkStringArray>::New();
  this->SetReferencedInstanceUIDs(referencedInstanceUIDs);
}

//----------------------------------------------------------------------------
vtkSlicerDICOMLoadable::~vtkSlicerDICOMLoadable()
{
  this->SetFiles(nullptr);
  this->SetReferencedInstanceUIDs(nullptr);
}

//----------------------------------------------------------------------------
void vtkSlicerDICOMLoadable::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Name:   " << (this->Name?this->Name:"NULL") << "\n";
  os << indent << "Tooltip:   " << (this->Tooltip?this->Tooltip:"NULL") << "\n";
  os << indent << "Warning:   " << (this->Warning?this->Warning:"NULL") << "\n";
  os << indent << "Files:   " << (this->Files?"":"NULL") << "\n";
  if (this->Files)
    {
    for (int fileIndex=0; fileIndex<this->Files->GetNumberOfValues(); ++fileIndex)
      {
      os << indent << "  " << this->Files->GetValue(fileIndex) << "\n";
      }
    }
  os << indent << "Selected:   " << (this->Selected?"true":"false") << "\n";
  os << indent << "Confidence:   " << this->Confidence << "\n";
  os << indent << "ReferencedInstanceUIDs:   " << (this->ReferencedInstanceUIDs?"":"NULL") << "\n";
  if (this->ReferencedInstanceUIDs)
    {
    for (int fileIndex=0; fileIndex<this->ReferencedInstanceUIDs->GetNumberOfValues(); ++fileIndex)
      {
      os << indent << "  " << this->ReferencedInstanceUIDs->GetValue(fileIndex) << "\n";
      }
    }
}

//----------------------------------------------------------------------------
void vtkSlicerDICOMLoadable::AddFile(const char* file)
{
  this->Files->InsertNextValue(file);
}

//----------------------------------------------------------------------------
void vtkSlicerDICOMLoadable::AddReferencedInstanceUID(const char* referencedInstanceUID)
{
  this->ReferencedInstanceUIDs->InsertNextValue(referencedInstanceUID);
}
