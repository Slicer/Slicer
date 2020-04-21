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
#include "vtkSlicerDICOMExportable.h"

// MRML includes
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkObjectFactory.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerDICOMExportable);

//----------------------------------------------------------------------------
vtkSlicerDICOMExportable::vtkSlicerDICOMExportable()
{
  this->Name = nullptr;
  this->SetName("Unknown exporter");
  this->Tooltip = nullptr;
  this->SetTooltip("Creates a DICOM file from the selected data");
  this->SubjectHierarchyItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  this->PluginClass = nullptr;
  this->Directory = nullptr;
  this->Confidence = 0.0;
  this->Tags.clear();
}

//----------------------------------------------------------------------------
vtkSlicerDICOMExportable::~vtkSlicerDICOMExportable() = default;

//----------------------------------------------------------------------------
void vtkSlicerDICOMExportable::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Name:   " << (this->Name?this->Name:"NULL") << "\n";
  os << indent << "Tooltip:   " << (this->Tooltip?this->Tooltip:"NULL") << "\n";
  os << indent << "SubjectHierarchyItemID:   " << this->SubjectHierarchyItemID << "\n";
  os << indent << "PluginClass:   " << (this->PluginClass?this->PluginClass:"NULL") << "\n";
  os << indent << "Directory:   " << (this->Directory?this->Directory:"NULL") << "\n";
  os << indent << "Confidence:   " << this->Confidence << "\n";
  os << indent << "Tags:   " << "\n";
  for ( std::map<std::string, std::string>::iterator it=this->Tags.begin();
    it != this->Tags.end(); ++it )
    {
    os << indent << "  " << it->first << ", " << it->second << "\n";
    }
}

//----------------------------------------------------------------------------
const char* vtkSlicerDICOMExportable::GetTag(std::string tagName)
{
  return this->Tags[tagName].c_str();
}

//----------------------------------------------------------------------------
void vtkSlicerDICOMExportable::SetTag(std::string tagName, std::string tagValue)
{
  this->Tags[tagName] = tagValue;
}
