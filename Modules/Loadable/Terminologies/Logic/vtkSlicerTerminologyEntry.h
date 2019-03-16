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

#ifndef __vtkSlicerTerminologyEntry_h
#define __vtkSlicerTerminologyEntry_h

// VTK includes
#include <vtkObject.h>

// Terminology includes
#include "vtkSlicerTerminologiesModuleLogicExport.h"

#include "vtkSlicerTerminologyCategory.h"
#include "vtkSlicerTerminologyType.h"

/// VTK implementation of \sa qSlicerDICOMLoadable
class VTK_SLICER_TERMINOLOGIES_LOGIC_EXPORT vtkSlicerTerminologyEntry : public vtkObject
{
public:
  static vtkSlicerTerminologyEntry *New();
  vtkTypeMacro(vtkSlicerTerminologyEntry, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Copy one entry into another
  virtual void Copy(vtkSlicerTerminologyEntry* aEntry);

public:
  vtkGetStringMacro(TerminologyContextName);
  vtkSetStringMacro(TerminologyContextName);

  vtkGetObjectMacro(CategoryObject, vtkSlicerTerminologyCategory);
  vtkGetObjectMacro(TypeObject, vtkSlicerTerminologyType);
  vtkGetObjectMacro(TypeModifierObject, vtkSlicerTerminologyType);

  vtkGetStringMacro(AnatomicContextName);
  vtkSetStringMacro(AnatomicContextName);

  vtkGetObjectMacro(AnatomicRegionObject, vtkSlicerTerminologyType);
  vtkGetObjectMacro(AnatomicRegionModifierObject, vtkSlicerTerminologyType);

protected:
  vtkSetObjectMacro(CategoryObject, vtkSlicerTerminologyCategory);
  vtkSetObjectMacro(TypeObject, vtkSlicerTerminologyType);
  vtkSetObjectMacro(TypeModifierObject, vtkSlicerTerminologyType);

  vtkSetObjectMacro(AnatomicRegionObject, vtkSlicerTerminologyType);
  vtkSetObjectMacro(AnatomicRegionModifierObject, vtkSlicerTerminologyType);

protected:
  vtkSlicerTerminologyEntry();
  ~vtkSlicerTerminologyEntry() override;
  vtkSlicerTerminologyEntry(const vtkSlicerTerminologyEntry&);
  void operator=(const vtkSlicerTerminologyEntry&);

protected:
  /// Terminology context name (SegmentationCategoryTypeContextName in terminology Json)
  char* TerminologyContextName;
  /// Category properties
  vtkSlicerTerminologyCategory* CategoryObject;
  /// Type properties
  vtkSlicerTerminologyType* TypeObject;
  /// Type modifier properties
  vtkSlicerTerminologyType* TypeModifierObject;

  /// Anatomic context name (AnatomicContextName in anatomy Json) - optional
  char* AnatomicContextName;
  /// Anatomical region properties
  vtkSlicerTerminologyType* AnatomicRegionObject;
  /// Anatomical region modifier properties
  vtkSlicerTerminologyType* AnatomicRegionModifierObject;
};

#endif
