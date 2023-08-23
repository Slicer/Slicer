#ifndef qMRMLTranslator_h
#define qMRMLTranslator_h

#include "vtkMRMLTranslator.h"

#include "qSlicerBaseQTCoreExport.h"

class Q_SLICER_BASE_QTCORE_EXPORT qMRMLTranslator: public vtkMRMLTranslator
{
public:

  /// Translation function for logic classes
  std::string Translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) override;
};

#endif
