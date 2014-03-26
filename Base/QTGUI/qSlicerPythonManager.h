/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerPythonManager_h
#define __qSlicerPythonManager_h

// SlicerQt includes
#include "qSlicerCorePythonManager.h"
#include "qSlicerBaseQTGUIExport.h"

class vtkObject;

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
  virtual void executeInitializationScripts();

private:

  /// This is the callback helper function that isolates the event broker from
  /// knowing about any particular scripting implementation of observations code.
  static void eventBrokerScriptHandler(const char *script, void *clientData);

};

#endif
