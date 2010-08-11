/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __qSlicerCorePythonManager_h
#define __qSlicerCorePythonManager_h

// CTK includes
# include <ctkAbstractPythonManager.h>

#include "qSlicerBaseQTCoreExport.h"

class PythonQtObjectPtr;
class vtkObject;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerCorePythonManager : public ctkAbstractPythonManager
{
  Q_OBJECT

public:
  typedef ctkAbstractPythonManager Superclass;
  qSlicerCorePythonManager(QObject* parent=0);
  ~qSlicerCorePythonManager();

  /// Convenient function allowing to add a VTK object to the interpreter main module
  void addVTKObjectToPythonMain(const QString& name, vtkObject * object);
  
protected:

  virtual QStringList pythonPaths();
  virtual void preInitialization();

};

#endif
