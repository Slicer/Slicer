/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerPythonManager_h
#define __qSlicerPythonManager_h

// SlicerQt includes
#include "qSlicerCorePythonManager.h"
#include "qSlicerBaseQTGUIExport.h"

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerPythonManager : public qSlicerCorePythonManager
{
  Q_OBJECT

public:
  typedef qSlicerCorePythonManager Superclass;
  qSlicerPythonManager(QObject* parent=0);
  ~qSlicerPythonManager();
  
protected:

  virtual QStringList pythonPaths();
  virtual void preInitialization();

};

#endif
