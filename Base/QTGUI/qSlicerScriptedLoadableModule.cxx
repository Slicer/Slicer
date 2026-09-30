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
#include "qSlicerScriptedUtils_p.h"
#include "vtkSlicerScriptedLoadableModuleLogic.h"

// MRML includes
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLFileIOHandler.h>
#include <vtkMRMLFileIOManager.h>

// VTK includes
#include <vtkNew.h>
#include <vtkPythonUtil.h>
#include <vtkWeakPointer.h>

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
  QIcon Icon;
  bool Hidden;
  QVariantMap Extensions;
  int Index;

  enum
  {
    SetupMethod = 0
  };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString PythonSourceFilePath;

  /// File IO manager where the scripted reader and writer of the module are registered
  vtkWeakPointer<vtkMRMLFileIOManager> RegisteredFileIOManager;
  /// Registered scripted reader and writer
  std::vector<vtkWeakPointer<vtkMRMLFileIOHandler>> RegisteredFileIOHandlers;

  /// Unregister the scripted reader and writer that were registered by registerIO()
  void unregisterFileIOHandlers();
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
void qSlicerScriptedLoadableModulePrivate::unregisterFileIOHandlers()
{
  if (this->RegisteredFileIOManager)
  {
    for (vtkMRMLFileIOHandler* handler : this->RegisteredFileIOHandlers)
    {
      this->RegisteredFileIOManager->UnregisterHandler(handler); // deleted handlers (nullptr) are ignored
    }
  }
  this->RegisteredFileIOHandlers.clear();
  this->RegisteredFileIOManager = nullptr;
}

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
qSlicerScriptedLoadableModule::~qSlicerScriptedLoadableModule()
{
  Q_D(qSlicerScriptedLoadableModule);
  // Unregister the scripted reader and writer while Python is still available
  d->unregisterFileIOHandlers();
}

//-----------------------------------------------------------------------------
QString qSlicerScriptedLoadableModule::pythonSource() const
{
  Q_D(const qSlicerScriptedLoadableModule);
  return d->PythonSourceFilePath;
}

//-----------------------------------------------------------------------------
bool qSlicerScriptedLoadableModule::setPythonSource(const QString& filePath)
{
  Q_D(qSlicerScriptedLoadableModule);

  if (!Py_IsInitialized())
  {
    return false;
  }

  if (!filePath.endsWith(".py") && !filePath.endsWith(".pyc"))
  {
    return false;
  }

  // Extract moduleName from the provided filename
  QString moduleName = QFileInfo(filePath).baseName();
  this->setName(moduleName);
  QString className = moduleName;

  // Get a reference to the main module and global dictionary
  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* global_dict = PyModule_GetDict(main_module);

  // Get actual module from sys.modules
  PyObject* sysModules = PyImport_GetModuleDict();
  PyObject* module = PyDict_GetItemString(sysModules, moduleName.toUtf8());

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
    if (!qSlicerScriptedUtils::loadSourceAsModule(moduleName, filePath, global_dict, local_dict))
    {
      return false;
    }

    // After loading, re-fetch actual module from sys.modules
    module = PyDict_GetItemString(PyImport_GetModuleDict(), moduleName.toUtf8());

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
                            "class %1 was not found in file %2")
                      .arg(className)
                      .arg(filePath)
                      .toLatin1());
    PythonQt::self()->handleError();
    return false;
  }

  d->PythonCppAPI.setObjectName(className);

  PyObject* self = d->PythonCppAPI.instantiateClass(this, className, classToInstantiate);
  if (!self)
  {
    return false;
  }

  d->PythonSourceFilePath = filePath;

  if (!qSlicerScriptedUtils::setModuleAttribute("slicer.modules", moduleName + "Instance", self))
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
  this->registerFileDialog();
  this->registerIO();
  d->PythonCppAPI.callMethod(Pimpl::SetupMethod);
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModule::registerFileDialog()
{
  Q_D(qSlicerScriptedLoadableModule);
  QScopedPointer<qSlicerScriptedFileDialog> fileDialog(new qSlicerScriptedFileDialog(this));
  bool ret = fileDialog->setPythonSource(d->PythonSourceFilePath);
  if (!ret)
  {
    return;
  }
  qSlicerApplication::application()->ioManager()->registerDialog(fileDialog.take());
}

//-----------------------------------------------------------------------------
void qSlicerScriptedLoadableModule::registerIO()
{
  Q_D(qSlicerScriptedLoadableModule);
  vtkSlicerApplicationLogic* appLogic = this->appLogic();
  vtkMRMLFileIOManager* fileIOManager = appLogic ? appLogic->GetFileIOManager() : nullptr;
  if (!fileIOManager)
  {
    return;
  }
  if (!Py_IsInitialized())
  {
    return;
  }
  // Readers and writers that were registered previously (if the module is set up again) are replaced
  d->unregisterFileIOHandlers();
  // Register <ModuleName>FileWriter and <ModuleName>FileReader Python classes (if they exist).
  // The registered readers and writers are stored so that they can be unregistered when the module is deleted.
  PythonQtObjectPtr scriptedFileIOModule;
  scriptedFileIOModule.setNewRef(PyImport_ImportModule("slicer.ScriptedFileIO"));
  PythonQtObjectPtr registerFunction;
  if (scriptedFileIOModule)
  {
    registerFunction.setNewRef(PyObject_GetAttrString(scriptedFileIOModule, "registerScriptedFileIO"));
  }
  PythonQtObjectPtr result;
  if (registerFunction)
  {
    PythonQtObjectPtr pyFileIOManager;
    pyFileIOManager.setNewRef(vtkPythonUtil::GetObjectFromPointer(fileIOManager));
    PythonQtObjectPtr pyModuleName;
    pyModuleName.setNewRef(PyUnicode_FromString(this->name().toUtf8().constData()));
    result.setNewRef(PyObject_CallFunctionObjArgs(registerFunction, pyModuleName.object(), pyFileIOManager.object(), nullptr));
  }
  d->RegisteredFileIOManager = fileIOManager;
  if (!result || !PyList_Check(result.object()))
  {
    qCritical() << Q_FUNC_INFO << ": failed to register file readers and writers of module" << this->name();
    PythonQt::self()->handleError();
    return;
  }
  for (Py_ssize_t index = 0; index < PyList_Size(result.object()); ++index)
  {
    vtkMRMLFileIOHandler* handler = vtkMRMLFileIOHandler::SafeDownCast( //
      vtkPythonUtil::GetPointerFromObject(PyList_GetItem(result.object(), index), "vtkMRMLFileIOHandler"));
    if (!handler)
    {
      qCritical() << Q_FUNC_INFO << ": invalid file reader or writer returned for module" << this->name();
      PythonQt::self()->handleError();
      continue;
    }
    d->RegisteredFileIOHandlers.emplace_back(handler);
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
  bool ret = widget->setPythonSource(d->PythonSourceFilePath);
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
