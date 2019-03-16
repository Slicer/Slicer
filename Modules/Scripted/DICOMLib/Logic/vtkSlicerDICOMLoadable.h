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

#ifndef __vtkSlicerDICOMLoadable_h
#define __vtkSlicerDICOMLoadable_h

// VTK includes
#include <vtkObject.h>
#include <vtkStringArray.h>

#include "vtkSlicerDICOMLibModuleLogicExport.h"

/// VTK implementation of \sa qSlicerDICOMLoadable
class VTK_SLICER_DICOMLIB_MODULE_LOGIC_EXPORT vtkSlicerDICOMLoadable : public vtkObject
{
public:
  static vtkSlicerDICOMLoadable *New();
  vtkTypeMacro(vtkSlicerDICOMLoadable, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

public:
  vtkGetStringMacro(Name);
  vtkSetStringMacro(Name);

  vtkGetStringMacro(Tooltip);
  vtkSetStringMacro(Tooltip);

  vtkGetStringMacro(Warning);
  vtkSetStringMacro(Warning);

  vtkGetObjectMacro(Files, vtkStringArray);

  vtkGetMacro(Selected, bool);
  vtkSetMacro(Selected, bool);
  vtkBooleanMacro(Selected, bool);

  vtkGetMacro(Confidence, double);
  vtkSetMacro(Confidence, double);

  vtkGetObjectMacro(ReferencedInstanceUIDs, vtkStringArray);

  /// Add file to file list \sa Files
  void AddFile(const char* file);

  /// Add referenced instance UID to the UIDs list \sa ReferencedInstanceUIDs
  void AddReferencedInstanceUID(const char* referencedInstanceUID);

protected:
  vtkSetObjectMacro(Files, vtkStringArray);
  vtkSetObjectMacro(ReferencedInstanceUIDs, vtkStringArray);

protected:
  vtkSlicerDICOMLoadable();
  ~vtkSlicerDICOMLoadable() override;
  vtkSlicerDICOMLoadable(const vtkSlicerDICOMLoadable&);
  void operator=(const vtkSlicerDICOMLoadable&);

protected:
  /// Name exposed to the user for the node
  char* Name;

  /// Extra information the user sees on mouse over of the thing
  char* Tooltip;

  /// Things the user should know before loading this data
  char* Warning;

  /// The file list of the data to be loaded
  vtkStringArray* Files;

  /// Is the object checked for loading by default
  bool Selected;

  /// Confidence - from 0 to 1 where 0 means low chance
  /// that the user actually wants to load their data this
  /// way up to 1, which means that the plugin is very confident
  /// that this is the best way to load the data.
  /// When more than one plugin marks the same series as
  /// selected, the one with the highest confidence is
  /// actually selected by default.  In the case of a tie,
  /// both series are selected for loading.
  double Confidence;

  /// List of UIDs for the DICOM instances that are referenced by this loadable
  vtkStringArray* ReferencedInstanceUIDs;
};

#endif
