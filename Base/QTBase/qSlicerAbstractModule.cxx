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

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerLogic.h"
#include "vtkSlicerModuleLogic.h"

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
    this->Enabled = false;
    this->Logic = 0;
    this->Name = "NA"; 
    }
  ~qSlicerAbstractModulePrivate();
  
  bool                                       Enabled;
  QString                                    Name;
  QPointer<qSlicerAbstractModuleWidget>      WidgetRepresentation;
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
  vtkSmartPointer<vtkSlicerLogic>            Logic; 
};

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
}

//-----------------------------------------------------------------------------
// qSlicerAbstractModule methods

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::initialize(vtkSlicerApplicationLogic* appLogic)
{
  this->setAppLogic(appLogic);

  vtkSlicerLogic* moduleLogic = this->logic();
  if (moduleLogic)
    {
    //moduleLogic->initialize();
    }
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
    d->Logic->SetMRMLScene(_mrmlScene);
    }
  if (d->WidgetRepresentation)
    {
    d->WidgetRepresentation->setMRMLScene(_mrmlScene);
    }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setAppLogic(vtkSlicerApplicationLogic* newAppLogic)
{
  QCTK_D(qSlicerAbstractModule);
  vtkSlicerModuleLogic* moduleLogic = vtkSlicerModuleLogic::SafeDownCast(this->logic());
  if (moduleLogic)
    {
    moduleLogic->SetApplicationLogic(newAppLogic);
    }
  d->AppLogic = newAppLogic;
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractModule, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractModule, bool, isEnabled, Enabled);
QCTK_SET_CXX(qSlicerAbstractModule, bool, setEnabled, Enabled);

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
    // WidgetRepresentation->setMRMLScene() because some methods
    // might need a logic when a MRML scene is set.
    if (d->Logic)
      {
      d->WidgetRepresentation->setLogic(d->Logic);
      }
    d->WidgetRepresentation->setName(this->name());
    d->WidgetRepresentation->setup();
    // Note: setMRMLScene should be called after setup (just to make sure widgets
    // are well written and can handle empty mrmlscene
    d->WidgetRepresentation->setMRMLScene(this->mrmlScene());
    d->WidgetRepresentation->setWindowTitle(this->title());
    }
  return d->WidgetRepresentation; 
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerAbstractModule::logic()
{
  QCTK_D(qSlicerAbstractModule);
  if (d->Logic)
    {
    return d->Logic;
    }
  d->Logic.TakeReference(this->createLogic());
  if (d->Logic)
    {
    vtkSlicerModuleLogic* moduleLogic = vtkSlicerModuleLogic::SafeDownCast(d->Logic);
    if (moduleLogic)
      {
      moduleLogic->SetApplicationLogic(d->AppLogic);
      }
    d->Logic->SetMRMLScene(this->mrmlScene());
    }
  return d->Logic; 
}

