#ifndef vtkMRMLTranslator_h
#define vtkMRMLTranslator_h

#include "vtkMRMLLogicExport.h"

// STD includes

#include <string>

class VTK_MRML_LOGIC_EXPORT vtkMRMLTranslator
{
public:

  /// Default noop translation function for logic classes
  virtual std::string Translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1);
};

#endif
