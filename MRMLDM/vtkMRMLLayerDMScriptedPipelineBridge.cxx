#include "vtkMRMLLayerDMScriptedPipelineBridge.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineManager.h"

// Slicer includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkPythonUtil.h>
#include <vtkRenderer.h>
#include <vtkSmartPyObject.h>

vtkStandardNewMacro(vtkMRMLLayerDMScriptedPipelineBridge);

inline PyObject* ToPyObject(vtkObjectBase* obj)
{
  return vtkPythonUtil::GetObjectFromPointer(obj);
}

inline PyObject* ToPyObject(unsigned long value)
{
  return PyLong_FromUnsignedLong(value);
}

inline PyObject* RawPtrToPython(void* ptr)
{
  if (ptr)
  {
    return PyCapsule_New(ptr, nullptr, nullptr);
  }

  // Return borrowed reference to Py_None
  Py_INCREF(Py_None);
  return Py_None;
}

vtkSmartPyObject ToPyArgs(const std::vector<PyObject*>& pyObjs)
{
  if (pyObjs.empty())
  {
    return {};
  }

  // Pack into a Python tuple and transfer ownership
  PyObject* pyTuple = PyTuple_New(pyObjs.size());
  for (size_t i = 0; i < (pyObjs.size()); ++i)
  {
    PyTuple_SET_ITEM(pyTuple, i, pyObjs[i]);
  }

  return { pyTuple };
}

vtkSmartPyObject ToPyArgs(vtkObjectBase* obj)
{
  return ToPyArgs({ ToPyObject(obj) });
}

vtkSmartPyObject ToPyArgs(vtkObject* obj, unsigned long eventId, void* callData)
{
  return ToPyArgs({ ToPyObject(obj), ToPyObject(eventId), RawPtrToPython(callData) });
}

void vtkMRMLLayerDMScriptedPipelineBridge::UpdatePipeline()
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod({}, __func__);
}

PyObject* vtkMRMLLayerDMScriptedPipelineBridge::CastCallData(PyObject* object, int vtkType)
{
  if (!Py_IsInitialized())
  {
    return nullptr;
  }

  if (!PyCapsule_CheckExact(object))
  {
    PyErr_SetString(PyExc_TypeError, "Expected a PyCapsule object");
    return nullptr;
  }

  void* ptr = PyCapsule_GetPointer(object, nullptr);
  if (!ptr)
  {
    PyErr_SetString(PyExc_NotImplementedError, "Invalid call data object");
    return nullptr;
  }

  switch (vtkType)
  {
    case VTK_INT:
    {
      int value = *static_cast<int*>(ptr);
      return PyLong_FromLong(static_cast<long>(value));
    }
    case VTK_LONG:
    {
      long value = *static_cast<long*>(ptr);
      return PyLong_FromLong(value);
    }
    case VTK_FLOAT:
    {
      float value = *static_cast<float*>(ptr);
      return PyFloat_FromDouble(static_cast<double>(value));
    }
    case VTK_DOUBLE:
    {
      double value = *static_cast<double*>(ptr);
      return PyFloat_FromDouble(value);
    }
    case VTK_STRING:
    {
      const char* value = *static_cast<const char**>(ptr);
      return PyUnicode_FromString(value);
    }
    case VTK_OBJECT:
    {
      if (vtkObject* obj = vtkObject::SafeDownCast(static_cast<vtkObject*>(ptr)))
      {
        return ToPyObject(obj);
      }
      PyErr_SetString(PyExc_NotImplementedError, "Invalid VTK object");
      return nullptr;
    }
    default:
    {
      PyErr_SetString(PyExc_ValueError, "Unknown vtkType. Expected one of : [VTK_INT, VTK_LONG, VTK_FLOAT, VTK_DOUBLE, VTK_STRING, VTK_OBJECT]");
      return nullptr;
    }
  }
}

vtkMRMLLayerDMScriptedPipelineBridge::vtkMRMLLayerDMScriptedPipelineBridge()
  : m_object{ nullptr }
{
}

