/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// SlicerQt includes
#include "qSlicerPythonManager.h"
#include "qSlicerApplication.h"
#include "qSlicerBaseQTGUIPythonQtDecorators.h"

// Qt includes
#include <QVariant>
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// VTK includes
#include <vtkObject.h>

// Slicer vtk includes
#include "vtkEventBroker.h"

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
}

//-----------------------------------------------------------------------------
QStringList qSlicerPythonManager::pythonPaths()
{  
  QStringList paths;  
  paths << Superclass::pythonPaths();
  
  return paths; 
}

//-----------------------------------------------------------------------------
// this is the callback helper function that isolates the 
// event broker from knowing about any particular scripting
// implementation of observations code
void Slicer3_BrokerScriptHandler ( const char *script, void *clientData )
{
  QString pythonScript = QString("global _tpycl; _tpycl.tcl_callback('%1')").arg(script);
  qSlicerPythonManager *pymgr = 
    reinterpret_cast<qSlicerPythonManager*>(clientData);
  logger.setTrace();
  logger.trace(QString("Running broker observation script: %1").arg(script));
  pymgr->executeString(pythonScript);
}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::preInitialization()
{
  Superclass::preInitialization();

  // Register decorators
  this->registerPythonQtDecorator(new qSlicerBaseQTGUIPythonQtDecorators(this));
  
  qSlicerApplication* app = qSlicerApplication::application();
  
  // Evaluate application script
  this->executeFile(app->slicerHome() + "/bin/Python/slicer/slicerqt.py");

  // -- event broker
  // - script handler to pass callback strings to the tcl interp
  // - synchronous mode so that redunant events do not get collapsed
  //   (this is compatible with standard vtk event behavior, but adds
  //   the ability to trace event execution and auto cleanup on 
  //   Delete events).
  //
  vtkEventBroker::GetInstance()->SetScriptHandler( Slicer3_BrokerScriptHandler, reinterpret_cast<void *>(this) );
  vtkEventBroker::GetInstance()->SetEventModeToSynchronous();
  this->addVTKSlicerObject ("slicer.broker", vtkEventBroker::GetInstance());


  // initialize global slicer.sliceViews dict
  // -- it gets populated in 
  // qSlicerLayoutManagerPrivate::createSliceView
  //    and then used by the scripted code that needs to access
  //    the slice views
  this->executeString(QString("slicer.sliceViews = {}"));

}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::addVTKObject(const char *name, vtkObject *obj)
{
  char pointerString[BUFSIZ];
  *pointerString = 0;
  if ( sizeof(unsigned int) == sizeof(vtkObject*) ) 
    {
    sprintf( pointerString, "%x", reinterpret_cast<uintptr_t>(obj) );
    }
  if ( sizeof(unsigned long) == sizeof(vtkObject*) ) 
    {
    sprintf( pointerString, "%lx", reinterpret_cast<uintptr_t>(obj) );
    }
  if ( !*pointerString )
    {
    qCritical() << "Can't convert vtk object to python object";
    return;
    }

  if ( strlen(name) > BUFSIZ - 100 )
    {
    qCritical() << "vtk object name too long";
    return;
    }

  char script[BUFSIZ];
  sprintf( script, "import vtk; %s = vtk.%s('_%s_%s_p')\n", 
    name, obj->GetClassName(), pointerString, obj->GetClassName() );
  this->executeString(QString(script));
}

//-----------------------------------------------------------------------------
void qSlicerPythonManager::addVTKSlicerObject(const char *name, vtkObject *obj)
{
  char pointerString[BUFSIZ];
  *pointerString = 0;
  if ( sizeof(unsigned int) == sizeof(vtkObject*) ) 
    {
    sprintf( pointerString, "%x", reinterpret_cast<uintptr_t>(obj) );
    }
  if ( sizeof(unsigned long) == sizeof(vtkObject*) ) 
    {
    sprintf( pointerString, "%lx", reinterpret_cast<uintptr_t>(obj) );
    }
  if ( !*pointerString )
    {
    qCritical() << "Can't convert slicer object to python object";
    return;
    }

  if ( strlen(name) > BUFSIZ - 100 )
    {
    qCritical() << "Slicer object name too long";
    return;
    }

  char script[BUFSIZ];
  sprintf( script, "import slicer; %s = slicer.%s('_%s_%s_p')\n", 
    name, obj->GetClassName(), pointerString, obj->GetClassName() );
  this->executeString(QString(script));
}
