/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#include "qSlicerAbstractModule.h"

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerModuleLogic.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// QT includes
#include <QPointer>

//-----------------------------------------------------------------------------
struct qSlicerAbstractModulePrivate: public qCTKPrivate<qSlicerAbstractModule>
{
  QCTK_DECLARE_PUBLIC(qSlicerAbstractModule);
  qSlicerAbstractModulePrivate()
    {
    this->ModuleEnabled = false;
    this->Logic = 0;
    }
  ~qSlicerAbstractModulePrivate();
  
  bool                                       ModuleEnabled;
  QPointer<qSlicerAbstractModuleWidget>      WidgetRepresentation;
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
  qSlicerModuleLogic*                        Logic; 
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::initialize(vtkSlicerApplicationLogic* appLogic)
{
  Q_ASSERT(appLogic);
  //this->setAppLogic(appLogic);
  qctk_d()->AppLogic = appLogic; 
  this->setup();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::printAdditionalInfo()
{
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractModule, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setMRMLScene(vtkMRMLScene* mrmlScene)
{
  QCTK_D(qSlicerAbstractModule);
  if (d->MRMLScene == mrmlScene)
    {
    return; 
    }
  d->MRMLScene = mrmlScene;
  if (d->WidgetRepresentation)
    {
    d->WidgetRepresentation->setMRMLScene(mrmlScene);
    }
  if (d->Logic)
    {
    d->Logic->setMRMLScene(mrmlScene);
    }
}

//-----------------------------------------------------------------------------
//QCTK_SET_CXX(qSlicerAbstractModule, vtkSlicerApplicationLogic*, setAppLogic, AppLogic);
QCTK_GET_CXX(qSlicerAbstractModule, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractModule, bool, moduleEnabled, ModuleEnabled);
QCTK_SET_CXX(qSlicerAbstractModule, bool, setModuleEnabled, ModuleEnabled);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget* qSlicerAbstractModule::widgetRepresentation()
{
  QCTK_D(qSlicerAbstractModule);
  
  // If required, create module logic
  if (!d->Logic)
    {
    d->Logic = this->createLogic(); 
    }

  // If required, create widgetRepresentation
  if (!d->WidgetRepresentation)
    {
    d->WidgetRepresentation = this->createWidgetRepresentation();
    Q_ASSERT(d->WidgetRepresentation);
    // Note: WidgetRepresentation->setLogic should be called before
    // WidgetRepresentation->setMRMLScene
    if (d->Logic)
      {
      d->WidgetRepresentation->setLogic(d->Logic);
      }
      
    d->WidgetRepresentation->setName(this->name());
    d->WidgetRepresentation->initialize();
    // Note: setMRMLScene should be called after initialize
    d->WidgetRepresentation->setMRMLScene(this->mrmlScene());
    d->WidgetRepresentation->setWindowTitle(this->title());
    }
  return d->WidgetRepresentation; 
}

//-----------------------------------------------------------------------------
qSlicerModuleLogic* qSlicerAbstractModule::logic()
{
  QCTK_D(qSlicerAbstractModule);
  if (!d->Logic)
    {
    d->Logic = this->createLogic();
    }
  return d->Logic; 
}

//-----------------------------------------------------------------------------
// qSlicerAbstractModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerAbstractModulePrivate::~qSlicerAbstractModulePrivate()
{
  // Delete the widget representation
  if (this->WidgetRepresentation)
    {
    delete this->WidgetRepresentation;
    }
  // Delete the Logic
  if (this->Logic)
    {
    delete this->Logic; 
    }
}
