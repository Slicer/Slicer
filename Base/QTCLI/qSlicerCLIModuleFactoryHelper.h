/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerCLIModuleFactoryHelper_h
#define __qSlicerCLIModuleFactoryHelper_h

// QT includes
#include <QStringList>

#include "qSlicerBaseQTCLIExport.h"

class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIModuleFactoryHelper
{
public:

  // Description:
  // Convenient method returning a list of module paths that the factory could use
  static const QStringList modulePaths();

private:
  // Not implemented
  qSlicerCLIModuleFactoryHelper(){}
  virtual ~qSlicerCLIModuleFactoryHelper(){}
};

#endif
