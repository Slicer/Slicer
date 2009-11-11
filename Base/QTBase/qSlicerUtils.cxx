
#include "qSlicerUtils.h"

#include <QStringList>
#include <QDebug>

//------------------------------------------------------------------------------
bool qSlicerUtils::isExecutableName(const QString& name)
{
  QStringList extensions;
  extensions << ".bat" << ".com" << ".sh" << ".csh" << ".tcsh" 
             << ".pl" << ".py" << ".tcl" << ".m" << ".exe";
             
  foreach(const QString& extension, extensions)
    {
    if (name.endsWith(extension, Qt::CaseInsensitive))
      {
      return true;
      }
    }
  return false;
}

