/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QDebug>
#include <QVariant>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerPythonManager.h"

#ifdef Slicer_USE_PYTHONQT_WITH_TCL
// SlicerVTK includes
# include "vtkEventBroker.h"
#endif

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
}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::executeInitializationScripts()
{
  qSlicerApplication* app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }

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
  //qDebug() << "Running broker observation script:" << script;
  self->executeString(pythonScript);
#else
  Q_UNUSED(script);
  Q_UNUSED(clientData);
#endif
}

