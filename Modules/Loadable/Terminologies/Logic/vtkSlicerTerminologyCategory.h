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

#ifndef __vtkSlicerTerminologyCategory_h
#define __vtkSlicerTerminologyCategory_h

// VTK includes
#include <vtkObject.h>

#include "vtkSlicerTerminologiesModuleLogicExport.h"

/// VTK implementation of \sa qSlicerDICOMExportable
class VTK_SLICER_TERMINOLOGIES_LOGIC_EXPORT vtkSlicerTerminologyCategory : public vtkObject
{
public:
  static vtkSlicerTerminologyCategory *New();
  vtkTypeMacro(vtkSlicerTerminologyCategory, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Reset state of object
  virtual void Initialize();

  /// Copy one category into another
  virtual void Copy(vtkSlicerTerminologyCategory* aCategory);

public:
  vtkGetStringMacro(CodeMeaning);
  vtkSetStringMacro(CodeMeaning);
  vtkGetStringMacro(CodingScheme);
  vtkSetStringMacro(CodingScheme);
  vtkGetStringMacro(SNOMEDCTConceptID);
  vtkSetStringMacro(SNOMEDCTConceptID);
  vtkGetStringMacro(UMLSConceptUID);
  vtkSetStringMacro(UMLSConceptUID);
  vtkGetStringMacro(Cid);
  vtkSetStringMacro(Cid);
  vtkGetStringMacro(CodeValue);
  vtkSetStringMacro(CodeValue);
  vtkGetStringMacro(ContextGroupName);
  vtkSetStringMacro(ContextGroupName);

  vtkGetMacro(ShowAnatomy, bool);
  vtkSetMacro(ShowAnatomy, bool);
  vtkBooleanMacro(ShowAnatomy, bool);

protected:
  vtkSlicerTerminologyCategory();
  ~vtkSlicerTerminologyCategory();
  vtkSlicerTerminologyCategory(const vtkSlicerTerminologyCategory&);
  void operator=(const vtkSlicerTerminologyCategory&);

protected:
  /// 'codeMeaning' member of the category object. Value example "Tissue"
  char* CodeMeaning;
  /// 'codingScheme' member of the category object. Value example "SRT"
  char* CodingScheme;
  /// 'SNOMEDCTConceptID' member of the category object. Value example "85756007"
  char* SNOMEDCTConceptID;
  /// 'UMLSConceptUID' member of the category object. Value example "C0040300"
  char* UMLSConceptUID;
  /// 'cid' member of the category object. Value example "7051"
  char* Cid;
  /// 'codeValue' member of the category object. Value example "T-D0050"
  char* CodeValue;
  /// 'contextGroupName' member of the category object. Value example "Segmentation Property Categories"
  char* ContextGroupName;

  /// 'showAnatomy' member of the category object
  bool ShowAnatomy;
};

#endif
