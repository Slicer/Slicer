/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerPythonManager_h
#define __qSlicerPythonManager_h

// qVTK includes
# include <qVTKAbstractPythonManager.h>

#include "qSlicerBaseQTCoreExport.h"

class PythonQtObjectPtr;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerPythonManager : public qVTKAbstractPythonManager
{
  Q_OBJECT

public:
  typedef qVTKAbstractPythonManager Superclass;
  qSlicerPythonManager(QObject* parent=NULL);
  ~qSlicerPythonManager();
  
protected:

  virtual QStringList pythonPaths();
  virtual void preInitialization();

};

#endif
