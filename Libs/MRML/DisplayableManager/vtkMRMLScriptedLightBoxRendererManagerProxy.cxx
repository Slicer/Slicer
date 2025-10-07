#include "vtkMRMLScriptedLightBoxRendererManagerProxy.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkPythonUtil.h>
#include <vtkRenderer.h>
#include <vtkSmartPyObject.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLScriptedLightBoxRendererManagerProxy);

//---------------------------------------------------------------------------
vtkMRMLScriptedLightBoxRendererManagerProxy::vtkMRMLScriptedLightBoxRendererManagerProxy()
  : m_object(nullptr)
{
}

//---------------------------------------------------------------------------
vtkMRMLScriptedLightBoxRendererManagerProxy::~vtkMRMLScriptedLightBoxRendererManagerProxy() = default;

//---------------------------------------------------------------------------
vtkRenderer* vtkMRMLScriptedLightBoxRendererManagerProxy::GetRenderer(int lightboxId)
{
  if (!Py_IsInitialized() || !this->m_object)
  {
    return nullptr;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  PyObject* method = PyObject_GetAttrString(this->m_object, __func__);
  if (!method || !PyCallable_Check(method))
  {
    vtkErrorMacro("" << __func__ << ": Python method doesn't exist or cannot be called.");
    return nullptr;
  }

  vtkSmartPyObject pyArgs(PyTuple_Pack(1, PyLong_FromLongLong(lightboxId)));
  PyObject* result = PyObject_CallObject(method, pyArgs);
  if (!result)
  {
    PyErr_Print();
    return nullptr;
  }
  return vtkRenderer::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkRenderer"));
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedLightBoxRendererManagerProxy::SetPythonObject(PyObject* object)
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
