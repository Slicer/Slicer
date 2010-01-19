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
class qSlicerAbstractModulePrivate: public qCTKPrivate<qSlicerAbstractModule>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerAbstractModule);
  qSlicerAbstractModulePrivate()
    {
    this->ModuleEnabled = false;
    this->Logic = 0;
    this->Name = "NA"; 
    }
  ~qSlicerAbstractModulePrivate();
  
  bool                                       ModuleEnabled;
  QString                                    Name;
  QPointer<qSlicerAbstractModuleWidget>      WidgetRepresentation;
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
  qSlicerModuleLogic*                        Logic; 
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::initialize(vtkSlicerApplicationLogic* _appLogic)
{
  QCTK_D(qSlicerAbstractModule);
  Q_ASSERT(_appLogic);
  //this->setAppLogic(appLogic);
  if (d->Logic)
    {
    d->Logic->initialize(_appLogic);
    }
  d->AppLogic = _appLogic; 
  this->setup();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::printAdditionalInfo()
{
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::name()const
{
  return qctk_d()->Name;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setName(const QString& _name)
{
  qctk_d()->Name = _name;
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::category()const 
{ 
  return QString(); 
}
 
//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::contributor()const 
{ 
  return QString(); 
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::helpText()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::acknowledgementText()const 
{ 
  return QString();
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractModule, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  QCTK_D(qSlicerAbstractModule);
  if (d->MRMLScene == _mrmlScene)
    {
    return; 
    }
  d->MRMLScene = _mrmlScene;
  if (d->Logic)
    {// logic should be updated first (because it doesn't depends on the widget
    d->Logic->setMRMLScene(_mrmlScene);
    }
  if (d->WidgetRepresentation)
    {
    d->WidgetRepresentation->setMRMLScene(_mrmlScene);
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
  this->logic();

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
  if (d->Logic)
    {
    return d->Logic;
    }
  d->Logic = this->createLogic();
  if (d->Logic)
    {
    d->Logic->initialize(d->AppLogic);
    d->Logic->setMRMLScene(this->mrmlScene());
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
