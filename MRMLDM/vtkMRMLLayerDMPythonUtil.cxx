#include "vtkMRMLLayerDMPythonUtil.h"
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLLayerDMPythonUtil);

vtkMRMLLayerDMPythonUtil::vtkMRMLLayerDMPythonUtil() = default;

vtkMRMLLayerDMPythonUtil::~vtkMRMLLayerDMPythonUtil() = default;

PyObject* vtkMRMLLayerDMPythonUtil::ToPyObject(vtkObjectBase* obj)
{
  return vtkPythonUtil::GetObjectFromPointer(obj);
}

PyObject* vtkMRMLLayerDMPythonUtil::ToPyObject(unsigned long value)
{
  return PyLong_FromUnsignedLong(value);
}

PyObject* vtkMRMLLayerDMPythonUtil::RawPtrToPython(void* ptr)
{
  if (ptr)
  {
    return PyCapsule_New(ptr, nullptr, nullptr);
  }

  // Return borrowed reference to Py_None
  Py_INCREF(Py_None);
  return Py_None;
}

vtkSmartPyObject vtkMRMLLayerDMPythonUtil::ToPyArgs(const std::vector<PyObject*>& pyObjs)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
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

vtkSmartPyObject vtkMRMLLayerDMPythonUtil::ToPyArgs(vtkObjectBase* obj)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
  return ToPyArgs({ ToPyObject(obj) });
}

vtkSmartPyObject vtkMRMLLayerDMPythonUtil::ToPyArgs(vtkObject* obj, unsigned long eventId, void* callData)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
  return ToPyArgs({ ToPyObject(obj), ToPyObject(eventId), RawPtrToPython(callData) });
}

PyObject* vtkMRMLLayerDMPythonUtil::CastCallData(PyObject* object, int vtkType)
{
  vtkPythonScopeGilEnsurer gilEnsurer;
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

PyObject* vtkMRMLLayerDMPythonUtil::CallPythonMethod(PyObject* object, const vtkSmartPyObject& pyArgs, const std::string& fName)
{
  if (!Py_IsInitialized() || !object)
  {
    return nullptr;
  }

  PyObject* method = PyObject_GetAttrString(object, fName.c_str());
  if (!method || !PyCallable_Check(method))
  {
    const std::string errorString = "Method is not callable : " + fName;
    PyErr_SetString(PyExc_ValueError, errorString.c_str());
    return nullptr;
  }

  return CallPythonObject(method, pyArgs);
}

PyObject* vtkMRMLLayerDMPythonUtil::CallPythonObject(PyObject* object, const vtkSmartPyObject& pyArgs)
{
  if (!Py_IsInitialized() || !object || !PyCallable_Check(object))
  {
    return nullptr;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  PyObject* result = PyObject_CallObject(object, pyArgs);
  if (!result)
  {
    PyErr_Print();
    return nullptr;
  }
  return result;
}

void vtkMRMLLayerDMPythonUtil::SetPythonObject(PyObject** destObject, PyObject* object)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  if (object == (*destObject))
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  DeletePythonObject(destObject);
  *destObject = object;
  Py_XINCREF(*destObject);
}

void vtkMRMLLayerDMPythonUtil::DeletePythonObject(PyObject** destObject)
{
  if (!Py_IsInitialized())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  Py_XDECREF(*destObject);
  *destObject = nullptr;
}
