/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QPointer>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModuleRepresentation.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLAbstractLogic.h"
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerAbstractCoreModulePrivate
{
public:
  qSlicerAbstractCoreModulePrivate();
  virtual ~qSlicerAbstractCoreModulePrivate();

  bool                                       Enabled;
  QString                                    Name;
  qSlicerAbstractModuleRepresentation*       WidgetRepresentation;
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
  vtkSmartPointer<vtkMRMLAbstractLogic>      Logic;
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
qSlicerAbstractCoreModule::qSlicerAbstractCoreModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerAbstractCoreModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule::~qSlicerAbstractCoreModule()
{
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::initialize(vtkSlicerApplicationLogic* _appLogic)
{
  this->setAppLogic(_appLogic);
  this->logic(); // Create the logic if it hasn't been created already.
  this->setup(); // Setup is a virtual pure method overloaded in subclass
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::printAdditionalInfo()
{
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::name()const
{
  Q_D(const qSlicerAbstractCoreModule);
  return d->Name;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::setName(const QString& _name)
{
  Q_D(qSlicerAbstractCoreModule);
  d->Name = _name;
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
int qSlicerAbstractCoreModule::index()const
{
  return -1;
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
CTK_GET_CPP(qSlicerAbstractCoreModule, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::setMRMLScene(vtkMRMLScene* _mrmlScene)
{
  Q_D(qSlicerAbstractCoreModule);
  if (d->MRMLScene == _mrmlScene)
    {
    return;
    }
  d->MRMLScene = _mrmlScene;
  // Since we don't want 'setMRMLScene' to instantiate explicitly the logic,
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
  Q_D(qSlicerAbstractCoreModule);
  d->AppLogic = newAppLogic;
  // here we don't want to create a logic if no logic exists yet. it's not setAppLogic
  // role to create logics.
  vtkSlicerModuleLogic* moduleLogic =
    vtkSlicerModuleLogic::SafeDownCast(d->Logic);
  if (moduleLogic)
    {
    moduleLogic->SetApplicationLogic(newAppLogic);
    }
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerAbstractCoreModule, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerAbstractCoreModule, bool, isEnabled, Enabled);
CTK_SET_CPP(qSlicerAbstractCoreModule, bool, setEnabled, Enabled);

//-----------------------------------------------------------------------------
bool qSlicerAbstractCoreModule::isHidden()const
{
  return false;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAbstractCoreModule::widgetRepresentation()
{
  Q_D(qSlicerAbstractCoreModule);

  // Since 'logic()' should have been called in 'initialize(), let's make
  // sure the 'logic()' method call is consistent and won't create a
  // diffent logic object
#ifndef QT_NO_DEBUG // Required to avoid undefined variable warning
  vtkMRMLAbstractLogic* currentLogic = d->Logic;
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
    // internally sets the logic and call setup, 
    d->WidgetRepresentation->setModule(this);
    // Note: setMRMLScene should be called after setup (just to make sure widgets
    // are well written and can handle empty mrmlscene
    d->WidgetRepresentation->setMRMLScene(this->mrmlScene());
    //d->WidgetRepresentation->setWindowTitle(this->title());
    }
  return d->WidgetRepresentation;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerAbstractCoreModule::logic()
{
  Q_D(qSlicerAbstractCoreModule);

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
  Q_D(qSlicerAbstractCoreModule);
  d->WidgetRepresentation = 0;
}
