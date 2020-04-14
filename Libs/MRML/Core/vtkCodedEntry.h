/*=auto=========================================================================

Portions (c) Copyright 2017 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkCodedEntry_h
#define __vtkCodedEntry_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>

/// \brief Simple class for storing standard coded entries (coding scheme, value, meaning triplets)
///
/// vtkCodedEntry is a lightweight class that stores standard coded entries consisting of
/// CodingSchemeDesignator + CodeValue + CodeMeaning triplets.
/// This is a commonly used concept in DICOM, see chapter 3: Encoding of Coded Entry Data
/// (http://dicom.nema.org/medical/dicom/current/output/chtml/part03/sect_8.3.html).
///
class VTK_MRML_EXPORT vtkCodedEntry : public vtkObject
{
public:

  static vtkCodedEntry *New();
  vtkTypeMacro(vtkCodedEntry, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Reset state of object
  virtual void Initialize();

  /// Copy one type into another
  virtual void Copy(vtkCodedEntry* aEntry);

  ///
  /// Code Value (DICOM tag (0008,0100)) is an identifier that is unambiguous within
  /// the Coding Scheme denoted by Coding Scheme Designator and Coding Scheme Version.
  vtkGetStringMacro(CodeValue);
  vtkSetStringMacro(CodeValue);

  ///
  /// Coding Scheme Designator identifies the coding scheme
  /// in which the code for a term is defined. Standard coding scheme designators used
  /// in DICOM information interchange are listed in PS3.16. Other coding scheme designators,
  /// for both private and public coding schemes, may be used, in accordance with PS3.16.
  vtkGetStringMacro(CodingSchemeDesignator);
  vtkSetStringMacro(CodingSchemeDesignator);

  ///
  /// Code Meaning is text that has meaning to a human and conveys
  /// the meaning of the term defined by the combination of Code Value and Coding Scheme Designator.
  /// Though such a meaning can be "looked up" in the dictionary for the coding scheme, it is encoded
  /// for the convenience of applications that do not have access to such a dictionary.
  vtkGetStringMacro(CodeMeaning);
  vtkSetStringMacro(CodeMeaning);

  /// Convenience function for setting code value, coding scheme, and code meaning with a single method call
  virtual void SetValueSchemeMeaning(const std::string& value, const std::string& scheme, const std::string& meaning);

  ///
  /// Get content of the object as a single human-readable string.
  /// Example: ([hnsf'U], UCUM, "Hounsfield unit")
  std::string GetAsPrintableString();

  ///
  /// Get content of the object as a single machine-readable string.
  /// Example: CodeValue:[hnsf'U]|CodingSchemeDesignator:UCUM|CodeMeaning:Hounsfield unit
  std::string GetAsString();

  ///
  /// Set content of the object from a single machine-readable string.
  /// Example input: CodeValue:[hnsf'U]|CodingSchemeDesignator:UCUM|CodeMeaning:Hounsfield unit
  /// \return true on success
  bool SetFromString(const std::string& content);

protected:
  vtkCodedEntry();
  ~vtkCodedEntry() override;
  vtkCodedEntry(const vtkCodedEntry&);
  void operator=(const vtkCodedEntry&);

protected:
  char* CodeValue{nullptr};
  char* CodingSchemeDesignator{nullptr};
  char* CodeMeaning{nullptr};
};

#endif
