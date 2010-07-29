/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QDebug>
#include <QPointer>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModuleRepresentation.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerLogic.h"
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerAbstractCoreModulePrivate: public ctkPrivate<qSlicerAbstractCoreModule>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerAbstractCoreModule);
  qSlicerAbstractCoreModulePrivate();
  virtual ~qSlicerAbstractCoreModulePrivate();

  bool                                       Enabled;
  QString                                    Name;
  qSlicerAbstractModuleRepresentation*       WidgetRepresentation;
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
  vtkSmartPointer<vtkSlicerLogic>            Logic;
};

//-----------------------------------------------------------------------------
// qSlicerAbstractCoreModulePrivate methods
qSlicerAbstractCoreModulePrivate::qSlicerAbstractCoreModulePrivate()
{
  this->Enabled = false;
  this->Name = "NA";
  this->WidgetRepresentation = 0;
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModulePrivate::~qSlicerAbstractCoreModulePrivate()
{
  // Delete the widget representation
  delete this->WidgetRepresentation;
}

//-----------------------------------------------------------------------------
// qSlicerAbstractCoreModule methods

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractCoreModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::initialize(vtkSlicerApplicationLogic* _appLogic)
{
  this->setAppLogic(_appLogic);
  this->logic(); // Required to instanciate moduleLogic
  this->setup(); // Setup is a virtual pure method overloaded in subclass
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::printAdditionalInfo()
{
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::name()const
{
  return ctk_d()->Name;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::setName(const QString& _name)
{
  ctk_d()->Name = _name;
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::category()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::contributor()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::helpText()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::acknowledgementText()const
{
  return QString();
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerAbstractCoreModule, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  CTK_D(qSlicerAbstractCoreModule);
  if (d->MRMLScene == _mrmlScene)
    {
    return;
    }
  d->MRMLScene = _mrmlScene;
  // Since we don't want 'setMRMLScene' to instanciate explicitly the logic,
  // we just check the pointer (instead of calling 'this->logic()')
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
void qSlicerAbstractCoreModule::setAppLogic(vtkSlicerApplicationLogic* newAppLogic)
{
  CTK_D(qSlicerAbstractCoreModule);
  vtkSlicerModuleLogic* moduleLogic = vtkSlicerModuleLogic::SafeDownCast(this->logic());
  if (moduleLogic)
    {
    moduleLogic->SetApplicationLogic(newAppLogic);
    }
  d->AppLogic = newAppLogic;
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerAbstractCoreModule, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerAbstractCoreModule, bool, isEnabled, Enabled);
CTK_SET_CXX(qSlicerAbstractCoreModule, bool, setEnabled, Enabled);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAbstractCoreModule::widgetRepresentation()
{
  CTK_D(qSlicerAbstractCoreModule);

  // Since 'logic()' should have been called in 'initialize(), let's make
  // sure the 'logic()' method call is consistent and won't create a
  // diffent logic object
#ifndef QT_NO_DEBUG // Required to avoid undefined variable warning
  vtkSlicerLogic* currentLogic = d->Logic;
  Q_ASSERT(currentLogic == this->logic());
#endif

  // If required, create widgetRepresentation
  if (!d->WidgetRepresentation)
    {
    d->WidgetRepresentation = this->createWidgetRepresentation();
    if (d->WidgetRepresentation == 0)
      {
      qDebug() << "Warning, the module "<<this->title()<< "has no widget representation";
      return 0;
      }
    d->WidgetRepresentation->setModule(this);
    d->WidgetRepresentation->setup();
    // Note: setMRMLScene should be called after setup (just to make sure widgets
    // are well written and can handle empty mrmlscene
    d->WidgetRepresentation->setMRMLScene(this->mrmlScene());
    //d->WidgetRepresentation->setWindowTitle(this->title());
    }
  return d->WidgetRepresentation;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerAbstractCoreModule::logic()
{
  CTK_D(qSlicerAbstractCoreModule);

  // Return a logic object is one already exists
  if (d->Logic)
    {
    return d->Logic;
    }
  // Attempt to create a logic object
  d->Logic.TakeReference(this->createLogic());

  // If createLogic return a valid object, set its Scene and AppLogic
  // Note also that, in case no logic is associated with the module,
  // 'createLogic()' could return 0
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

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::representationDeleted()
{
  CTK_D(qSlicerAbstractCoreModule);
  d->WidgetRepresentation = 0;
}
