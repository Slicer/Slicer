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

#include "vtkMRMLSubjectHierarchyConstants.h"

//----------------------------------------------------------------------------
// Constant strings
//----------------------------------------------------------------------------

// Subject hierarchy constants
const std::string vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_NODE_NAME_POSTFIX = "_SubjectHierarchy";
const std::string vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_ATTRIBUTE_PREFIX = "SubjectHierarchy.";
const std::string vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_EXCLUDE_FROM_TREE_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_ATTRIBUTE_PREFIX + "ExcludeFromTree"; // Identifier
const std::string vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_EXCLUDE_FROM_POTENTIAL_NODES_LIST_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_EXCLUDE_FROM_TREE_ATTRIBUTE_NAME; //TODO: Deprecated member, kept for backwards compatibility
const std::string vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_NEW_NODE_NAME_PREFIX = "New";

const char* vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT = "Subject";
const char* vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER = "GenericFolder";

// DICOM plugin constants
const char* vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_PATIENT = "Patient";
const char* vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_STUDY = "Study";
const char* vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SERIES = "Series";
const char* vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_LEVEL_SUBSERIES = "Subseries";

const std::string vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_ATTRIBUTE_PREFIX = "DICOMHierarchy.";
const std::string vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_PATIENT_NAME_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_ATTRIBUTE_PREFIX + "PatientName";
const std::string vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_PATIENT_ID_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_ATTRIBUTE_PREFIX + "PatientId";
const std::string vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_PATIENT_SEX_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_ATTRIBUTE_PREFIX + "PatientSex";
const std::string vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_PATIENT_BIRTH_DATE_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_ATTRIBUTE_PREFIX + "PatientBirthDate";
const std::string vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_STUDY_DATE_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_ATTRIBUTE_PREFIX + "StudyDate";
const std::string vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_STUDY_TIME_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_ATTRIBUTE_PREFIX + "StudyTime";
const std::string vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_SERIES_MODALITY_ATTRIBUTE_NAME = vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_ATTRIBUTE_PREFIX + "SeriesModality";
const char* vtkMRMLSubjectHierarchyConstants::DICOMHIERARCHY_DICOM_UID_NAME = "DICOM";
