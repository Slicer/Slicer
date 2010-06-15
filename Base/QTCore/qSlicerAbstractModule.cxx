/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


// Qt includes
#include <QPointer>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
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
class qSlicerAbstractModulePrivate: public ctkPrivate<qSlicerAbstractModule>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerAbstractModule);
  qSlicerAbstractModulePrivate();
  virtual ~qSlicerAbstractModulePrivate();

  bool                                       Enabled;
  QString                                    Name;
  qSlicerAbstractModuleRepresentation*       WidgetRepresentation;
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
  vtkSmartPointer<vtkSlicerLogic>            Logic;
};

//-----------------------------------------------------------------------------
// qSlicerAbstractModulePrivate methods
qSlicerAbstractModulePrivate::qSlicerAbstractModulePrivate()
{
  this->Enabled = false;
  this->Name = "NA";
  this->WidgetRepresentation = 0;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModulePrivate::~qSlicerAbstractModulePrivate()
{
  // Delete the widget representation
  delete this->WidgetRepresentation;
}

//-----------------------------------------------------------------------------
// qSlicerAbstractModule methods

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::initialize(vtkSlicerApplicationLogic* _appLogic)
{
  this->setAppLogic(_appLogic);
  this->logic(); // Required to instanciate moduleLogic
  this->setup(); // Setup is a virtual pure method overloaded in subclass
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::printAdditionalInfo()
{
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::name()const
{
  return ctk_d()->Name;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setName(const QString& _name)
{
  ctk_d()->Name = _name;
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
CTK_GET_CXX(qSlicerAbstractModule, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  CTK_D(qSlicerAbstractModule);
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
void qSlicerAbstractModule::setAppLogic(vtkSlicerApplicationLogic* newAppLogic)
{
  CTK_D(qSlicerAbstractModule);
  vtkSlicerModuleLogic* moduleLogic = vtkSlicerModuleLogic::SafeDownCast(this->logic());
  if (moduleLogic)
    {
    moduleLogic->SetApplicationLogic(newAppLogic);
    }
  d->AppLogic = newAppLogic;
}

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerAbstractModule, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
CTK_GET_CXX(qSlicerAbstractModule, bool, isEnabled, Enabled);
CTK_SET_CXX(qSlicerAbstractModule, bool, setEnabled, Enabled);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAbstractModule::widgetRepresentation()
{
  CTK_D(qSlicerAbstractModule);

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
    Q_ASSERT(d->WidgetRepresentation);
    d->WidgetRepresentation->setModule(this);
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
    //d->WidgetRepresentation->setWindowTitle(this->title());
    }
  return d->WidgetRepresentation;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerAbstractModule::logic()
{
  CTK_D(qSlicerAbstractModule);

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
void qSlicerAbstractModule::representationDeleted()
{
  CTK_D(qSlicerAbstractModule);
  d->WidgetRepresentation = 0;
}
