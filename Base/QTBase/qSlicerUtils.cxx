/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


// QT includes
#include <QStringList>
#include <QFile>
#include <QDebug>

// SlicerQt includes
#include "qSlicerUtils.h"

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

//------------------------------------------------------------------------------
QString qSlicerUtils::searchTargetInIntDir(const QString& directory, const QString& target)
{
#ifdef _WIN32
  QStringList intDirs;
  intDirs << "." << "Debug" << "RelWithDebInfo" << "Release" << "MinSizeRel";
  QString intDir = directory + "/%2/" + target;
  foreach(const QString& subdir, intDirs)
    {
    if (QFile::exists(intDir.arg(subdir)))
      {
      return directory+"/"+subdir+"/";
      }
    }
  return QString();
#else
  Q_UNUSED(target);
  return directory; 
#endif
}

//------------------------------------------------------------------------------
QString qSlicerUtils::executableExtension()
{
#ifdef _WIN32
  return QLatin1String(".exe"); 
#else
  return QString();
#endif
}

//-----------------------------------------------------------------------------
QString qSlicerUtils::extractModuleNameFromLibraryName(const QString& libraryName)
{
  QString moduleName = libraryName;
  
  // Truncate string before first dot "."
  moduleName.truncate(moduleName.indexOf("."));
  
  // Remove prefix 'lib' if needed
  if (moduleName.indexOf("lib") == 0)
    {
    moduleName.remove(0, 3);
    }

  // Remove prefix 'qSlicer' if needed
  if (moduleName.indexOf("qSlicer") == 0)
    {
    moduleName.remove(0, 7);
    }

  // Remove suffix 'Module' if needed
  int index = moduleName.lastIndexOf("Module");
  if (index != -1)
    {
    moduleName.remove(index, 6);
    }
    
  // Remove suffix 'Lib' if needed
  index = moduleName.lastIndexOf("Lib");
  if (index == (moduleName.size() - 3))
    {
    moduleName.remove(index, 3);
    }

  return moduleName.toLower();
}
