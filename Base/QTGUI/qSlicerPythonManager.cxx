/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// PythonQT includes
#include <PythonQt.h>

// CTK includes
#include <ctkLogger.h>
#include <ctkWidgetsPythonQtDecorators.h>
#include <ctkDICOMWidgetsPythonQtDecorators.h>

// SlicerQt includes
#include "qSlicerPythonManager.h"
#include "qSlicerApplication.h"
#include "qSlicerBaseQTGUIPythonQtDecorators.h"

#ifdef Slicer_USE_PYTHONQT_WITH_TCL
// SlicerVTK includes
#include "vtkEventBroker.h"
#endif

// PythonQt wrapper initialization methods
void PythonQt_init_org_commontk_CTKWidgets(PyObject*);
void PythonQt_init_org_commontk_CTKDICOMWidgets(PyObject*);
void PythonQt_init_org_commontk_CTKScriptingPythonWidgets(PyObject*);
void PythonQt_init_org_commontk_CTKVisualizationVTKWidgets(PyObject*);
void PythonQt_init_org_slicer_libs_qMRMLWidgets(PyObject*);

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtgui.qSlicerPythonManager");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
qSlicerPythonManager::qSlicerPythonManager(QObject* _parent) : Superclass(_parent)
{

}

//-----------------------------------------------------------------------------
qSlicerPythonManager::~qSlicerPythonManager()
{
#ifdef Slicer_USE_PYTHONQT_WITH_TCL
  vtkEventBroker::GetInstance()->SetScriptHandler(0, 0);
#endif
}

//-----------------------------------------------------------------------------
QStringList qSlicerPythonManager::pythonPaths()
{  
  QStringList paths;  
  paths << Superclass::pythonPaths();
  
  return paths; 
}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::preInitialization()
{
  Superclass::preInitialization();
  
  // Initialize wrappers
  PythonQt_init_org_commontk_CTKWidgets(0);
  PythonQt_init_org_commontk_CTKDICOMWidgets(0);
  PythonQt_init_org_commontk_CTKScriptingPythonWidgets(0);
  PythonQt_init_org_commontk_CTKVisualizationVTKWidgets(0);
  PythonQt_init_org_slicer_libs_qMRMLWidgets(0);

  // Register decorators
  this->registerPythonQtDecorator(new ctkWidgetsPythonQtDecorators());
  this->registerPythonQtDecorator(new ctkDICOMWidgetsPythonQtDecorators());
  this->registerPythonQtDecorator(new qSlicerBaseQTGUIPythonQtDecorators(this));
}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::executeInitializationScripts()
{
  qSlicerApplication* app = qSlicerApplication::application();

  // Evaluate application script
  this->executeFile(app->slicerHome() + "/bin/Python/slicer/slicerqt.py");

#ifdef Slicer_USE_PYTHONQT_WITH_TCL
  // Evaluate application script specific to the TCL layer
  // Note that it should be sourced after slicerqt.py
  this->executeFile(app->slicerHome() + "/bin/Python/slicer/slicerqt-with-tcl.py");

  // -- event broker
  // - script handler to pass callback strings to the tcl interpeter
  // - synchronous mode so that redundant events do not get collapsed
  //   (this is compatible with standard vtk event behavior, but adds
  //   the ability to trace event execution and auto cleanup on Delete events).
  vtkEventBroker::GetInstance()->SetScriptHandler(qSlicerPythonManager::eventBrokerScriptHandler,
                                                  reinterpret_cast<void *>(this));
  vtkEventBroker::GetInstance()->SetEventModeToSynchronous();
  this->addVTKObjectToPythonMain("slicer.broker", vtkEventBroker::GetInstance());
#endif
}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::eventBrokerScriptHandler(const char *script, void *clientData)
{
#ifdef Slicer_USE_PYTHONQT_WITH_TCL
  QString pythonScript = QString("global _tpycl; _tpycl.tcl_callback('%1')").arg(script);
  qSlicerPythonManager * self = reinterpret_cast<qSlicerPythonManager*>(clientData);
  Q_ASSERT(self);
  //logger.setTrace();
  logger.trace(QString("Running broker observation script: %1").arg(script));
  self->executeString(pythonScript);
#else
  Q_UNUSED(script);
  Q_UNUSED(clientData);
#endif
}

