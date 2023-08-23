#include <QCoreApplication>

#include "qMRMLTranslator.h"

std::string qMRMLTranslator::Translate(const char *context, const char *sourceText, const char *disambiguation, int n)
{
  return QCoreApplication::translate(context, sourceText, disambiguation, n).toStdString();
}
