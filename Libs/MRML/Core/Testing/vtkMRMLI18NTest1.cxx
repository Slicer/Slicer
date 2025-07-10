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
    std::string Translate(const char* context, const char* sourceText,
                          const char* vtkNotUsed(disambiguation) /*= nullptr*/, int vtkNotUsed(n) /*= -1*/) override
    {
      return std::string("translated-") + context + sourceText;
    }

  protected:
    vtkTestTranslator () = default;
    ~vtkTestTranslator () override = default;
    vtkTestTranslator (const vtkTestTranslator&) = delete;
    void operator=(const vtkTestTranslator&) = delete;
  };

  vtkStandardNewMacro(vtkTestTranslator );
}

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

  CHECK_STD_STRING(vtkMRMLI18N::Format("Some text without replacement", "aaa"), "Some text without replacement");
  CHECK_STD_STRING(vtkMRMLI18N::Format("Some text with %1 replacement", "aaa"), "Some text with aaa replacement");
  CHECK_STD_STRING(vtkMRMLI18N::Format("Some text %2 with %1 replacement", "aaa", "qwerty"), "Some text qwerty with aaa replacement");
  CHECK_STD_STRING(vtkMRMLI18N::Format("Some text with missing %1 replacement %2 end", "aaa"), "Some text with missing aaa replacement  end");
  CHECK_STD_STRING(vtkMRMLI18N::Format("Some %2 with %1 escaping %%2 and %% end", "aaa", "qwerty"), "Some qwerty with aaa escaping %2 and % end");
  CHECK_STD_STRING(vtkMRMLI18N::Format("Some text edge case %", "aaa"), "Some text edge case %");

  return EXIT_SUCCESS;
}
