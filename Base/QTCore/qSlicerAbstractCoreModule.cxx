/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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
#include <QList>

// Slicer includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModuleRepresentation.h"
#include "qSlicerCoreApplication.h"

// SlicerLogic includes
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

  bool                                       Hidden;
  QString                                    Name;
  QString                                    Path;
  bool                                       Installed;
  bool                                       BuiltIn;
  bool                                       WidgetRepresentationCreationEnabled;
  qSlicerAbstractModuleRepresentation*       WidgetRepresentation;
  QList<qSlicerAbstractModuleRepresentation*> WidgetRepresentations;
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
  vtkSmartPointer<vtkMRMLAbstractLogic>      Logic;
};

//-----------------------------------------------------------------------------
// qSlicerAbstractCoreModulePrivate methods
qSlicerAbstractCoreModulePrivate::qSlicerAbstractCoreModulePrivate()
{
  this->Hidden = false;
  this->Name = "NA";
  this->WidgetRepresentation = nullptr;
  this->Installed = false;
  this->BuiltIn = true;
  this->WidgetRepresentationCreationEnabled = true;
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModulePrivate::~qSlicerAbstractCoreModulePrivate()
{
  // Delete the widget representation
  if (this->WidgetRepresentation)
    {
    delete this->WidgetRepresentation;
    }
  qDeleteAll(this->WidgetRepresentations.begin(), this->WidgetRepresentations.end());
  this->WidgetRepresentations.clear();
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
qSlicerAbstractCoreModule::~qSlicerAbstractCoreModule() = default;

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
QStringList qSlicerAbstractCoreModule::categories()const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractCoreModule::contributors()const
{
  return QStringList();
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
    moduleLogic->SetMRMLApplicationLogic(newAppLogic);
    }
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerAbstractCoreModule, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
bool qSlicerAbstractCoreModule::isHidden()const
{
  return this->isWidgetRepresentationCreationEnabled() ? false : true;
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractCoreModule::dependencies()const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerAbstractCoreModule, QString, path, Path);
CTK_SET_CPP(qSlicerAbstractCoreModule, const QString&, setPath, Path);

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerAbstractCoreModule, bool, isInstalled, Installed);
CTK_SET_CPP(qSlicerAbstractCoreModule, bool, setInstalled, Installed);

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerAbstractCoreModule, bool, isBuiltIn, BuiltIn);
CTK_SET_CPP(qSlicerAbstractCoreModule, bool, setBuiltIn, BuiltIn);

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerAbstractCoreModule, bool, isWidgetRepresentationCreationEnabled, WidgetRepresentationCreationEnabled);
CTK_SET_CPP(qSlicerAbstractCoreModule, bool, setWidgetRepresentationCreationEnabled, WidgetRepresentationCreationEnabled);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAbstractCoreModule::widgetRepresentation()
{
  Q_D(qSlicerAbstractCoreModule);

  // If required, create widgetRepresentation
  if (!d->WidgetRepresentation)
    {
    d->WidgetRepresentation = this->createNewWidgetRepresentation();
    }
  return d->WidgetRepresentation;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerAbstractCoreModule::createNewWidgetRepresentation()
{
  Q_D(qSlicerAbstractCoreModule);

  if (!this->isWidgetRepresentationCreationEnabled())
    {
    return nullptr;
    }

  // Since 'logic()' should have been called in 'initialize(), let's make
  // sure the 'logic()' method call is consistent and won't create a
  // diffent logic object
#ifndef QT_NO_DEBUG // Required to avoid undefined variable warning
  vtkMRMLAbstractLogic* currentLogic = d->Logic;
  Q_ASSERT(currentLogic == this->logic());
#endif

  qSlicerAbstractModuleRepresentation *newWidgetRepresentation;
  newWidgetRepresentation = this->createWidgetRepresentation();

  if (newWidgetRepresentation == nullptr)
    {
    qDebug() << "Warning, the module "<<this->name()<< "has no widget representation";
    return nullptr;
    }
  // internally sets the logic and call setup,
  newWidgetRepresentation->setModule(this);
  // Note: setMRMLScene should be called after setup (just to make sure widgets
  // are well written and can handle empty mrmlscene
  newWidgetRepresentation->setMRMLScene(this->mrmlScene());
  //d->WidgetRepresentation->setWindowTitle(this->title());
  // Add the copy of the widget representation to the list
  d->WidgetRepresentations << newWidgetRepresentation;

  return newWidgetRepresentation;
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
      moduleLogic->SetMRMLApplicationLogic(d->AppLogic);
      moduleLogic->SetModuleShareDirectory(vtkSlicerApplicationLogic::GetModuleShareDirectory(
                                       this->name().toStdString(), this->path().toStdString()));
      }
    d->Logic->SetMRMLScene(this->mrmlScene());
    }
  return d->Logic;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::representationDeleted(qSlicerAbstractModuleRepresentation *representation)
{
  Q_D(qSlicerAbstractCoreModule);

  // Just remove the list entry, the object storage has already been
  // deleted by caller.
  if (d->WidgetRepresentation == representation)
    {
    d->WidgetRepresentation = nullptr;
    }
  d->WidgetRepresentations.removeAll(representation);
}

//-----------------------------------------------------------------------------
QStringList qSlicerAbstractCoreModule::associatedNodeTypes()const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::defaultDocumentationLink()const
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  // Use "latest" version for Preview (installed preview release) and Experimental (developer build),
  // and use "majorVersion.minorVersion" for Stable release.
  QString version = QLatin1String("latest");
  if (app->releaseType() == "Stable")
    {
    version = QString("%1.%2").arg(app->majorVersion()).arg(app->minorVersion());
    }
  QString url = QString("https://slicer.readthedocs.io/en/%1/user_guide/modules/%2.html").arg(version).arg(this->name().toLower());
  QString linkText = QString("<p>For more information see the <a href=\"%1\">online documentation</a>.</p>").arg(url);
  return linkText;
}
