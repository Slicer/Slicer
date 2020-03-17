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

#ifndef __vtkSlicerTerminologyType_h
#define __vtkSlicerTerminologyType_h

// MRML includes
#include "vtkCodedEntry.h"

#include "vtkSlicerTerminologiesModuleLogicExport.h"

/// \brief Terminology property type object
///
/// Encapsulates the mandatory and optional fields for a terminology type.
/// The following fields inherited from \sa vtkCodedEntry:
///   CodingSchemeDesignator: 'codingScheme' member of the type object. Value example "SCT"
///   CodeValue: 'codeValue' member of the type object. Value example "51114001"
///   CodeMeaning: 'codeMeaning' member of the type object. Value example "Artery"
///

class VTK_SLICER_TERMINOLOGIES_LOGIC_EXPORT vtkSlicerTerminologyType : public vtkCodedEntry
{
public:
  static int INVALID_COLOR[3];

public:
  static vtkSlicerTerminologyType *New();
  vtkTypeMacro(vtkSlicerTerminologyType, vtkCodedEntry);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Reset state of object
  void Initialize() override;

  /// Copy one type into another
  void Copy(vtkCodedEntry* aType) override;

public:
  vtkGetVector3Macro(RecommendedDisplayRGBValue, unsigned char);
  vtkSetVector3Macro(RecommendedDisplayRGBValue, unsigned char);
  vtkGetStringMacro(SlicerLabel);
  vtkSetStringMacro(SlicerLabel);
  vtkGetStringMacro(SNOMEDCTConceptID);
  vtkSetStringMacro(SNOMEDCTConceptID);
  vtkGetStringMacro(UMLSConceptUID);
  vtkSetStringMacro(UMLSConceptUID);
  vtkGetStringMacro(Cid);
  vtkSetStringMacro(Cid);
  vtkGetStringMacro(ContextGroupName);
  vtkSetStringMacro(ContextGroupName);

  vtkGetMacro(HasModifiers, bool);
  vtkSetMacro(HasModifiers, bool);
  vtkBooleanMacro(HasModifiers, bool);

protected:
  vtkSlicerTerminologyType();
  ~vtkSlicerTerminologyType() override;
  vtkSlicerTerminologyType(const vtkSlicerTerminologyType&);
  void operator=(const vtkSlicerTerminologyType&);

protected:
  /// 'recommendedDisplayRGBValue' member of the type object
  unsigned char RecommendedDisplayRGBValue[3];
  /// '3dSlicerLabel' member of the type object. Value example "artery"
  char* SlicerLabel;
  /// 'SNOMEDCTConceptID' member of the type object. Value example "275989006"
  char* SNOMEDCTConceptID;
  /// 'UMLSConceptUID' member of the type object. Value example "C0555806"
  char* UMLSConceptUID;
  /// 'cid' member of the type object. Value example "7166"
  char* Cid;
  /// 'contextGroupName' member of the type object. Value example "Common Tissue Segmentation Types"
  char* ContextGroupName;

  /// Flag indicating whether the type object has a 'Modifier' member array. False by default.
  /// A Type object EITHER has 'recommendedDisplayRGBValue' and '3dSlicerLabel' OR a 'Modifier' member
  /// array with the modifiers, so if this flag is true, then there is no valid color and Slicer label.
  /// Anatomic region objects can have neither.
  bool HasModifiers;
};

#endif
