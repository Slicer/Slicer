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

#ifndef __vtkSlicerDICOMExportable_h
#define __vtkSlicerDICOMExportable_h

// VTK includes
#include <vtkObject.h>
#include <vtkStringArray.h>

// STD includes
#include <map>
#include <string>

#include "vtkSlicerDICOMLibModuleLogicExport.h"

/// VTK implementation of \sa qSlicerDICOMExportable
class VTK_SLICER_DICOMLIB_MODULE_LOGIC_EXPORT vtkSlicerDICOMExportable : public vtkObject
{
public:
  static vtkSlicerDICOMExportable *New();
  vtkTypeMacro(vtkSlicerDICOMExportable, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

public:
  vtkGetStringMacro(Name);
  vtkSetStringMacro(Name);

  vtkGetStringMacro(Tooltip);
  vtkSetStringMacro(Tooltip);

  vtkGetMacro(SubjectHierarchyItemID, vtkIdType);
  vtkSetMacro(SubjectHierarchyItemID, vtkIdType);

  vtkGetStringMacro(PluginClass);
  vtkSetStringMacro(PluginClass);

  vtkGetStringMacro(Directory);
  vtkSetStringMacro(Directory);

  vtkGetMacro(Confidence, double);
  vtkSetMacro(Confidence, double);

  std::map<std::string, std::string> GetTags() { return this->Tags; };
  void SetTags(std::map<std::string, std::string> tags) { this->Tags = tags; };
  const char* GetTag(std::string tagName);
  void SetTag(std::string tagName, std::string tagValue);

protected:
  vtkSlicerDICOMExportable();
  ~vtkSlicerDICOMExportable() override;
  vtkSlicerDICOMExportable(const vtkSlicerDICOMExportable&);
  void operator=(const vtkSlicerDICOMExportable&);

protected:
  /// Name exposed to the user for the export method
  char* Name;

  /// Extra information the user sees on mouse over of the export option
  char* Tooltip;

  /// ID of the subject hierarchy item to be exported
  vtkIdType SubjectHierarchyItemID;

  /// Class of the plugin that created this exportable
  char* PluginClass;

  /// Target directory to export this exportable
  char* Directory;

  /// Confidence - from 0 to 1 where 0 means that the plugin
  /// cannot export the given node, up to 1 that means that the
  /// plugin considers itself the best plugin to export the node
  /// (in case of specialized objects, e.g. RT dose volume)
  double Confidence;

  /// Pseudo-tags offered by the plugin that are to be filled out for export.
  /// The pseudo-tags are translated into real DICOM tags at the time of export.
  /// It tag is a pair of strings (name, value). When the exportable is created
  /// by the DICOM plugin, value is the default value that is set in the editor widget
  std::map<std::string, std::string> Tags;
};

#endif
