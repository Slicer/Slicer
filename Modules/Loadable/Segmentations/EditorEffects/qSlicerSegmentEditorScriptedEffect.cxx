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
#include "qSlicerSegmentEditorScriptedEffect.h"

// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileInfo>

// Slicer includes
#include "qSlicerScriptedUtils_p.h"
#include "qSlicerUtils.h"

// PythonQt includes
#include <PythonQt.h>
#include <PythonQtConversion.h>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkPythonUtil.h>

//-----------------------------------------------------------------------------
class qSlicerSegmentEditorScriptedEffectPrivate
{
public:
  typedef qSlicerSegmentEditorScriptedEffectPrivate Self;
  qSlicerSegmentEditorScriptedEffectPrivate();
  virtual ~qSlicerSegmentEditorScriptedEffectPrivate();

  enum {
    IconMethod = 0,
    HelpTextMethod,
    CloneMethod,
    ActivateMethod,
    DeactivateMethod,
    SetupOptionsFrameMethod,
    CreateCursorMethod,
    ProcessInteractionEventsMethod,
    ProcessViewNodeEventsMethod,
    SetMRMLDefaultsMethod,
    ReferenceGeometryChangedMethod,
    SourceVolumeNodeChangedMethod,
    MasterVolumeNodeChangedMethod, // deprecated
    LayoutChangedMethod,
    InteractionNodeModifiedMethod,
    UpdateGUIFromMRMLMethod,
    UpdateMRMLFromGUIMethod,
    };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString PythonSource;
};

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorScriptedEffectPrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScriptedEffectPrivate::qSlicerSegmentEditorScriptedEffectPrivate()
{
  this->PythonCppAPI.declareMethod(Self::IconMethod, "icon");
  this->PythonCppAPI.declareMethod(Self::HelpTextMethod, "helpText");
  this->PythonCppAPI.declareMethod(Self::CloneMethod, "clone");
  this->PythonCppAPI.declareMethod(Self::ActivateMethod, "activate");
  this->PythonCppAPI.declareMethod(Self::DeactivateMethod, "deactivate");
  this->PythonCppAPI.declareMethod(Self::SetupOptionsFrameMethod, "setupOptionsFrame");
  this->PythonCppAPI.declareMethod(Self::CreateCursorMethod, "createCursor");
  this->PythonCppAPI.declareMethod(Self::ProcessInteractionEventsMethod, "processInteractionEvents");
  this->PythonCppAPI.declareMethod(Self::ProcessViewNodeEventsMethod, "processViewNodeEvents");
  this->PythonCppAPI.declareMethod(Self::SetMRMLDefaultsMethod, "setMRMLDefaults");
  this->PythonCppAPI.declareMethod(Self::ReferenceGeometryChangedMethod, "referenceGeometryChanged");
  this->PythonCppAPI.declareMethod(Self::SourceVolumeNodeChangedMethod, "sourceVolumeNodeChanged");
  this->PythonCppAPI.declareMethod(Self::MasterVolumeNodeChangedMethod, "masterVolumeNodeChanged");
  this->PythonCppAPI.declareMethod(Self::LayoutChangedMethod, "layoutChanged");
  this->PythonCppAPI.declareMethod(Self::InteractionNodeModifiedMethod, "interactionNodeModified");
  this->PythonCppAPI.declareMethod(Self::UpdateGUIFromMRMLMethod, "updateGUIFromMRML");
  this->PythonCppAPI.declareMethod(Self::UpdateMRMLFromGUIMethod, "updateMRMLFromGUI");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScriptedEffectPrivate::~qSlicerSegmentEditorScriptedEffectPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorScriptedEffect methods

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScriptedEffect::qSlicerSegmentEditorScriptedEffect(QObject *parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSegmentEditorScriptedEffectPrivate)
{
  this->m_Name = QString("UnnamedScriptedEffect");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScriptedEffect::~qSlicerSegmentEditorScriptedEffect() = default;

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorScriptedEffect::pythonSource()const
{
  Q_D(const qSlicerSegmentEditorScriptedEffect);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorScriptedEffect::setPythonSource(const QString newPythonSource)
{
  Q_D(qSlicerSegmentEditorScriptedEffect);

  if (!Py_IsInitialized())
    {
    return false;
    }

  if (!newPythonSource.endsWith(".py") && !newPythonSource.endsWith(".pyc"))
    {
    return false;
    }

  // Use parent directory name as module name
  QDir sourceDir(newPythonSource);
  sourceDir.cdUp();
  QString moduleName = sourceDir.dirName();

  // Use filename as class name
  QString className = QFileInfo(newPythonSource).baseName();
  if (!className.endsWith("Effect"))
    {
    className.append("Effect");
    }

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Get a reference (or create if needed) the <moduleName> python module
  PyObject * module = PyImport_AddModule(moduleName.toUtf8());

  // Get a reference to the python module class to instantiate
  PythonQtObjectPtr classToInstantiate;
  if (PyObject_HasAttrString(module, className.toUtf8()))
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
                    QString("qSlicerSegmentEditorScriptedEffect::setPythonSource - "
                            "Failed to load segment editor scripted effect: "
                            "class %1 was not found in %2").arg(className).arg(newPythonSource).toUtf8());
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
        "slicer", moduleName, self))
    {
    qCritical() << "Failed to set" << ("slicer." + moduleName);
    }

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerSegmentEditorScriptedEffect::self() const
{
  Q_D(const qSlicerSegmentEditorScriptedEffect);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::setName(QString name)
{
  this->m_Name = name;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::setPerSegment(bool perSegment)
{
  this->m_PerSegment = perSegment;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::setRequireSegments(bool requireSegments)
{
  this->m_RequireSegments = requireSegments;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSegmentEditorScriptedEffect::icon()
{
  Q_D(const qSlicerSegmentEditorScriptedEffect);
  PyObject* result = d->PythonCppAPI.callMethod(d->IconMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::icon();
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result, QVariant::Icon);
  if (resultVariant.isNull())
    {
    return this->Superclass::icon();
    }
  return resultVariant.value<QIcon>();
}

//-----------------------------------------------------------------------------
const QString qSlicerSegmentEditorScriptedEffect::helpText()const
{
  Q_D(const qSlicerSegmentEditorScriptedEffect);
  PyObject* result = d->PythonCppAPI.callMethod(d->HelpTextMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::helpText();
    }

  // Parse result
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSegmentEditorScriptedEffect: Function 'helpText' is expected to return a string!";
    return this->Superclass::helpText();
    }

  const char* role = PyString_AsString(result);
  return QString::fromLocal8Bit(role);
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qSlicerSegmentEditorScriptedEffect::clone()
{
  Q_D(const qSlicerSegmentEditorScriptedEffect);
  PyObject* result = d->PythonCppAPI.callMethod(d->CloneMethod);
  if (!result)
    {
    qCritical() << d->PythonSource << ": clone: Failed to call mandatory clone method! If it is implemented, please see python output for errors.";
    return nullptr;
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result);
  qSlicerSegmentEditorAbstractEffect* clonedEffect = qobject_cast<qSlicerSegmentEditorAbstractEffect*>(
    resultVariant.value<QObject*>() );
  if (!clonedEffect)
    {
    qCritical() << d->PythonSource << ": clone: Invalid cloned effect object returned from python!";
    return nullptr;
    }
  return clonedEffect;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::activate()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::activate();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->ActivateMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::deactivate()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::deactivate();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->DeactivateMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::setupOptionsFrame()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::setupOptionsFrame();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->SetupOptionsFrameMethod);
}

//-----------------------------------------------------------------------------
QCursor qSlicerSegmentEditorScriptedEffect::createCursor(qMRMLWidget* viewWidget)
{
  Q_D(const qSlicerSegmentEditorScriptedEffect);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PythonQtConv::QVariantToPyObject(QVariant::fromValue<QObject*>((QObject*)viewWidget)));
  PyObject* result = d->PythonCppAPI.callMethod(d->CreateCursorMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::createCursor(viewWidget);
    }

  // Parse result
  QVariant resultVariant = PythonQtConv::PyObjToQVariant(result, QVariant::Cursor);
  return resultVariant.value<QCursor>();
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorScriptedEffect::processInteractionEvents(vtkRenderWindowInteractor* callerInteractor, unsigned long eid, qMRMLWidget* viewWidget)
{
  Q_D(const qSlicerSegmentEditorScriptedEffect);
  PyObject* arguments = PyTuple_New(3);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer((vtkObject*)callerInteractor));
  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(eid));
  PyTuple_SET_ITEM(arguments, 2, PythonQtConv::QVariantToPyObject(QVariant::fromValue<QObject*>((QObject*)viewWidget)));
  PyObject* result = d->PythonCppAPI.callMethod(d->ProcessInteractionEventsMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::processInteractionEvents(callerInteractor, eid, viewWidget);
    }
  if (!PyBool_Check(result))
    {
    qWarning() << d->PythonSource
               << " - function 'processInteractionEvents' "
               << "is expected to return a boolean";
    return false;
    }
  return result == Py_True;
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::processViewNodeEvents(vtkMRMLAbstractViewNode* callerViewNode, unsigned long eid, qMRMLWidget* viewWidget)
{
  Q_D(const qSlicerSegmentEditorScriptedEffect);
  PyObject* arguments = PyTuple_New(3);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer((vtkObject*)callerViewNode));
  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(eid));
  PyTuple_SET_ITEM(arguments, 2, PythonQtConv::QVariantToPyObject(QVariant::fromValue<QObject*>((QObject*)viewWidget)));
  PyObject* result = d->PythonCppAPI.callMethod(d->ProcessViewNodeEventsMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::processViewNodeEvents(callerViewNode, eid, viewWidget);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::setMRMLDefaults()
{
  // Base class implementation needs to be called before the effect-specific one
  // Note: Left here as comment in case this class is used as template for adaptor
  //  classes of effect base classes that have default implementation of this method
  //  (such as LabelEffect, etc.)
  //this->Superclass::setMRMLDefaults();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->SetMRMLDefaultsMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::referenceGeometryChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::referenceGeometryChanged();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->ReferenceGeometryChangedMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::sourceVolumeNodeChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::sourceVolumeNodeChanged();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->SourceVolumeNodeChangedMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::masterVolumeNodeChanged()
{
  // Note: deprecated
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::masterVolumeNodeChanged();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->MasterVolumeNodeChangedMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::layoutChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::layoutChanged();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->LayoutChangedMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::interactionNodeModified(vtkMRMLInteractionNode* interactionNode)
{
  // Do not call base class implementation by default.
  // This way the effect may decide to not deactivate itself when markups place mode
  // is activated.

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer((vtkObject*)interactionNode));
  PyObject* result = d->PythonCppAPI.callMethod(d->InteractionNodeModifiedMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    this->Superclass::interactionNodeModified(interactionNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::updateGUIFromMRML()
{
  if (!this->active())
  {
    // updateGUIFromMRML is called when the effect is activated
    return;
  }

  // Base class implementation needs to be called before the effect-specific one
  // Note: Left here as comment in case this class is used as template for adaptor
  //  classes of effect base classes that have default implementation of this method
  //  (such as LabelEffect, etc.)
  //this->Superclass::updateGUIFromMRML();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->UpdateGUIFromMRMLMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedEffect::updateMRMLFromGUI()
{
  // Base class implementation needs to be called before the effect-specific one
  // Note: Left here as comment in case this class is used as template for adaptor
  //  classes of effect base classes that have default implementation of this method
  //  (such as LabelEffect, etc.)
  //this->Superclass::updateMRMLFromGUI();

  Q_D(const qSlicerSegmentEditorScriptedEffect);
  d->PythonCppAPI.callMethod(d->UpdateMRMLFromGUIMethod);
}
