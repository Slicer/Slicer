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

#ifndef __vtkMRMLSubjectHierarchyConstants_h
#define __vtkMRMLSubjectHierarchyConstants_h

// MRML includes
#include "vtkMRML.h"

// STD includes
#include <cstdlib>
#include <string>
#include <vector>

class VTK_MRML_EXPORT vtkMRMLSubjectHierarchyConstants
{
public:
  //----------------------------------------------------------------------------
  // Constant strings (std::string types for easy concatenation)
  //----------------------------------------------------------------------------

  // Data node subject hierarchy attribute
  static const std::string GetSubjectHierarchyAttributePrefix()
    { return "SubjectHierarchy."; };
  static const std::string GetSubjectHierarchyExcludeFromTreeAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyAttributePrefix() + "ExcludeFromPotentialNodesList"; };

  // Subject hierarchy item attributes
  static const std::string GetSubjectHierarchyVirtualBranchAttributeName()
    { return "VirtualBranch"; };
  static const std::string GetSubjectHierarchyLevelAttributeName()
    { return "Level"; };
  static const std::string GetSubjectHierarchyNewItemNamePrefix()
    { return "New"; };

  // Non-DICOM levels
  static const char* GetSubjectHierarchyLevelFolder()
    { return "Folder"; };
  // DICOM levels
  static const std::string GetDICOMLevelPatient()
    { return "Patient"; };
  static const std::string GetDICOMLevelStudy()
    { return "Study"; };

  // DICOM attributes
  static const char* GetDICOMUIDName()
    { return "DICOM"; };
  static const char* GetDICOMInstanceUIDName()
    { return "DICOMInstanceUID"; };
  static const std::string GetDICOMAttributePrefix()
    { return "DICOM."; };
  static std::string GetDICOMReferencedInstanceUIDsAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + "ReferencedInstanceUIDs"; };

  // Patient tags
  static const std::string GetDICOMPatientNameTagName()
    { return "PatientName"; };
  static const std::string GetDICOMPatientNameAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMPatientNameTagName(); };
  static const std::string GetDICOMPatientIDTagName()
    { return "PatientID"; };
  static const std::string GetDICOMPatientIDAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMPatientIDTagName(); };
  static const std::string GetDICOMPatientSexTagName()
    { return "PatientSex"; };
  static const std::string GetDICOMPatientSexAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMPatientSexTagName(); };
  static const std::string GetDICOMPatientBirthDateTagName()
    { return "PatientBirthDate"; };
  static const std::string GetDICOMPatientBirthDateAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMPatientBirthDateTagName(); };
  static const std::string GetDICOMPatientCommentsTagName()
    { return "PatientComments"; };
  static const std::string GetDICOMPatientCommentsAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMPatientCommentsTagName(); };

  /// Get patient tag names (attribute names are these values prefixed!)
  static const std::vector<std::string> GetDICOMPatientTagNames()
  {
    std::vector<std::string> patientTagNames;
    patientTagNames.push_back(vtkMRMLSubjectHierarchyConstants::GetDICOMPatientNameTagName());
    patientTagNames.push_back(vtkMRMLSubjectHierarchyConstants::GetDICOMPatientIDTagName());
    patientTagNames.push_back(vtkMRMLSubjectHierarchyConstants::GetDICOMPatientSexTagName());
    patientTagNames.push_back(vtkMRMLSubjectHierarchyConstants::GetDICOMPatientBirthDateTagName());
    patientTagNames.push_back(vtkMRMLSubjectHierarchyConstants::GetDICOMPatientCommentsTagName());
    return patientTagNames;
  }

  // Study tags
  static const std::string  GetDICOMStudyInstanceUIDTagName()
    { return "StudyInstanceUID"; };
  static const std::string  GetDICOMStudyIDTagName()
    { return "StudyID"; };
  static const std::string  GetDICOMStudyDescriptionTagName()
    { return "StudyDescription"; };
  static const std::string  GetDICOMStudyDescriptionAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMStudyDescriptionTagName(); };
  static const std::string GetDICOMStudyDateTagName()
    { return "StudyDate"; };
  static const std::string GetDICOMStudyDateAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMStudyDateTagName(); };
  static const std::string GetDICOMStudyTimeTagName()
    { return "StudyTime"; };
  static const std::string GetDICOMStudyTimeAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + vtkMRMLSubjectHierarchyConstants::GetDICOMStudyTimeTagName(); };

  /// Get study tag names (attribute names are these values prefixed!)
  static const std::vector<std::string> GetDICOMStudyTagNames()
  {
    std::vector<std::string> studyTagNames;
    studyTagNames.push_back(vtkMRMLSubjectHierarchyConstants::GetDICOMStudyDescriptionTagName());
    studyTagNames.push_back(vtkMRMLSubjectHierarchyConstants::GetDICOMStudyDateTagName());
    studyTagNames.push_back(vtkMRMLSubjectHierarchyConstants::GetDICOMStudyTimeTagName());
    return studyTagNames;
  }

  // Series tags
  static const std::string GetDICOMSeriesModalityAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + "Modality"; };
  static const std::string GetDICOMSeriesNumberAttributeName()
    { return vtkMRMLSubjectHierarchyConstants::GetDICOMAttributePrefix() + "SeriesNumber"; };

};

#endif
