#include "vtkMRMLTranslator.h"

std::string vtkMRMLTranslator::Translate(const char *vtkNotUsed(context), const char *sourceText, const char *vtkNotUsed(disambiguation), int vtkNotUsed(n))
{
  return sourceText;
}
