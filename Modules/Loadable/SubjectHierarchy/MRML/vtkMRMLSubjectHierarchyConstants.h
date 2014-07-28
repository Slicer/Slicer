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

#include "vtkSlicerSubjectHierarchyModuleMRMLExport.h"

// STD includes
#include <cstdlib>
#include <string>

class VTK_SLICER_SUBJECTHIERARCHY_MODULE_MRML_EXPORT vtkMRMLSubjectHierarchyConstants
{
public:
  //----------------------------------------------------------------------------
  // Constant strings (std::string types for easy concatenation)
  //----------------------------------------------------------------------------

  // Subject hierarchy constants
  static const std::string SUBJECTHIERARCHY_NODE_NAME_POSTFIX;
  static const std::string SUBJECTHIERARCHY_ATTRIBUTE_PREFIX;
  static const std::string SUBJECTHIERARCHY_EXCLUDE_FROM_POTENTIAL_NODES_LIST_ATTRIBUTE_NAME;
  static const std::string SUBJECTHIERARCHY_NEW_NODE_NAME_PREFIX;

  static const char* SUBJECTHIERARCHY_LEVEL_SUBJECT;
  static const char* SUBJECTHIERARCHY_LEVEL_STUDY;

  // DICOM plugin constants
  static const char* DICOMHIERARCHY_LEVEL_SERIES;
  static const char* DICOMHIERARCHY_LEVEL_SUBSERIES;

  static const std::string DICOMHIERARCHY_ATTRIBUTE_PREFIX;
  static const std::string DICOMHIERARCHY_PATIENT_NAME_ATTRIBUTE_NAME;
  static const std::string DICOMHIERARCHY_PATIENT_ID_ATTRIBUTE_NAME;
  static const std::string DICOMHIERARCHY_PATIENT_SEX_ATTRIBUTE_NAME;
  static const std::string DICOMHIERARCHY_PATIENT_BIRTH_DATE_ATTRIBUTE_NAME;
  static const std::string DICOMHIERARCHY_STUDY_DATE_ATTRIBUTE_NAME;
  static const std::string DICOMHIERARCHY_STUDY_TIME_ATTRIBUTE_NAME;
  static const std::string DICOMHIERARCHY_SERIES_MODALITY_ATTRIBUTE_NAME;
  static const char* DICOMHIERARCHY_DICOM_UID_NAME;

};

#endif
