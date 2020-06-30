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
#include <QFileInfo>

// PythonQt includes
#include <PythonQt.h>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerScriptedLoadableModule.h"
#include "qSlicerScriptedLoadableModuleWidget.h"
#include "qSlicerScriptedFileDialog.h"
#include "qSlicerScriptedFileReader.h"
#include "qSlicerScriptedFileWriter.h"
#include "qSlicerScriptedUtils_p.h"
#include "vtkSlicerScriptedLoadableModuleLogic.h"

//-----------------------------------------------------------------------------
class qSlicerScriptedLoadableModulePrivate
{
public:
  typedef qSlicerScriptedLoadableModulePrivate Self;
  qSlicerScriptedLoadableModulePrivate();
  virtual ~qSlicerScriptedLoadableModulePrivate();

  QString Title;
  QStringList Categories;
  QStringList Contributors;
  QStringList AssociatedNodeTypes;
  QStringList Dependencies;
  QString HelpText;
  QString AcknowledgementText;
  QIcon   Icon;
  bool   Hidden;
  QVariantMap   Extensions;
  int Index;

  enum {
    SetupMethod = 0
    };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString    PythonSource;
};

//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModulePrivate::qSlicerScriptedLoadableModulePrivate()
{
  this->Hidden = false;
  this->Index = -1;

  this->PythonCppAPI.declareMethod(Self::SetupMethod, "setup");
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModulePrivate::~qSlicerScriptedLoadableModulePrivate() = default;


//-----------------------------------------------------------------------------
// qSlicerScriptedLoadableModule methods

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModule::qSlicerScriptedLoadableModule(QObject* _parentObject)
  : Superclass(_parentObject)
  , d_ptr(new qSlicerScriptedLoadableModulePrivate)
{
  Q_D(qSlicerScriptedLoadableModule);
  d->Icon = this->Superclass::icon();
}

//-----------------------------------------------------------------------------
qSlicerScriptedLoadableModule::~qSlicerScriptedLoadableModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModule::pythonSource()const
{
  Q_D(const qSlicerScriptedLoadableModule);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModule::setPythonSource(const QString& newPythonSource)
{
  Q_D(qSlicerScriptedLoadableModule);

  if (!Py_IsInitialized())
    {
    return false;
    }

  if (!newPythonSource.endsWith(".py") && !newPythonSource.endsWith(".pyc"))
    {
    return false;
    }

  // Extract moduleName from the provided filename
  QString moduleName = QFileInfo(newPythonSource).baseName();
  this->setName(moduleName);
  QString className = moduleName;

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Get a reference (or create if needed) the <moduleName> python module
  PyObject * module = PyImport_AddModule(moduleName.toUtf8());

  // Get a reference to the python module class to instantiate
  PythonQtObjectPtr classToInstantiate;
  if (module && PyObject_HasAttrString(module, className.toUtf8()))
    {
    classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toUtf8()));
    }
  if (!classToInstantiate)
    {
    PythonQtObjectPtr local_dict;
    local_dict.setNewRef(PyDict_New());
    if (!qSlicerScriptedUtils::loadSourceAsModule(moduleName, newPythonSource, global_dict, local_dict))
      {
      return false;
      }
    if (PyObject_HasAttrString(module, className.toUtf8()))
      {
      classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toUtf8()));
      }
    }

  if (!classToInstantiate)
    {
    PythonQt::self()->handleError();
    PyErr_SetString(PyExc_RuntimeError,
                    QString("qSlicerScriptedLoadableModule::setPythonSource - "
                            "Failed to load scripted loadable module: "
                            "class %1 was not found in file %2").arg(className).arg(newPythonSource).toLatin1());
    PythonQt::self()->handleError();
    return false;
    }

  d->PythonCppAPI.setObjectName(className);

  PyObject* self = d->PythonCppAPI.instantiateClass(this, className, classToInstantiate);
  if (!self)
    {
    return false;
    }

  d->PythonSource = newPythonSource;

  if (!qSlicerScriptedUtils::setModuleAttribute(
        "slicer.modules", moduleName + "Instance", self))
    {
    qCritical() << "Failed to set" << ("slicer.modules." + moduleName + "Instance");
    }

  // Check if there is module widget class
  QString widgetClassName = className + "Widget";
  if (!PyObject_HasAttrString(module, widgetClassName.toLatin1()))
    {
    this->setWidgetRepresentationCreationEnabled(false);
    }

  return true;
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModule::setup()
{
  Q_D(qSlicerScriptedLoadableModule);

  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (app)
    {
    // Set to /path/to/lib/Slicer-X.Y/qt-scripted-modules
    QString modulePath = QFileInfo(this->path()).absolutePath();
    // Set to /path/to/lib/Slicer-X.Y
    modulePath = QFileInfo(modulePath).absolutePath();
    // Set to /path/to/lib/Slicer-X.Y/qt-loadable-modules
    modulePath = modulePath + "/" Slicer_QTLOADABLEMODULES_SUBDIR;

    bool isEmbedded = app->isEmbeddedModule(this->path());
    if (!isEmbedded)
      {
      if (!qSlicerLoadableModule::importModulePythonExtensions(
            app->corePythonManager(), app->intDir(), modulePath, isEmbedded))
        {
        qWarning() << "qSlicerLoadableModule::setup - Failed to import module" << this->name() << "python extensions";
        }
      }
    }

  this->registerFileDialog();
  this->registerIO();
  d->PythonCppAPI.callMethod(Pimpl::SetupMethod);
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModule::registerFileDialog()
{
  Q_D(qSlicerScriptedLoadableModule);
  QScopedPointer<qSlicerScriptedFileDialog> fileDialog(new qSlicerScriptedFileDialog(this));
  bool ret = fileDialog->setPythonSource(d->PythonSource);
  if (!ret)
    {
    return;
    }
  qSlicerApplication::application()->ioManager()
    ->registerDialog(fileDialog.take());
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModule::registerIO()
{
  Q_D(qSlicerScriptedLoadableModule);
  QScopedPointer<qSlicerScriptedFileWriter> fileWriter(new qSlicerScriptedFileWriter(this));
  bool ret = fileWriter->setPythonSource(d->PythonSource);
  if (ret)
    {
    qSlicerApplication::application()->ioManager()->registerIO(fileWriter.take());
    }
  QScopedPointer<qSlicerScriptedFileReader> fileReader(new qSlicerScriptedFileReader(this));
  ret = fileReader->setPythonSource(d->PythonSource);
  if (ret)
    {
    qSlicerApplication::application()->ioManager()->registerIO(fileReader.take());
    }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerScriptedLoadableModule::createWidgetRepresentation()
{
  Q_D(qSlicerScriptedLoadableModule);

  if (!this->isWidgetRepresentationCreationEnabled())
    {
    return nullptr;
    }

  QScopedPointer<qSlicerScriptedLoadableModuleWidget> widget(new qSlicerScriptedLoadableModuleWidget);
  bool ret = widget->setPythonSource(d->PythonSource);
  if (!ret)
    {
    return nullptr;
    }

  return widget.take();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerScriptedLoadableModule::createLogic()
{
//  Q_D(qSlicerScriptedLoadableModule);

  vtkSlicerScriptedLoadableModuleLogic* logic = vtkSlicerScriptedLoadableModuleLogic::New();

//  bool ret = logic->SetPythonSource(d->PythonSource.toStdString());
//  if (!ret)
//    {
//    logic->Delete();
//    return 0;
//    }

  return logic;
}

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QString&, setTitle, Title)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QString, title, Title)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QStringList&, setCategories, Categories)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QStringList, categories, Categories)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QStringList&, setContributors, Contributors)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QStringList, contributors, Contributors)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QStringList&, setAssociatedNodeTypes, AssociatedNodeTypes)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QStringList, associatedNodeTypes, AssociatedNodeTypes)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QString&, setHelpText, HelpText)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QString, helpText, HelpText)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QString&, setAcknowledgementText, AcknowledgementText)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QString, acknowledgementText, AcknowledgementText)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QVariantMap&, setExtensions, Extensions)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QVariantMap, extensions, Extensions)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QIcon&, setIcon, Icon)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QIcon, icon, Icon)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, bool, setHidden, Hidden)
CTK_GET_CPP(qSlicerScriptedLoadableModule, bool, isHidden, Hidden)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const QStringList&, setDependencies, Dependencies)
CTK_GET_CPP(qSlicerScriptedLoadableModule, QStringList, dependencies, Dependencies)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerScriptedLoadableModule, const int, setIndex, Index)
CTK_GET_CPP(qSlicerScriptedLoadableModule, int, index, Index)
