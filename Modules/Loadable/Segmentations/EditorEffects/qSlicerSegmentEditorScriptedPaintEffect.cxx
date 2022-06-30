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

==============================================================================*/

// SubjectHierarchy includes
#include "qSlicerSegmentEditorScriptedPaintEffect.h"

// Qt includes
#include <QDebug>
#include <QFileInfo>

// Slicer includes
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
class qSlicerSegmentEditorScriptedPaintEffectPrivate
{
public:
  typedef qSlicerSegmentEditorScriptedPaintEffectPrivate Self;
  qSlicerSegmentEditorScriptedPaintEffectPrivate();
  virtual ~qSlicerSegmentEditorScriptedPaintEffectPrivate();

  enum {
    IconMethod = 0,
    HelpTextMethod,
    CloneMethod,
    ActivateMethod,
    DeactivateMethod,
    SetupOptionsFrameMethod,
    CreateCursorMethod,
    SetMRMLDefaultsMethod,
    ReferenceGeometryChangedMethod,
    MasterVolumeNodeChangedMethod,
    SourceVolumeNodeChangedMethod,
    LayoutChangedMethod,
    UpdateGUIFromMRMLMethod,
    UpdateMRMLFromGUIMethod,
    PaintApplyMethod,
    };

  mutable qSlicerPythonCppAPI PythonCppAPI;

  QString PythonSource;
};

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorScriptedPaintEffectPrivate methods

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScriptedPaintEffectPrivate::qSlicerSegmentEditorScriptedPaintEffectPrivate()
{
  this->PythonCppAPI.declareMethod(Self::IconMethod, "icon");
  this->PythonCppAPI.declareMethod(Self::HelpTextMethod, "helpText");
  this->PythonCppAPI.declareMethod(Self::CloneMethod, "clone");
  this->PythonCppAPI.declareMethod(Self::ActivateMethod, "activate");
  this->PythonCppAPI.declareMethod(Self::DeactivateMethod, "deactivate");
  this->PythonCppAPI.declareMethod(Self::SetupOptionsFrameMethod, "setupOptionsFrame");
  this->PythonCppAPI.declareMethod(Self::CreateCursorMethod, "createCursor");
  this->PythonCppAPI.declareMethod(Self::SetMRMLDefaultsMethod, "setMRMLDefaults");
  this->PythonCppAPI.declareMethod(Self::ReferenceGeometryChangedMethod, "referenceGeometryChanged");
  this->PythonCppAPI.declareMethod(Self::SourceVolumeNodeChangedMethod, "sourceVolumeNodeChanged");
  this->PythonCppAPI.declareMethod(Self::MasterVolumeNodeChangedMethod, "masterVolumeNodeChanged");
  this->PythonCppAPI.declareMethod(Self::LayoutChangedMethod, "layoutChanged");
  this->PythonCppAPI.declareMethod(Self::UpdateGUIFromMRMLMethod, "updateGUIFromMRML");
  this->PythonCppAPI.declareMethod(Self::UpdateMRMLFromGUIMethod, "updateMRMLFromGUI");
  this->PythonCppAPI.declareMethod(Self::PaintApplyMethod, "paintApply");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScriptedPaintEffectPrivate::~qSlicerSegmentEditorScriptedPaintEffectPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorScriptedPaintEffect methods

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScriptedPaintEffect::qSlicerSegmentEditorScriptedPaintEffect(QObject *parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSegmentEditorScriptedPaintEffectPrivate)
{
  this->m_Name = QString("UnnamedScriptedPaintEffect");
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorScriptedPaintEffect::~qSlicerSegmentEditorScriptedPaintEffect() = default;

//-----------------------------------------------------------------------------
QString qSlicerSegmentEditorScriptedPaintEffect::pythonSource()const
{
  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  return d->PythonSource;
}

//-----------------------------------------------------------------------------
bool qSlicerSegmentEditorScriptedPaintEffect::setPythonSource(const QString newPythonSource)
{
  Q_D(qSlicerSegmentEditorScriptedPaintEffect);

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

  // In case the effect is within the main module file
  QString className = moduleName;
  if (!moduleName.endsWith("Effect"))
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
                    QString("qSlicerSegmentEditorScriptedPaintEffect::setPythonSource - "
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
        "slicer", className, self))
    {
    qCritical() << "Failed to set" << ("slicer." + className);
    }

  return true;
}

//-----------------------------------------------------------------------------
PyObject* qSlicerSegmentEditorScriptedPaintEffect::self() const
{
  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  return d->PythonCppAPI.pythonSelf();
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::setName(QString name)
{
  this->m_Name = name;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSegmentEditorScriptedPaintEffect::icon()
{
  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
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
const QString qSlicerSegmentEditorScriptedPaintEffect::helpText()const
{
  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  PyObject* result = d->PythonCppAPI.callMethod(d->HelpTextMethod);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    return this->Superclass::helpText();
    }

  // Parse result
  if (!PyString_Check(result))
    {
    qWarning() << d->PythonSource << ": qSlicerSegmentEditorScriptedPaintEffect: Function 'helpText' is expected to return a string!";
    return this->Superclass::helpText();
    }

  const char* role = PyString_AsString(result);
  return QString::fromLocal8Bit(role);
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qSlicerSegmentEditorScriptedPaintEffect::clone()
{
  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
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
void qSlicerSegmentEditorScriptedPaintEffect::activate()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::activate();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->ActivateMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::deactivate()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::deactivate();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->DeactivateMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::setupOptionsFrame()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::setupOptionsFrame();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->SetupOptionsFrameMethod);
}

//-----------------------------------------------------------------------------
QCursor qSlicerSegmentEditorScriptedPaintEffect::createCursor(qMRMLWidget* viewWidget)
{
  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
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
void qSlicerSegmentEditorScriptedPaintEffect::setMRMLDefaults()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::setMRMLDefaults();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->SetMRMLDefaultsMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::referenceGeometryChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::referenceGeometryChanged();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->ReferenceGeometryChangedMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::sourceVolumeNodeChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::sourceVolumeNodeChanged();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->SourceVolumeNodeChangedMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::masterVolumeNodeChanged()
{
  // Note: deprecated
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::masterVolumeNodeChanged();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->MasterVolumeNodeChangedMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::layoutChanged()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::layoutChanged();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->LayoutChangedMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::updateGUIFromMRML()
{
  if (!this->active())
    {
    // updateGUIFromMRML is called when the effect is activated
    return;
    }

  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::updateGUIFromMRML();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->UpdateGUIFromMRMLMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::updateMRMLFromGUI()
{
  // Base class implementation needs to be called before the effect-specific one
  this->Superclass::updateMRMLFromGUI();

  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  d->PythonCppAPI.callMethod(d->UpdateMRMLFromGUIMethod);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorScriptedPaintEffect::paintApply(qMRMLWidget* viewWidget)
{
  Q_D(const qSlicerSegmentEditorScriptedPaintEffect);
  PyObject* arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PythonQtConv::QVariantToPyObject(QVariant::fromValue<QObject*>((QObject*)viewWidget)));
  PyObject* result = d->PythonCppAPI.callMethod(d->PaintApplyMethod, arguments);
  Py_DECREF(arguments);
  if (!result)
    {
    // Method call failed (probably an omitted function), call default implementation
    this->Superclass::paintApply(viewWidget);
    }
}
