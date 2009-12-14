/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


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

