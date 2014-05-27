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
