/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLTranslator.h>

namespace
{

class vtkTestTranslator : public vtkMRMLTranslator
{
public:
  static vtkTestTranslator* New();
  vtkTypeMacro(vtkTestTranslator, vtkMRMLTranslator);

  /// Translation method for testing that returns "translated-(context)(sourceText)" as translation
  std::string Translate(const char* context,
                        const char* sourceText,
                        const char* vtkNotUsed(disambiguation) /*= nullptr*/,
                        int vtkNotUsed(n) /*= -1*/) override
  {
    return std::string("translated-") + context + sourceText;
  }

protected:
  vtkTestTranslator() = default;
  ~vtkTestTranslator() override = default;
  vtkTestTranslator(const vtkTestTranslator&) = delete;
  void operator=(const vtkTestTranslator&) = delete;
};

vtkStandardNewMacro(vtkTestTranslator);
} // namespace

int vtkMRMLI18NTest1(int, char*[])
{
  // Check default translation (simply sourcetext is returned)
  CHECK_STD_STRING(vtkMRMLI18N::Translate("SomeContext", "SomeMessage"), "SomeMessage");

  // Set custom translator
  vtkNew<vtkTestTranslator> translator;
  vtkMRMLI18N::GetInstance()->SetTranslator(translator);

  // Check translation with custom translator
  CHECK_STD_STRING(vtkMRMLI18N::Translate("SomeContext", "SomeMessage"), "translated-SomeContextSomeMessage");
  // Use translation convenience function
  CHECK_STD_STRING(vtkMRMLTr("SomeContext", "SomeMessage"), "translated-SomeContextSomeMessage");

  return EXIT_SUCCESS;
}