vtkMRMLLayerDMScriptedPipelineBridge::~vtkMRMLLayerDMScriptedPipelineBridge()
{
  if (Py_IsInitialized())
  {
    vtkPythonScopeGilEnsurer gilEnsurer;
    Py_XDECREF(this->m_object);
  }
}

bool vtkMRMLLayerDMScriptedPipelineBridge::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  if (!Py_IsInitialized())
  {
    return false;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod(ToPyArgs(eventData), __func__))
  {
    int canProcess;
    if (PyTuple_Check(result) && PyArg_ParseTuple(result, "pd", &canProcess, &distance2))
    {
      return canProcess;
    }

    // Unpack error or unexpected return type
    PyErr_SetString(PyExc_TypeError, "Expected a tuple[bool, float] return type");
    PyErr_Print();
  }

  return false;
}

vtkCamera* vtkMRMLLayerDMScriptedPipelineBridge::GetCamera() const
{
  if (!Py_IsInitialized())
  {
    return Superclass::GetCamera();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  auto result = this->CallPythonMethod({}, __func__);
  if (result && (result != Py_None))
  {
    return vtkCamera::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkCamera"));
  }
  return Superclass::GetCamera();
}

int vtkMRMLLayerDMScriptedPipelineBridge::GetMouseCursor() const
{
  if (!Py_IsInitialized())
  {
    return Superclass::GetMouseCursor();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod({}, __func__))
  {
    return PyLong_AsLong(result);
  }
  return Superclass::GetMouseCursor();
}

unsigned int vtkMRMLLayerDMScriptedPipelineBridge::GetRenderLayer() const
{
  if (!Py_IsInitialized())
  {
    return Superclass::GetRenderLayer();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod({}, __func__))
  {
    return PyLong_AsLong(result);
  }
  return Superclass::GetRenderLayer();
}

int vtkMRMLLayerDMScriptedPipelineBridge::GetWidgetState() const
{
  if (!Py_IsInitialized())
  {
    return Superclass::GetWidgetState();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod({}, __func__))
  {
    return PyLong_AsLong(result);
  }
  return Superclass::GetWidgetState();
}

void vtkMRMLLayerDMScriptedPipelineBridge::LoseFocus(vtkMRMLInteractionEventData* eventData)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(eventData), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnDefaultCameraModified(vtkCamera* camera)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(camera), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnRendererAdded(vtkRenderer* renderer)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(renderer), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnRendererRemoved(vtkRenderer* renderer)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(renderer), __func__);
}

bool vtkMRMLLayerDMScriptedPipelineBridge::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!Py_IsInitialized())
  {
    return false;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod(ToPyArgs(eventData), __func__))
  {
    return result == Py_True;
  }
  return false;
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetDisplayNode(vtkMRMLNode* displayNode)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(displayNode), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(viewNode), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetScene(vtkMRMLScene* scene)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(scene), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(pipelineManager), __func__);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetPythonObject(PyObject* object)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (!object || object == this->m_object)
  {
    return;
  }

  // Set the new python lambda
  Py_XDECREF(this->m_object);
  this->m_object = object;
  Py_INCREF(this->m_object);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnUpdate(vtkObject* obj, unsigned long eventId, void* callData)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(ToPyArgs(obj, eventId, callData), __func__);
}

PyObject* vtkMRMLLayerDMScriptedPipelineBridge::CallPythonMethod(const vtkSmartPyObject& pyArgs, const std::string& fName) const
{
  if (!this->m_object)
  {
    vtkErrorMacro("" << __func__ << ": Python object is invalid.");
    return nullptr;
  }

  PyObject* method = PyObject_GetAttrString(this->m_object, fName.c_str());
  if (!method || !PyCallable_Check(method))
  {
    vtkErrorMacro("" << __func__ << ": Invalid method : " << fName);
    return nullptr;
  }

  PyObject* result = PyObject_CallObject(method, pyArgs);
  if (!result)
  {
    PyErr_Print();
    return nullptr;
  }
  return result;
}
