/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyScriptedPlugin.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QAction>

// SlicerQt includes
#include "qSlicerScriptedUtils_p.h"

// PythonQt includes
#include <PythonQt.h>
#include <PythonQtConversion.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPythonUtil.h>

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyScriptedPluginPrivate
{
public:
  typedef qSlicerSubjectHierarchyScriptedPluginPrivate Self;
  qSlicerSubjectHierarchyScriptedPluginPrivate();
  virtual ~qSlicerSubjectHierarchyScriptedPluginPrivate();

  enum {
    CanOwnSubjectHierarchyNodeMethod = 0,
    RoleForPluginMethod,
    HelpTextMethod,
    IconMethod,
    VisibilityIconMethod,
    EditPropertiesMethod,
    DisplayedNodeNameMethod,
    TooltipMethod,
    SetDisplayVisibilityMethod,
    GetDisplayVisibilityMethod,
    NodeContextMenuActionsMethod,
    SceneContextMenuActionsMethod,
    ShowContextMenuActionsForNodeMethod,
    CanAddNodeToSubjectHierarchyMethod,
    CanReparentNodeInsideSubjectHierarchyMethod,
    ReparentNodeInsideSubjectHierarchyMethod
    };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString PythonSource;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyScriptedPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyScriptedPluginPrivate::qSlicerSubjectHierarchyScriptedPluginPrivate()
{
  // Role-related methods
  this->PythonCppAPI.declareMethod(Self::CanOwnSubjectHierarchyNodeMethod, "canOwnSubjectHierarchyNode");
  this->PythonCppAPI.declareMethod(Self::RoleForPluginMethod, "roleForPlugin");
  this->PythonCppAPI.declareMethod(Self::HelpTextMethod, "helpText");
  this->PythonCppAPI.declareMethod(Self::IconMethod, "icon");
  this->PythonCppAPI.declareMethod(Self::VisibilityIconMethod, "visibilityIcon");
  this->PythonCppAPI.declareMethod(Self::EditPropertiesMethod, "editProperties");
  this->PythonCppAPI.declareMethod(Self::DisplayedNodeNameMethod, "displayedNodeName");
  this->PythonCppAPI.declareMethod(Self::TooltipMethod, "tooltip");
  this->PythonCppAPI.declareMethod(Self::SetDisplayVisibilityMethod, "setDisplayVisibility");
  this->PythonCppAPI.declareMethod(Self::GetDisplayVisibilityMethod, "getDisplayVisibility");
  // Function related methods
  this->PythonCppAPI.declareMethod(Self::NodeContextMenuActionsMethod, "nodeContextMenuActions");
  this->PythonCppAPI.declareMethod(Self::SceneContextMenuActionsMethod, "sceneContextMenuActions");
  this->PythonCppAPI.declareMethod(Self::ShowContextMenuActionsForNodeMethod, "showContextMenuActionsForNode");
  // Parenting related methods (with default implementation)
  this->PythonCppAPI.declareMethod(Self::CanAddNodeToSubjectHierarchyMethod, "canAddNodeToSubjectHierarchy");
  this->PythonCppAPI.declareMethod(Self::CanReparentNodeInsideSubjectHierarchyMethod, "canReparentNodeInsideSubjectHierarchy");
  this->PythonCppAPI.declareMethod(Self::ReparentNodeInsideSubjectHierarchyMethod, "reparentNodeInsideSubjectHierarchy");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyScriptedPluginPrivate::~qSlicerSubjectHierarchyScriptedPluginPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyScriptedPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyScriptedPlugin::qSlicerSubjectHierarchyScriptedPlugin(QObject *parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSubjectHierarchyScriptedPluginPrivate)
{
  this->m_Name = QString("UnnamedScriptedPlugin");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyScriptedPlugin::~qSlicerSubjectHierarchyScriptedPlugin()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyScriptedPlugin::pythonSource()const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyScriptedPlugin::setPythonSource(const QString newPythonSource)
{
  Q_D(qSlicerSubjectHierarchyScriptedPlugin);

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

  // In case the plugin is within the main module file
  QString className = moduleName;
  if (!moduleName.endsWith("SubjectHierarchyPlugin"))
    {
    className.append("SubjectHierarchyPlugin");
    }

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Get a reference (or create if needed) the <moduleName> python module
  PyObject * module = PyImport_AddModule(moduleName.toLatin1());

  // Get a reference to the python module class to instantiate
  PythonQtObjectPtr classToInstantiate;
  if (PyObject_HasAttrString(module, className.toLatin1()))
    {
    classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toLatin1()));
    }
  if (!classToInstantiate)
    {
    PythonQtObjectPtr local_dict;
    local_dict.setNewRef(PyDict_New());
    if (!qSlicerScriptedUtils::loadSourceAsModule(moduleName, newPythonSource, global_dict, local_dict))
      {
      return false;
      }
    if (PyObject_HasAttrString(module, className.toLatin1()))
      {
      classToInstantiate.setNewRef(PyObject_GetAttrString(module, className.toLatin1()));
      }
    }

  if (!classToInstantiate)
    {
    PythonQt::self()->handleError();
    PyErr_SetString(PyExc_RuntimeError,
                    QString("qSlicerSubjectHierarchyScriptedPlugin::setPythonSource - "
                            "Failed to load subject hierarchy scripted plugin: "
                            "class %1 was not found in %2").arg(className).arg(newPythonSource).toLatin1());
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
        "slicer", className, self))
    {
    qCritical() << "Failed to set" << ("slicer." + className);
    }

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerSubjectHierarchyScriptedPlugin::self() const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyScriptedPlugin::setName(QString name)
{
  this->m_Name = name;
}

//-----------------------------------------------------------------------------
double qSlicerSubjectHierarchyScriptedPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyObject* result = d->PythonCppAPI.callMethod(d->CanOwnSubjectHierarchyNodeMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::canOwnSubjectHierarchyNode(node);
    }

  // Parse result
  if (!PyFloat_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'canOwnSubjectHierarchyNode' is expected to return a floating point number!";
    return this->Superclass::canOwnSubjectHierarchyNode(node);
    }

  return PyFloat_AsDouble(result);
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyScriptedPlugin::roleForPlugin()const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* result = d->PythonCppAPI.callMethod(d->RoleForPluginMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::roleForPlugin();
    }

  // Parse result
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'roleForPlugin' is expected to return a string!";
    return this->Superclass::roleForPlugin();
    }

  const char* role = PyString_AsString(result);
  return QString::fromLocal8Bit(role);
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyScriptedPlugin::helpText()const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* result = d->PythonCppAPI.callMethod(d->HelpTextMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::helpText();
    }

  // Parse result
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'helpText' is expected to return a string!";
    return this->Superclass::helpText();
    }

  const char* role = PyString_AsString(result);
  return QString::fromLocal8Bit(role);
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyScriptedPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyObject* result = d->PythonCppAPI.callMethod(d->IconMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::icon(node);
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result, QVariant::Icon);
  if (resultVariant.isNull())
    {
    return this->Superclass::icon(node);
    }
  return resultVariant.value<QIcon>();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyScriptedPlugin::visibilityIcon(int visible)
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PyInt_FromLong(visible));
  PyObject* result = d->PythonCppAPI.callMethod(d->VisibilityIconMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::visibilityIcon(visible);
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result, QVariant::Icon);
  if (resultVariant.isNull())
    {
    return this->Superclass::visibilityIcon(visible);
    }
  return resultVariant.value<QIcon>();
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyScriptedPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyObject* result = d->PythonCppAPI.callMethod(d->EditPropertiesMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    this->Superclass::editProperties(node);
    }
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyScriptedPlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* result = d->PythonCppAPI.callMethod(d->NodeContextMenuActionsMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::nodeContextMenuActions();
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result, QVariant::List);
  if (resultVariant.isNull())
    {
    return this->Superclass::nodeContextMenuActions();
    }
  QList<QVariant> resultVariantList = resultVariant.toList();
  QList<QAction*> actionList;
  foreach(QVariant actionVariant, resultVariantList)
    {
    QAction* action = qobject_cast<QAction*>( actionVariant.value<QObject*>() );
    actionList << action;
    }
  return actionList;
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyScriptedPlugin::sceneContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* result = d->PythonCppAPI.callMethod(d->SceneContextMenuActionsMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::sceneContextMenuActions();
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result, QVariant::List);
  if (resultVariant.isNull())
    {
    return this->Superclass::sceneContextMenuActions();
    }
  QList<QVariant> resultVariantList = resultVariant.toList();
  QList<QAction*> actionList;
  foreach(QVariant actionVariant, resultVariantList)
    {
    QAction* action = qobject_cast<QAction*>( actionVariant.value<QObject*>() );
    actionList << action;
    }
  return actionList;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyScriptedPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyScriptedPlugin);

  // Hide all actions before showing them based on node
  this->hideAllContextMenuActions();

  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyObject* result = d->PythonCppAPI.callMethod(d->ShowContextMenuActionsForNodeMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    this->Superclass::showContextMenuActionsForNode(node);
    }
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyScriptedPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node,
                                                                           vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(2);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyTuple_SET_ITEM(arguments, 1, vtkPythonUtil::GetObjectFromPointer(parent));
  PyObject* result = d->PythonCppAPI.callMethod(d->CanAddNodeToSubjectHierarchyMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::canAddNodeToSubjectHierarchy(node, parent);
    }

  // Parse result
  if (!PyFloat_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'canAddNodeToSubjectHierarchy' is expected to return a floating point number!";
    return this->Superclass::canAddNodeToSubjectHierarchy(node, parent);
    }

  return PyFloat_AsDouble(result);
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyScriptedPlugin::canReparentNodeInsideSubjectHierarchy(vtkMRMLSubjectHierarchyNode* node,
                                                                                    vtkMRMLSubjectHierarchyNode* parent)const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(2);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyTuple_SET_ITEM(arguments, 1, vtkPythonUtil::GetObjectFromPointer(parent));
  PyObject* result = d->PythonCppAPI.callMethod(d->CanReparentNodeInsideSubjectHierarchyMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::canReparentNodeInsideSubjectHierarchy(node, parent);
    }

  // Parse result
  if (!PyFloat_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'canReparentNodeInsideSubjectHierarchy' is expected to return a floating point number!";
    return this->Superclass::canReparentNodeInsideSubjectHierarchy(node, parent);
    }

  return PyFloat_AsDouble(result);
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyScriptedPlugin::reparentNodeInsideSubjectHierarchy(vtkMRMLSubjectHierarchyNode* nodeToReparent,
                                                                               vtkMRMLSubjectHierarchyNode* parentNode)
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(2);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(nodeToReparent));
  PyTuple_SET_ITEM(arguments, 1, vtkPythonUtil::GetObjectFromPointer(parentNode));
  PyObject* result = d->PythonCppAPI.callMethod(d->ReparentNodeInsideSubjectHierarchyMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::reparentNodeInsideSubjectHierarchy(nodeToReparent, parentNode);
    }

  // Parse result
  if (!PyBool_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'reparentNodeInsideSubjectHierarchy' is expected to return a boolean!";
    return this->Superclass::reparentNodeInsideSubjectHierarchy(nodeToReparent, parentNode);
    }

  return result == Py_True;
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyScriptedPlugin::displayedNodeName(vtkMRMLSubjectHierarchyNode* node)const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyObject* result = d->PythonCppAPI.callMethod(d->DisplayedNodeNameMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::displayedNodeName(node);
    }

  // Parse result
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'displayedNodeName' is expected to return a string!";
    return this->Superclass::displayedNodeName(node);
    }

  return PyString_AsString(result);
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyScriptedPlugin::tooltip(vtkMRMLSubjectHierarchyNode* node)const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyObject* result = d->PythonCppAPI.callMethod(d->TooltipMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::tooltip(node);
    }

  // Parse result
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'tooltip' is expected to return a string!";
    return this->Superclass::tooltip(node);
    }

  return PyString_AsString(result);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyScriptedPlugin::setDisplayVisibility(vtkMRMLSubjectHierarchyNode* node, int visible)
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(2);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(visible));
  PyObject* result = d->PythonCppAPI.callMethod(d->SetDisplayVisibilityMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    this->Superclass::setDisplayVisibility(node, visible);
    }
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyScriptedPlugin::getDisplayVisibility(vtkMRMLSubjectHierarchyNode* node)const
{
  Q_D(const qSlicerSubjectHierarchyScriptedPlugin);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(node));
  PyObject* result = d->PythonCppAPI.callMethod(d->GetDisplayVisibilityMethod, arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::getDisplayVisibility(node);
    }

  // Parse result
  if (!PyInt_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSubjectHierarchyScriptedPlugin: Function 'getDisplayVisibility' is expected to return an integer!";
    return this->Superclass::getDisplayVisibility(node);
    }

  return (int)PyInt_AsLong(result);
}
