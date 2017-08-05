/*=auto=========================================================================

  Portions (c) Copyright 2010 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkCodedEntry.h"

// VTK includes
#include "vtkNew.h"

//----------------------------------------------------------------------------
int vtkCodedEntryTest1(int , char * [] )
{
  vtkNew<vtkCodedEntry> codedEntry;

  // Test set functions
  codedEntry->SetCodeValue("110852");
  CHECK_STD_STRING(codedEntry->GetAsPrintableString(), "(110852, (none), \"\")");

  codedEntry->SetCodingSchemeDesignator("DCM");
  CHECK_STD_STRING(codedEntry->GetAsPrintableString(), "(110852, DCM, \"\")");

  codedEntry->SetCodeMeaning("MR signal intensity");

  // Test getting as string
  CHECK_STD_STRING(codedEntry->GetAsPrintableString(), "(110852, DCM, \"MR signal intensity\")");
  CHECK_STD_STRING(codedEntry->GetAsString(), "CodeValue:110852|CodingSchemeDesignator:DCM|CodeMeaning:MR signal intensity");

  codedEntry->PrintSelf(std::cout, vtkIndent());

  // Test copy
  vtkNew<vtkCodedEntry> codedEntryCopy;
  codedEntryCopy->Copy(codedEntry.GetPointer());
  CHECK_STD_STRING(codedEntry->GetAsString(), "CodeValue:110852|CodingSchemeDesignator:DCM|CodeMeaning:MR signal intensity");

  // Test setting from string and get functions
  vtkNew<vtkCodedEntry> codedEntryParsed;
  codedEntryParsed->SetFromString(codedEntry->GetAsString());
  CHECK_STRING(codedEntryParsed->GetCodeValue(), "110852");
  CHECK_STRING(codedEntryParsed->GetCodingSchemeDesignator(), "DCM");
  CHECK_STRING(codedEntryParsed->GetCodeMeaning(), "MR signal intensity");

  return EXIT_SUCCESS;
}
